/*
    kopetecommand.cpp - Command

    Copyright (c) 2003 by Jason Keirstead <jason@keirstead.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "kopeteplugin.h"
#include "kopetemessagemanager.h"

#include "kopetecommand.h"

KopeteCommand::KopeteCommand( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help = QString::null ) : QObject(parent)
{
	m_command = command;
	m_help = help;
	QObject::connect( this, SIGNAL( handleCommand( const QString &, KopeteMessageManager *) ), parent, handlerSlot );
}

void KopeteCommand::processCommand( const QString &args, KopeteMessageManager *manager )
{
	emit( handleCommand( args, manager ) );
}

#include "kopetecommand.moc"
