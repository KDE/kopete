/*
    kopetecommandhandler.cpp - Command Handler

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstringlist.h>
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>

#include "kopetemessagemanager.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "kopeteview.h"

#include "kopetecommandhandler.h"

KopeteCommand::KopeteCommand( KopetePlugin *parent, const QString &command, const char* handlerSlot,
	const QString &help = QString::null ) : QObject(parent)
{
	kdDebug(14010) << k_funcinfo << "New Command: " << command << endl;
	m_command = command;
	m_help = help;
	QObject::connect( this, SIGNAL( handleCommand( const QString &, KopeteMessageManager *) ), parent, handlerSlot );
}

void KopeteCommand::processCommand( const QString &args, KopeteMessageManager *manager )
{
	emit( handleCommand( args, manager ) );
}

KopeteCommandHandler *KopeteCommandHandler::s_handler = 0L;

KopeteCommandHandler::KopeteCommandHandler()
{
	s_handler = this;

	reservedCommands.insert( QString::fromLatin1("help"),
		i18n("USAGE: /help [<command>] - Used to list avaialble commands, or show help for a specified command.") );
	reservedCommands.insert( QString::fromLatin1("close"),
		i18n("USAGE: /close - Closes the current view.") );
	reservedCommands.insert( QString::fromLatin1("part"),
		i18n("USAGE: /part - Closes the current view.") );
	reservedCommands.insert( QString::fromLatin1("clear"),
		i18n("USAGE: /clear - Clears the active view's chat buffer.") );
	reservedCommands.insert( QString::fromLatin1("me"),
		i18n("USAGE: /me <text> - Formats message as in \"<nickname> went to the store\".") );
	reservedCommands.insert( QString::fromLatin1("exec"),
		i18n("USAGE: /exec [-o] <command> - Executes the specified command and displays output in the chat buffer. If"
		" -o is specified, the results are output to all members of the chat.") );

	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded( KopetePlugin*) ), this, SLOT(slotPluginLoaded(KopetePlugin*)) );
}

KopeteCommandHandler::~KopeteCommandHandler()
{

}

KopeteCommandHandler *KopeteCommandHandler::commandHandler()
{
	return s_handler;
}

void KopeteCommandHandler::registerCommand( KopetePlugin *parent, const QString &command, const char* handlerSlot,
	const QString &help )
{
	QString lowerCommand = command.lower();

	KopeteCommand *mCommand = new KopeteCommand( parent, lowerCommand, handlerSlot, help);
	pluginCommands[ parent ].insert( lowerCommand, mCommand );
}

void KopeteCommandHandler::unregisterCommand( KopetePlugin *parent, const QString &command )
{
	if( pluginCommands[ parent ].find(command) )
		pluginCommands[ parent ].remove( command );
}

bool KopeteCommandHandler::processMessage( KopeteMessage &msg, KopeteMessageManager *manager )
{
	QRegExp spaces( QString::fromLatin1("\\s+") );
	QString messageBody = msg.plainBody();
	QString command = messageBody.section( spaces, 0, 0).section('/',1).lower();
	QString args = messageBody.section( spaces, 1 );

	//Try to find a plugin specified command first
	CommandList mCommands = commands( msg.from()->protocol() );
	QDictIterator<KopeteCommand> it( mCommands );
	for( ; it.current(); ++it )
	{
		if( it.current()->command() == command )
		{
			kdDebug(14010) << k_funcinfo << "Handled Command" << endl;
			it.current()->processCommand( args, manager );
			return true;
		}
	}

	//No plugin specified command. See if it is a reserved command
	if( reservedCommands.contains( command ) )
	{
		kdDebug(14010) << k_funcinfo << "Reserved Command" << endl;
		//Reserved command. Process it internally.
		reservedCommand( command, args, manager );
		return true;
	}

	return false;
}

void KopeteCommandHandler::reservedCommand( const QString &command, const QString &args, KopeteMessageManager *manager )
{
	QStringList argsList = parseArguments( args );
	if( command == QString::fromLatin1("help") )
	{
		QString output;
		if( argsList.isEmpty() )
		{
			int commandCount = 0;
			output = i18n("Available Commands:\n");
			for( CommandMap::Iterator it = reservedCommands.begin(); it != reservedCommands.end(); ++it )
			{
				output.append( it.key().upper() + '\t' );
				if( commandCount++ == 5 )
				{
					commandCount = 0;
					output.append( '\n' );
				}
			}

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
			KopeteCommand *c = commands( manager->user()->protocol() )[ argsList.front() ];
			if( c && !c->help().isNull() )
				output = c->help();
			else if( reservedCommands.contains( argsList.front() ) )
				output = reservedCommands[ argsList.front() ];
			else
				output = i18n("%1 has no help available.").arg( argsList.front() );
		}

		KopeteMessage msg(manager->user(), manager->members(), output, KopeteMessage::Internal, KopeteMessage::PlainText);
		manager->appendMessage(msg);
	}
	else if( command == QString::fromLatin1("exec") )
	{
		kdDebug(14010) << k_funcinfo << "Execute Command:" << args << endl;

		KProcess *proc = new KProcess(manager);

		if( argsList.front() == QString::fromLatin1("-o") )
		{
			*proc << *argsList.at(1);
			processMap.insert( proc, ManagerPair(manager, KopeteMessage::Outbound) );
			for( QStringList::Iterator it = argsList.at(2); it != argsList.end(); ++it )
				*proc << KProcess::quote( *it );
		}
		else
		{
			*proc << argsList.front();
			processMap.insert( proc, ManagerPair(manager, KopeteMessage::Internal) );
			for( QStringList::Iterator it = argsList.at(1); it != argsList.end(); ++it )
				*proc << KProcess::quote( *it );
		}

		connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
		connect(proc, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
		proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
	}
	else if( command == QString::fromLatin1("clear") )
		manager->view()->clear();
	else if( command == QString::fromLatin1("part") || command == QString::fromLatin1("close") )
		manager->view()->closeView();
	else if( command == QString::fromLatin1("me") )
	{
		QString output = manager->user()->displayName() + QChar(' ') + args;
		KopeteMessage msg(manager->user(), manager->members(), output, KopeteMessage::Outbound, KopeteMessage::PlainText);
		manager->sendMessage(msg);
	}
}

void KopeteCommandHandler::slotExecReturnedData(KProcess *proc, char *buff, int bufflen )
{
	kdDebug(14010) << k_funcinfo << endl;
	QString buffer = QString::fromLocal8Bit( buff, bufflen );
	ManagerPair p = processMap[ proc ];
	KopeteMessage msg(p.first->user(), p.first->members(), buffer, p.second, KopeteMessage::PlainText);
	if( p.second == KopeteMessage::Outbound )
		p.first->sendMessage( msg );
	else
		p.first->appendMessage( msg );
}

void KopeteCommandHandler::slotExecFinished(KProcess *proc)
{
	delete proc;
	processMap.remove( proc );
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
	for( PluginCommandMap::Iterator it = pluginCommands.begin(); it != pluginCommands.end(); ++it )
	{
		QDictIterator<KopeteCommand> itDict( it.data() );
		for( ; itDict.current(); ++itDict )
		{
			if( itDict.currentKey() == command )
				return true;
		}
	}

	if( reservedCommands.contains( command ) )
		return true;

	return false;
}

CommandList KopeteCommandHandler::commands( KopeteProtocol *protocol )
{
	CommandList commandList;
	for( PluginCommandMap::Iterator it = pluginCommands.begin(); it != pluginCommands.end(); ++it )
	{
		if( !it.key()->inherits("KopeteProtocol") || it.key() == protocol )
		{
			QDictIterator<KopeteCommand> itDict( it.data() );
			for( ; itDict.current(); ++itDict )
				commandList.insert( itDict.currentKey(), itDict.current() );
		}
	}

	return commandList;
}

void KopeteCommandHandler::slotPluginLoaded( KopetePlugin *plugin )
{
	connect( plugin, SIGNAL( destroyed( QObject * ) ), this, SLOT( slotPluginDestroyed( QObject * ) ) );
	if( !pluginCommands.contains( plugin ) )
	{
		//Create a QDict optomized for a larger # of commands, and case insensitive
		CommandList mCommands(63, false);
		mCommands.setAutoDelete( true );
		pluginCommands.insert( plugin, mCommands );
	}
}

void KopeteCommandHandler::slotPluginDestroyed( QObject *plugin )
{
	pluginCommands.remove( static_cast<KopetePlugin*>(plugin)  );
}

#include "kopetecommandhandler.moc"
