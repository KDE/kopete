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

#include "kopetemessagemanager.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"

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

	reservedCommands.append( QString::fromLatin1("help") );
	reservedCommands.append( QString::fromLatin1("clear") );
	reservedCommands.append( QString::fromLatin1("exec") );

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
	if( !reservedCommands.contains( lowerCommand ) )
	{
		KopeteCommand *mCommand = new KopeteCommand( parent, lowerCommand, handlerSlot, help);
		pluginCommands[ parent ].insert( lowerCommand, mCommand );
	}
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

	bool retVal = false;

	if( reservedCommands.contains( command ) )
	{
		kdDebug(14010) << k_funcinfo << "Reserved Command" << endl;
		//Reserved command. Process it internally.
		reservedCommand( command, parseArguments( args ), manager );
	}
	else
	{
		//Not a reserved command.
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
	}

	return retVal;
}

void KopeteCommandHandler::reservedCommand( const QString &command, const QStringList &args, KopeteMessageManager *manager )
{
	if( command == QString::fromLatin1("help") )
	{
		if( args.isEmpty() )
		{

		}
		else
		{

		}
	}
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
