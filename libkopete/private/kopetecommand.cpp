/*
    kopetecommand.cpp - Command

    Copyright (c) 2003 by Jason Keirstead <jason@keirstead.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qstringlist.h>
#include <kdebug.h>

#include "kopetecommand.h"

KopeteCommand::KopeteCommand( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help, KopeteCommandHandler::CommandType type, const QString &formatString ) : QObject(parent)
{
	m_command = command;
	m_help = help;
	m_type = type;
	m_formatString = formatString;
	
	if(  m_type == KopeteCommandHandler::Normal )
		QObject::connect( this, SIGNAL( handleCommand( const QString &, KopeteMessageManager *) ), parent, handlerSlot );
}

void KopeteCommand::processCommand( const QString &args, KopeteMessageManager *manager )
{
	if( m_type == KopeteCommandHandler::UserAlias || 
		m_type == KopeteCommandHandler::SystemAlias )
	{
		QString formatString = m_formatString;
		QStringList mArgs = KopeteCommandHandler::parseArguments( args );
		while( mArgs.count() > 0 )
		{
			formatString = formatString.arg( mArgs.front() );
			mArgs.pop_front();
		}
		
		kdDebug() << "New Command after processing alias: " << formatString << endl;
		
		KopeteCommandHandler::commandHandler()->processMessage( QString::fromLatin1("/") + formatString, manager ); 
	}
	else
	{
		emit( handleCommand( args, manager ) );
	}
}

#include "kopetecommand.moc"
