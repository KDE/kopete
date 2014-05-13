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

#include <qregexp.h>
#include <QList>
#include <QApplication>
#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kxmlguiclient.h>
#include <kaction.h>
#include <qdom.h>
#include <kauthorized.h>
#include <kactioncollection.h>
#include <ktoolinvocation.h>
#include <kprocess.h>

#include "kopetechatsessionmanager.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteview.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecommandhandler.h"
#include "kopetecontact.h"
#include "kopetecommand.h"
#include "kopeteonlinestatusmanager.h"

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

			CommandList::Iterator it, itEnd = mCommands.end();
			for( it = mCommands.begin(); it != itEnd; ++it )
			{
				KAction *a = static_cast<KAction*>( it.value() );
				actionCollection()->addAction( a->objectName(), a );
				QDomElement newNode = doc.createElement( QString::fromLatin1("Action") );
				newNode.setAttribute( QString::fromLatin1("name"), a->objectName() );

				bool added = false;
				for( QDomElement n = menu.firstChild().toElement();
					!n.isNull(); n = n.nextSibling().toElement() )
				{
					if( a->objectName() < n.attribute(QString::fromLatin1("name")))
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
	QList<KAction *> m_commands;
};

CommandHandlerPrivate *Kopete::CommandHandler::p = 0L;

Kopete::CommandHandler::CommandHandler() : QObject( qApp )
{
	p->s_handler = this;
	p->inCommand = false;

	CommandList mCommands;
	mCommands.reserve(31);
	p->pluginCommands.insert( this, mCommands );

	registerCommand( this, QString::fromLatin1("help"), SLOT(slotHelpCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /help [<command>] - Used to list available commands, or show help for a specified command." ), 0, 1 );

	registerCommand( this, QString::fromLatin1("url"), SLOT(slotOpenLastUrl(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /url - Opens last URL for current chat in default browser." ) );

	registerCommand( this, QString::fromLatin1("close"), SLOT(slotCloseCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /close - Closes the current view." ) );

	// FIXME: What's the difference with /close? The help doesn't explain it - Martijn
	registerCommand( this, QString::fromLatin1("part"), SLOT(slotPartCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /part - Closes the current view." ) );

	registerCommand( this, QString::fromLatin1("clear"), SLOT(slotClearCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /clear - Clears the active view's chat buffer." ) );

	//registerCommand( this, QString::fromLatin1("me"), SLOT(slotMeCommand(QString,Kopete::ChatSession*)),
	//	i18n( "USAGE: /me <text> - Formats message as in '<nickname> went to the store'." ) );

	registerCommand( this, QString::fromLatin1("away"), SLOT(slotAwayCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /away [<reason>] - Marks you as away/back for the current account only." ) );

	registerCommand( this, QString::fromLatin1("awayall"), SLOT(slotAwayAllCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /awayall [<reason>] - Marks you as away/back for all accounts." ) );

	registerCommand( this, QString::fromLatin1("say"), SLOT(slotSayCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /say <text> - Say text in this chat. This is the same as just typing a message, but is very "
			"useful for scripts." ), 1 );

	registerCommand( this, QString::fromLatin1("exec"), SLOT(slotExecCommand(QString,Kopete::ChatSession*)),
		i18n( "USAGE: /exec [-o] <command> - Executes the specified command and displays the output in the chat buffer. "
		"If -o is specified, the output is sent to all members of the chat."), 1 );

	connect( Kopete::PluginManager::self(), SIGNAL(pluginLoaded(Kopete::Plugin*)),
		this, SLOT(slotPluginLoaded(Kopete::Plugin*)) );

	connect( Kopete::ChatSessionManager::self(), SIGNAL(viewCreated(KopeteView*)),
		this, SLOT(slotViewCreated(KopeteView*)) );
}

Kopete::CommandHandler::~CommandHandler()
{
	CommandList commandList = p->pluginCommands[this];
	while (!commandList.isEmpty())
	{
		Kopete::Command *value = *commandList.begin();
		commandList.erase(commandList.begin());
		delete value;
    	}

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
	const QString lowerCommand = command.toLower();

	Kopete::Command *mCommand = new Kopete::Command( parent, lowerCommand, handlerSlot, help,
		Normal, QString::null, minArgs, maxArgs, cut, pix);	//krazy:exclude=nullstrassign for old broken gcc
	p->pluginCommands[ parent ].insert( lowerCommand, mCommand );
}

void Kopete::CommandHandler::unregisterCommand( QObject *parent, const QString &command )
{
	if( p->pluginCommands[ parent ].contains(command) )
		p->pluginCommands[ parent ].remove( command );
}

void Kopete::CommandHandler::registerAlias( QObject *parent, const QString &alias, const QString &formatString,
	const QString &help, CommandType type, uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
{
	const QString lowerAlias = alias.toLower();

	Kopete::Command *mCommand = new Kopete::Command( parent, lowerAlias, 0L, help, type,
		formatString, minArgs, maxArgs, cut, pix );
	p->pluginCommands[ parent ].insert( lowerAlias, mCommand );
}

void Kopete::CommandHandler::unregisterAlias( QObject *parent, const QString &alias )
{
	if( p->pluginCommands[ parent ].contains(alias) )
		p->pluginCommands[ parent ].remove( alias );
}

bool Kopete::CommandHandler::processMessage( const QString &msg, Kopete::ChatSession *manager )
{
	if( p->inCommand )
		return false;
	QRegExp splitRx( QString::fromLatin1("^/([\\S]+)(.*)") );
	QString command;
	QString args;
	if(splitRx.indexIn(msg) != -1)
	{
		command = splitRx.cap(1);
		args = splitRx.cap(2).mid(1);
	}
	else
		return false;

	CommandList mCommands = commands( manager->protocol() );
	Kopete::Command *c = mCommands.value(command);
	if(c)
	{
		kDebug(14010) << "Handled Command";
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
	const QString messageBody = msg.plainBody();

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
		CommandList::Iterator it, itEnd = mCommands.end();
		for( it = mCommands.begin(); it != itEnd; ++it )
		{
			output.append( it.value()->command().toUpper() + '\t' );
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
		const QString command = parseArguments( args ).front().toLower();
		Kopete::Command *c = commands( manager->myself()->protocol() ).value( command );
		if( c && !c->help().isNull() )
			output = c->help();
		else
			output = i18n("There is no help available for '%1'.", command );
	}

	Kopete::Message msg(manager->myself(), manager->members());
	msg.setDirection( Kopete::Message::Internal );
	msg.setPlainBody( output );

	manager->appendMessage(msg);
}

void Kopete::CommandHandler::slotSayCommand( const QString &args, Kopete::ChatSession *manager )
{
	//Just say whatever is passed
	Kopete::Message msg(manager->myself(), manager->members());
	msg.setPlainBody( args );
	msg.setDirection( Kopete::Message::Outbound );

	manager->sendMessage(msg);
}

void Kopete::CommandHandler::slotExecCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( !args.isEmpty() )
	{
		KProcess *proc = 0L;
		if ( KAuthorized::authorizeKAction( "shell_access" ) )
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

			connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotExecError(QProcess::ProcessError)));
			connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotExecFinished()));

			proc->setOutputChannelMode(KProcess::MergedChannels);
			proc->setNextOpenMode(QIODevice::ReadOnly);
			proc->clearEnvironment();
			proc->start();
			proc->closeWriteChannel();
		}
		else
		{
			Kopete::Message msg(manager->myself(), manager->members()  );
			msg.setDirection( Kopete::Message::Internal );
			msg.setPlainBody( i18n( "ERROR: Shell access has been restricted on your system. The /exec command will not function." ) );
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
	const bool goAway = !manager->account()->isAway();

	if( args.isEmpty() )
		manager->account()->setOnlineStatus( OnlineStatusManager::self()->onlineStatus(manager->account()->protocol() , goAway ? OnlineStatusManager::Away : OnlineStatusManager::Online) );
	else
		manager->account()->setOnlineStatus( OnlineStatusManager::self()->onlineStatus(manager->account()->protocol() , goAway ? OnlineStatusManager::Away : OnlineStatusManager::Online) , args);
}

void Kopete::CommandHandler::slotAwayAllCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( manager->account()->isAway() ) {
		Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Online );
	}
	else {
		Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Away, args );
	}
}

void Kopete::CommandHandler::slotOpenLastUrl( const QString &, Kopete::ChatSession *manager )
{
	QString tempstr = manager->lastUrl();
	if ( !tempstr.isEmpty() ) {
	//now there is no difference whether it is a brwserUrl or a mailtoUrl
		KToolInvocation::invokeBrowser( tempstr );
	}
}

void Kopete::CommandHandler::slotCloseCommand( const QString &, Kopete::ChatSession *manager )
{
	if( manager->view() )
		manager->view()->closeView();
}

void Kopete::CommandHandler::slotExecError( QProcess::ProcessError error )
{
	kDebug(14010);
	KProcess *proc = static_cast<KProcess *>(sender());
	if (error == QProcess::FailedToStart) {
		ManagerPair &mgrPair = p->processMap[ proc ];
		Kopete::Message msg( mgrPair.first->myself(), mgrPair.first->members() );
		msg.setDirection( Kopete::Message::Internal );
		msg.setPlainBody( i18n( "ERROR: Failed to start process from /exec command." ) );
		mgrPair.first->sendMessage( msg );
	} else {
		const QString &buffer = QString::fromUtf8(proc->readAll());
		if (!buffer.isEmpty())
			slotExecSendMessage(proc, buffer);
	}
	disconnect(proc, 0, 0, 0);
	p->processMap.remove(proc);
	proc->deleteLater();
}

void Kopete::CommandHandler::slotExecFinished()
{
	kDebug(14010);
	KProcess *proc = static_cast<KProcess *>(sender());
	const QString &buffer = QString::fromUtf8(proc->readAll());
	if (!buffer.isEmpty())
		slotExecSendMessage(proc, buffer);
	disconnect(proc, 0, 0, 0);
	p->processMap.remove(proc);
	proc->deleteLater();
}

void Kopete::CommandHandler::slotExecSendMessage( KProcess *proc, const QString &buffer )
{
	kDebug(14010);
	ManagerPair &mgrPair = p->processMap[ proc ];
	Kopete::Message msg( mgrPair.first->myself(), mgrPair.first->members());
	msg.setDirection( mgrPair.second );
	msg.setPlainBody( buffer );

	if( mgrPair.second == Kopete::Message::Outbound )
		mgrPair.first->sendMessage( msg );
	else
		mgrPair.first->appendMessage( msg );
}

QStringList Kopete::CommandHandler::parseArguments( const QString &args )
{
	QStringList arguments;
	QRegExp quotedArgs( QString::fromLatin1("\"(.*)\"") );
	quotedArgs.setMinimal( true );

	if ( quotedArgs.indexIn( args ) != -1 )
	{
		for( int i = 0; i< quotedArgs.numCaptures(); i++ )
			arguments.append( quotedArgs.cap(i) );
	}

	const QStringList otherArgs = args.section( quotedArgs, 0 ).split( QRegExp(QString::fromLatin1("\\s+")), QString::SkipEmptyParts);
	for( QStringList::ConstIterator it = otherArgs.constBegin(); it != otherArgs.constEnd(); ++it )
		arguments.append( *it );

	return arguments;
}

bool Kopete::CommandHandler::commandHandled( const QString &command )
{
	for( PluginCommandMap::Iterator it = p->pluginCommands.begin(); it != p->pluginCommands.end(); ++it )
	{
		if( it.value().value( command ) )
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
	CommandList::Iterator it, itEnd = commandList.end();

	// Loop through commands and check if they match the supplied command
	for( it = commandList.begin(); it != itEnd; ++it )
	{
		if( it.value()->command().toLower() == command )
			return true;
	}

	// No commands found
	return false;
}

CommandList Kopete::CommandHandler::commands( Kopete::Protocol *protocol )
{
	CommandList commandList;
	commandList.reserve(63);

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
			addCommands( it.value(), commandList );
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
	CommandList::Iterator itDict, itDictEnd = from.end();
	for( itDict = from.begin(); itDict != itDictEnd; ++itDict )
	{
		if( !to.value( itDict.key() ) &&
				( type == Undefined || itDict.value()->type() == type ) )
			to.insert( itDict.key(), itDict.value() );
	}
}

void Kopete::CommandHandler::slotViewCreated( KopeteView *view )
{
	new KopeteCommandGUIClient( view->msgManager() );
}

void Kopete::CommandHandler::slotPluginLoaded( Kopete::Plugin *plugin )
{
	connect( plugin, SIGNAL(destroyed(QObject*)), this, SLOT(slotPluginDestroyed(QObject*)) );
	if( !p->pluginCommands.contains( plugin ) )
	{
		//Create a QDict optomized for a larger # of commands, and case insensitive
		CommandList mCommands;
		mCommands.reserve(31);
		p->pluginCommands.insert( plugin, mCommands );
	}
}

void Kopete::CommandHandler::slotPluginDestroyed( QObject *plugin )
{
	p->pluginCommands.remove( static_cast<Kopete::Plugin*>(plugin)  );
}

#include "kopetecommandhandler.moc"

// vim: set noet ts=4 sts=4 sw=4:

