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
#include <kxmlguiclient.h>
#include <kaction.h>
#include <qdom.h>

#include "kopetechatsessionmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteview.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetecontact.h"
#include "kopetecommand.h"

using Kopete::CommandList;

typedef QMap<QObject*, CommandList> PluginCommandMap;
typedef QMap<QString,QString> CommandMap;
typedef QPair<Kopete::ChatSession*, Kopete::Message::MessageDirection> ManagerPair;

class KopeteCommandGUIClient : public QObject, public KXMLGUIClient
{
	public:
		KopeteCommandGUIClient( Kopete::ChatSession *manager ) : QObject(manager), KXMLGUIClient(manager)
		{
			setXMLFile( QString::fromLatin1("kopetecommandui.rc") );

			QDomDocument doc = domDocument();
			QDomNode menu = doc.documentElement().firstChild().firstChild().firstChild();
			CommandList mCommands = Kopete::CommandHandler::commandHandler()->commands(
					manager->protocol()
			);

			for( QDictIterator<Kopete::Command> it( mCommands ); it.current(); ++it )
			{
				KAction *a = static_cast<KAction*>( it.current() );
				actionCollection()->insert( a );
				QDomElement newNode = doc.createElement( QString::fromLatin1("Action") );
				newNode.setAttribute( QString::fromLatin1("name"),
					QString::fromLatin1( a->name() ) );

				bool added = false;
				for( QDomElement n = menu.firstChild().toElement();
					!n.isNull(); n = n.nextSibling().toElement() )
				{
					if( QString::fromLatin1(a->name()) < n.attribute(QString::fromLatin1("name")))
					{
						menu.insertBefore( newNode, n );
						added = true;
						break;
					}
				}

				if( !added )
				{
					menu.appendChild( newNode );
				}
			}

			setDOMDocument( doc );
		}
};

struct CommandHandlerPrivate
{
	PluginCommandMap pluginCommands;
	Kopete::CommandHandler *s_handler;
	QMap<KProcess*,ManagerPair> processMap;
	bool inCommand;
	QPtrList<KAction> m_commands;
};

CommandHandlerPrivate *Kopete::CommandHandler::p = 0L;

