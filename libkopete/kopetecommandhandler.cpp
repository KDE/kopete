/*
    kopetecommandhandler.cpp - Command Handler

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <kapplication.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kdeversion.h>

#include "kopetecommand.h"
#include "kopetemessagemanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteview.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetecontact.h"

typedef QMap<QObject*, CommandList> PluginCommandMap;
typedef QMap<QString,QString> CommandMap;
typedef QPair<KopeteMessageManager*, KopeteMessage::MessageDirection> ManagerPair;

struct CommandHandlerPrivate
{
	PluginCommandMap pluginCommands;
	KopeteCommandHandler *s_handler;
	QMap<KProcess*,ManagerPair> processMap;
};

CommandHandlerPrivate *KopeteCommandHandler::p = 0L;

KopeteCommandHandler::KopeteCommandHandler() : QObject( qApp )
{
	p->s_handler = this;

	CommandList mCommands(31, false);
	mCommands.setAutoDelete( true );
	p->pluginCommands.insert( this, mCommands );

	registerCommand( this, QString::fromLatin1("help"), SLOT( slotHelpCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /help [<command>] - Used to list available commands, or show help for a specified command.") );

	registerCommand( this, QString::fromLatin1("close"), SLOT( slotCloseCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /close - Closes the current view.") );

	registerCommand( this, QString::fromLatin1("part"), SLOT( slotPartCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /part - Closes the current view.") );

	registerCommand( this, QString::fromLatin1("clear"), SLOT( slotClearCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /clear - Clears the active view's chat buffer.") );

	registerCommand( this, QString::fromLatin1("me"), SLOT( slotMeCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /me <text> - Formats message as in \"<nickname> went to the store\".") );

	registerCommand( this, QString::fromLatin1("away"), SLOT( slotAwayCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /away [<reason>] - Sets you away/back in the current account only.") );

	registerCommand( this, QString::fromLatin1("awayall"), SLOT( slotAwayAllCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /awayall [<reason>] - Sets you away/back in all accounts.") );

	registerCommand( this, QString::fromLatin1("exec"), SLOT( slotExecCommand( const QString &, KopeteMessageManager * ) ),
		i18n("USAGE: /exec [-o] <command> - Executes the specified command and displays output in the chat buffer. If"
		" -o is specified, the output is sent to all members of the chat.") );

	connect( KopetePluginManager::self(), SIGNAL( pluginLoaded( KopetePlugin*) ), this, SLOT(slotPluginLoaded(KopetePlugin*)) );
}

KopeteCommandHandler::~KopeteCommandHandler()
{
	delete p;
}

KopeteCommandHandler *KopeteCommandHandler::commandHandler()
{
	if( !p )
	{
		p = new CommandHandlerPrivate;
		p->s_handler = new KopeteCommandHandler();
	}

	return p->s_handler;
}

void KopeteCommandHandler::registerCommand( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help )
{
	QString lowerCommand = command.lower();

	KopeteCommand *mCommand = new KopeteCommand( parent, lowerCommand, handlerSlot, help);
	p->pluginCommands[ parent ].insert( lowerCommand, mCommand );
}

void KopeteCommandHandler::unregisterCommand( QObject *parent, const QString &command )
{
	if( p->pluginCommands[ parent ].find(command) )
		p->pluginCommands[ parent ].remove( command );
}

bool KopeteCommandHandler::processMessage( KopeteMessage &msg, KopeteMessageManager *manager )
{
	QRegExp spaces( QString::fromLatin1("\\s+") );
	QString messageBody = msg.plainBody();
	QString command = messageBody.section(spaces, 0, 0).section('/',1).lower();

	if(command.isEmpty())
		return false;

	QString args = messageBody.section( spaces, 1 );


	//Try to find a plugin specified command first
	CommandList mCommands = commands( msg.from()->protocol() );
	KopeteCommand *c = mCommands[ command ];
	if(c)
	{
		kdDebug(14010) << k_funcinfo << "Handled Command" << endl;
		c->processCommand( args, manager );
		return true;
	}
	return false;
}

void KopeteCommandHandler::slotHelpCommand( const QString &args, KopeteMessageManager *manager )
{
	QString output;
	if( args.isEmpty() )
	{
		int commandCount = 0;
		output = i18n("Available Commands:\n");

		CommandList mCommands = commands( manager->user()->protocol() );
		QDictIterator<KopeteCommand> it( mCommands );
		for( ; it.current(); ++it )
		{
			output.append( it.current()->command().upper() + '\t' );
			if( commandCount++ == 5 )
			{
				commandCount = 0;
				output.append( '\n' );
			}
		}
		output.append( i18n("\nType /help <command> for more information.") );
	}
	else
	{
		QString command = parseArguments( args ).front().lower();
		KopeteCommand *c = commands( manager->user()->protocol() )[ command ];
		if( c && !c->help().isNull() )
			output = c->help();
		else
			output = i18n("%1 has no help available.").arg( command );
	}

	KopeteMessage msg(manager->user(), manager->members(), output, KopeteMessage::Internal, KopeteMessage::PlainText);
	manager->appendMessage(msg);
}

void KopeteCommandHandler::slotExecCommand( const QString &args, KopeteMessageManager *manager )
{
	if( !args.isEmpty() )
	{
		KProcess *proc = 0L;
		#if KDE_IS_VERSION( 3, 1, 90 )
			if ( kapp->authorize( QString::fromLatin1( "shell_access" ) ) )
				proc = new KProcess(manager);
		#else
			proc = new KProcess();
			connect( manager , SIGNAL (destroyed() ) , proc , SLOT(deleteLater()));
		#endif
		if( proc )
		{
			*proc << QString::fromLatin1("sh") << QString::fromLatin1("-c");

			QStringList argsList = parseArguments( args );
			if( argsList.front() == QString::fromLatin1("-o") )
			{
				p->processMap.insert( proc, ManagerPair(manager, KopeteMessage::Outbound) );
				*proc << args.section(QRegExp(QString::fromLatin1("\\s+")), 1);
			}
			else
			{
				p->processMap.insert( proc, ManagerPair(manager, KopeteMessage::Internal) );
				*proc << args;
			}

			connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
			connect(proc, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
			proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
		}
		else
		{
			KopeteMessage msg(manager->user(), manager->members(), i18n("ERROR: Shell access has been restricted on your system. The /exec command will not function."),
				KopeteMessage::Internal, KopeteMessage::PlainText);
			manager->sendMessage( msg );
		}
	}
}

void KopeteCommandHandler::slotClearCommand( const QString &, KopeteMessageManager *manager )
{
	manager->view()->clear();
}

void KopeteCommandHandler::slotPartCommand( const QString &, KopeteMessageManager *manager )
{
	manager->view()->closeView();
}

void KopeteCommandHandler::slotAwayCommand( const QString &args, KopeteMessageManager *manager )
{
	bool goAway = !manager->account()->isAway();

	if( args.isEmpty() )
		manager->account()->setAway( goAway );
	else
		manager->account()->setAway( goAway, args );
}

void KopeteCommandHandler::slotAwayAllCommand( const QString &args, KopeteMessageManager *manager )
{
	if( manager->account()->isAway() )
		KopeteAccountManager::manager()->setAvailableAll();

	else
	{
		if( args.isEmpty() )
			KopeteAccountManager::manager()->setAwayAll();
		else
			KopeteAccountManager::manager()->setAwayAll( args );
	}
}

void KopeteCommandHandler::slotCloseCommand( const QString &, KopeteMessageManager *manager )
{
	manager->view()->closeView();
}

void KopeteCommandHandler::slotMeCommand( const QString &args, KopeteMessageManager *manager )
{
	QString output = manager->user()->displayName() + QChar(' ') + args;
	KopeteMessage msg(manager->user(), manager->members(), output, KopeteMessage::Outbound, KopeteMessage::PlainText);
	manager->sendMessage(msg);
}

void KopeteCommandHandler::slotExecReturnedData(KProcess *proc, char *buff, int bufflen )
{
	kdDebug(14010) << k_funcinfo << endl;
	QString buffer = QString::fromLocal8Bit( buff, bufflen );
	ManagerPair mgrPair = p->processMap[ proc ];
	KopeteMessage msg( mgrPair.first->user(), mgrPair.first->members(), buffer, mgrPair.second, KopeteMessage::PlainText );
	if( mgrPair.second == KopeteMessage::Outbound )
		mgrPair.first->sendMessage( msg );
	else
		mgrPair.first->appendMessage( msg );
}

void KopeteCommandHandler::slotExecFinished(KProcess *proc)
{
	delete proc;
	p->processMap.remove( proc );
}

QStringList KopeteCommandHandler::parseArguments( const QString &args )
{
	QStringList arguments;
	QRegExp quotedArgs( QString::fromLatin1("\"(.*)\"") );
	quotedArgs.setMinimal( true );

	if ( quotedArgs.search( args ) != -1 )
	{
		for( int i = 0; i< quotedArgs.numCaptures(); i++ )
			arguments.append( quotedArgs.cap(i) );
	}

	QStringList otherArgs = QStringList::split( QRegExp(QString::fromLatin1("\\s+")), args.section( quotedArgs, 0 ) );
	for( QStringList::Iterator it = otherArgs.begin(); it != otherArgs.end(); ++it )
		arguments.append( *it );

	return arguments;
}

bool KopeteCommandHandler::commandHandled( const QString &command )
{
	for( PluginCommandMap::Iterator it = p->pluginCommands.begin(); it != p->pluginCommands.end(); ++it )
	{
		if( it.data()[ command ] )
			return true;
	}

	return false;
}

CommandList KopeteCommandHandler::commands( KopeteProtocol *protocol )
{
	CommandList commandList(63, false);

	//Add the commands for this protocol *first*
	addCommands( p->pluginCommands[protocol], commandList );

	//Add plugin commands
	for( PluginCommandMap::Iterator it = p->pluginCommands.begin(); it != p->pluginCommands.end(); ++it )
	{
		if( !it.key()->inherits("KopeteProtocol") && it.key()->inherits("KopetePlugin") )
			addCommands( it.data(), commandList );
	}

	//Add the internal commands *last*
	addCommands( p->pluginCommands[this], commandList );

	return commandList;
}

void KopeteCommandHandler::addCommands( CommandList &from, CommandList &to )
{
	QDictIterator<KopeteCommand> itDict( from );
	for( ; itDict.current(); ++itDict )
	{
		if( !to[ itDict.currentKey() ] )
			to.insert( itDict.currentKey(), itDict.current() );
	}
}

void KopeteCommandHandler::slotPluginLoaded( KopetePlugin *plugin )
{
	connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
	if( !p->pluginCommands.contains( plugin ) )
	{
		//Create a QDict optomized for a larger # of commands, and case insensitive
		CommandList mCommands(31, false);
		mCommands.setAutoDelete( true );
		p->pluginCommands.insert( plugin, mCommands );
	}
}

void KopeteCommandHandler::slotPluginDestroyed( QObject *plugin )
{
	p->pluginCommands.remove( static_cast<KopetePlugin*>(plugin)  );
}

#include "kopetecommandhandler.moc"