Kopete::CommandHandler::CommandHandler() : QObject( qApp )
{
	p->s_handler = this;
	p->inCommand = false;

	CommandList mCommands(31, false);
	mCommands.setAutoDelete( true );
	p->pluginCommands.insert( this, mCommands );

	registerCommand( this, QString::fromLatin1("help"), SLOT( slotHelpCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /help [<command>] - Used to list available commands, or show help for a specified command." ), 0, 1 );

	registerCommand( this, QString::fromLatin1("close"), SLOT( slotCloseCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /close - Closes the current view." ) );

	// FIXME: What's the difference with /close? The help doesn't explain it - Martijn
	registerCommand( this, QString::fromLatin1("part"), SLOT( slotPartCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /part - Closes the current view." ) );

	registerCommand( this, QString::fromLatin1("clear"), SLOT( slotClearCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /clear - Clears the active view's chat buffer." ) );

	//registerCommand( this, QString::fromLatin1("me"), SLOT( slotMeCommand( const QString &, Kopete::ChatSession * ) ),
	//	i18n( "USAGE: /me <text> - Formats message as in '<nickname> went to the store'." ) );

	registerCommand( this, QString::fromLatin1("away"), SLOT( slotAwayCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /away [<reason>] - Marks you as away/back for the current account only." ) );

	registerCommand( this, QString::fromLatin1("awayall"), SLOT( slotAwayAllCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /awayall [<reason>] - Marks you as away/back for all accounts." ) );

	registerCommand( this, QString::fromLatin1("say"), SLOT( slotSayCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /say <text> - Say text in this chat. This is the same as just typing a message, but is very "
			"useful for scripts." ), 1 );

	registerCommand( this, QString::fromLatin1("exec"), SLOT( slotExecCommand( const QString &, Kopete::ChatSession * ) ),
		i18n( "USAGE: /exec [-o] <command> - Executes the specified command and displays the output in the chat buffer. "
		"If -o is specified, the output is sent to all members of the chat."), 1 );

	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin*) ),
		this, SLOT(slotPluginLoaded(Kopete::Plugin*) ) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL( viewCreated( KopeteView * ) ),
		this, SLOT( slotViewCreated( KopeteView* ) ) );
}

Kopete::CommandHandler::~CommandHandler()
{
	delete p;
}

Kopete::CommandHandler *Kopete::CommandHandler::commandHandler()
{
	if( !p )
	{
		p = new CommandHandlerPrivate;
		p->s_handler = new Kopete::CommandHandler();
	}

	return p->s_handler;
}

void Kopete::CommandHandler::registerCommand( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help, uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
{
	QString lowerCommand = command.lower();

	Kopete::Command *mCommand = new Kopete::Command( parent, lowerCommand, handlerSlot, help,
		Normal, QString::null, minArgs, maxArgs, cut, pix);
	p->pluginCommands[ parent ].insert( lowerCommand, mCommand );
}

void Kopete::CommandHandler::unregisterCommand( QObject *parent, const QString &command )
{
	if( p->pluginCommands[ parent ].find(command) )
		p->pluginCommands[ parent ].remove( command );
}

void Kopete::CommandHandler::registerAlias( QObject *parent, const QString &alias, const QString &formatString,
	const QString &help, CommandType type, uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
{
	QString lowerAlias = alias.lower();

	Kopete::Command *mCommand = new Kopete::Command( parent, lowerAlias, 0L, help, type,
		formatString, minArgs, maxArgs, cut, pix );
	p->pluginCommands[ parent ].insert( lowerAlias, mCommand );
}

void Kopete::CommandHandler::unregisterAlias( QObject *parent, const QString &alias )
{
	if( p->pluginCommands[ parent ].find(alias) )
		p->pluginCommands[ parent ].remove( alias );
}

bool Kopete::CommandHandler::processMessage( const QString &msg, Kopete::ChatSession *manager )
{
	if( p->inCommand )
		return false;
	QRegExp splitRx( QString::fromLatin1("^/([\\S]+)(.*)") );
	QString command;
	QString args;
	if(splitRx.search(msg) != -1)
	{
		command = splitRx.cap(1);
		args = splitRx.cap(2).mid(1);
	}
	else
		return false;
	
	CommandList mCommands = commands( manager->protocol() );
	Kopete::Command *c = mCommands[ command ];
	if(c)
	{
		kdDebug(14010) << k_funcinfo << "Handled Command" << endl;
		if( c->type() != SystemAlias && c->type() != UserAlias )
			p->inCommand = true;

		c->processCommand( args, manager );
		p->inCommand = false;

		return true;
	}

	return false;
}

bool Kopete::CommandHandler::processMessage( Kopete::Message &msg, Kopete::ChatSession *manager )
{
	QString messageBody = msg.plainBody();

	return processMessage( messageBody, manager );
}

void Kopete::CommandHandler::slotHelpCommand( const QString &args, Kopete::ChatSession *manager )
{
	QString output;
	if( args.isEmpty() )
	{
		int commandCount = 0;
		output = i18n( "Available Commands:\n" );

		CommandList mCommands = commands( manager->myself()->protocol() );
		QDictIterator<Kopete::Command> it( mCommands );
		for( ; it.current(); ++it )
		{
			output.append( it.current()->command().upper() + '\t' );
			if( commandCount++ == 5 )
			{
				commandCount = 0;
				output.append( '\n' );
			}
		}
		output.append( i18n( "\nType /help <command> for more information." ) );
	}
	else
	{
		QString command = parseArguments( args ).front().lower();
		Kopete::Command *c = commands( manager->myself()->protocol() )[ command ];
		if( c && !c->help().isNull() )
			output = c->help();
		else
			output = i18n("There is no help available for '%1'.").arg( command );
	}

	Kopete::Message msg(manager->myself(), manager->members(), output, Kopete::Message::Internal, Kopete::Message::PlainText);
	manager->appendMessage(msg);
}

void Kopete::CommandHandler::slotSayCommand( const QString &args, Kopete::ChatSession *manager )
{
	//Just say whatever is passed
	Kopete::Message msg(manager->myself(), manager->members(), args,
		Kopete::Message::Outbound, Kopete::Message::PlainText);
	manager->sendMessage(msg);
}

void Kopete::CommandHandler::slotExecCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( !args.isEmpty() )
	{
		KProcess *proc = 0L;
		if ( kapp->authorize( QString::fromLatin1( "shell_access" ) ) )
				proc = new KProcess(manager);
		
		if( proc )
		{
			*proc << QString::fromLatin1("sh") << QString::fromLatin1("-c");

			QStringList argsList = parseArguments( args );
			if( argsList.front() == QString::fromLatin1("-o") )
			{
				p->processMap.insert( proc, ManagerPair(manager, Kopete::Message::Outbound) );
				*proc << args.section(QRegExp(QString::fromLatin1("\\s+")), 1);
			}
			else
			{
				p->processMap.insert( proc, ManagerPair(manager, Kopete::Message::Internal) );
				*proc << args;
			}

			connect(proc, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
			connect(proc, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotExecReturnedData(KProcess *, char *, int)));
			proc->start( KProcess::NotifyOnExit, KProcess::AllOutput );
		}
		else
		{
			Kopete::Message msg(manager->myself(), manager->members(),
				i18n( "ERROR: Shell access has been restricted on your system. The /exec command will not function." ),
				Kopete::Message::Internal, Kopete::Message::PlainText );
			manager->sendMessage( msg );
		}
	}
}

void Kopete::CommandHandler::slotClearCommand( const QString &, Kopete::ChatSession *manager )
{
	if( manager->view() )
		manager->view()->clear();
}

void Kopete::CommandHandler::slotPartCommand( const QString &, Kopete::ChatSession *manager )
{
	if( manager->view() )
		manager->view()->closeView();
}

void Kopete::CommandHandler::slotAwayCommand( const QString &args, Kopete::ChatSession *manager )
{
	bool goAway = !manager->account()->isAway();

	if( args.isEmpty() )
		manager->account()->setAway( goAway );
	else
		manager->account()->setAway( goAway, args );
}

void Kopete::CommandHandler::slotAwayAllCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( manager->account()->isAway() )
		Kopete::AccountManager::self()->setAvailableAll();

	else
	{
		if( args.isEmpty() )
			Kopete::AccountManager::self()->setAwayAll();
		else
			Kopete::AccountManager::self()->setAwayAll( args );
	}
}

void Kopete::CommandHandler::slotCloseCommand( const QString &, Kopete::ChatSession *manager )
{
	if( manager->view() )
		manager->view()->closeView();
}

void Kopete::CommandHandler::slotExecReturnedData(KProcess *proc, char *buff, int bufflen )
{
	kdDebug(14010) << k_funcinfo << endl;
	QString buffer = QString::fromLocal8Bit( buff, bufflen );
	ManagerPair mgrPair = p->processMap[ proc ];
	Kopete::Message msg( mgrPair.first->myself(), mgrPair.first->members(), buffer, mgrPair.second, Kopete::Message::PlainText );
	if( mgrPair.second == Kopete::Message::Outbound )
		mgrPair.first->sendMessage( msg );
	else
		mgrPair.first->appendMessage( msg );
}

void Kopete::CommandHandler::slotExecFinished(KProcess *proc)
{
	delete proc;
	p->processMap.remove( proc );
}

QStringList Kopete::CommandHandler::parseArguments( const QString &args )
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

bool Kopete::CommandHandler::commandHandled( const QString &command )
{
	for( PluginCommandMap::Iterator it = p->pluginCommands.begin(); it != p->pluginCommands.end(); ++it )
	{
		if( it.data()[ command ] )
			return true;
	}

	return false;
}

bool Kopete::CommandHandler::commandHandledByProtocol( const QString &command, Kopete::Protocol *protocol )
{
	// Make sure the protocol is not NULL
	if(!protocol)
		return false;

	// Fetch the commands for the protocol
	CommandList commandList = commands( protocol );
	QDictIterator<Kopete::Command> it ( commandList );

	// Loop through commands and check if they match the supplied command
	for( ; it.current(); ++it )
	{
		if( it.current()->command().lower() == command )
			return true;
	}

	// No commands found
	return false;
}

CommandList Kopete::CommandHandler::commands( Kopete::Protocol *protocol )
{
	CommandList commandList(63, false);

	//Add plugin user aliases first
	addCommands( p->pluginCommands[protocol], commandList, UserAlias );

	//Add plugin system aliases next
	addCommands( p->pluginCommands[protocol], commandList, SystemAlias );

	//Add the commands for this protocol next
	addCommands( p->pluginCommands[protocol], commandList );

	//Add plugin commands
	for( PluginCommandMap::Iterator it = p->pluginCommands.begin(); it != p->pluginCommands.end(); ++it )
	{
		if( !it.key()->inherits("Kopete::Protocol") && it.key()->inherits("Kopete::Plugin") )
			addCommands( it.data(), commandList );
	}

	//Add global user aliases first
	addCommands( p->pluginCommands[this], commandList, UserAlias );

	//Add global system aliases next
	addCommands( p->pluginCommands[this], commandList, SystemAlias );

	//Add the internal commands *last*
	addCommands( p->pluginCommands[this], commandList );

	return commandList;
}

void Kopete::CommandHandler::addCommands( CommandList &from, CommandList &to, CommandType type )
{
	QDictIterator<Kopete::Command> itDict( from );
	for( ; itDict.current(); ++itDict )
	{
		if( !to[ itDict.currentKey() ] &&
				( type == Undefined || itDict.current()->type() == type ) )
			to.insert( itDict.currentKey(), itDict.current() );
	}
}

void Kopete::CommandHandler::slotViewCreated( KopeteView *view )
{
	new KopeteCommandGUIClient( view->msgManager() );
}

void Kopete::CommandHandler::slotPluginLoaded( Kopete::Plugin *plugin )
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

void Kopete::CommandHandler::slotPluginDestroyed( QObject *plugin )
{
	p->pluginCommands.remove( static_cast<Kopete::Plugin*>(plugin)  );
}

#include "kopetecommandhandler.moc"

// vim: set noet ts=4 sts=4 sw=4:

