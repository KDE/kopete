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
#include <kapplication.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetechatsessionmanager.h"
#include "kopeteview.h"
#include "kopetecommand.h"
#include "kopeteuiglobal.h"

Kopete::Command::Command( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help, Kopete::CommandHandler::CommandType type, const QString &formatString,
	uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
	: KAction( command[0].upper() + command.right( command.length() - 1).lower(), pix, cut, parent,
	( command.lower() + QString::fromLatin1("_command") ).latin1() )
{
	init( command, handlerSlot, help, type, formatString, minArgs, maxArgs );
}

void Kopete::Command::init( const QString &command, const char* slot, const QString &help,
	Kopete::CommandHandler::CommandType type, const QString &formatString, uint minArgs, int maxArgs )
{
	m_command = command;
	m_help = help;
	m_type = type;
	m_formatString = formatString;
	m_minArgs = minArgs;
	m_maxArgs = maxArgs;
	m_processing = false;

	if(  m_type == Kopete::CommandHandler::Normal )
	{
		QObject::connect( this, SIGNAL( handleCommand( const QString &, Kopete::ChatSession *) ),
			parent(), slot );
	}

	QObject::connect( this, SIGNAL( activated() ), this, SLOT( slotAction() ) );
}

void Kopete::Command::slotAction()
{
	Kopete::ChatSession *manager = Kopete::ChatSessionManager::self()->activeView()->msgManager();

	QString args;
	if( m_minArgs > 0 )
	{
		args = KInputDialog::getText( i18n("Enter Arguments"), i18n("Enter the arguments to %1:").arg(m_command) );
		if( args.isNull() )
			return;
	}

	processCommand( args, manager, true );
}

void Kopete::Command::processCommand( const QString &args, Kopete::ChatSession *manager, bool gui )
{
	QStringList mArgs = Kopete::CommandHandler::parseArguments( args );
	if( m_processing )
	{
		printError( i18n("Alias \"%1\" expands to itself.").arg( text() ), manager, gui );
	}
	else if( mArgs.count() < m_minArgs )
	{
		printError( i18n("\"%1\" requires at least %n argument.",
			"\"%1\" requires at least %n arguments.", m_minArgs)
			.arg( text() ), manager, gui );
	}
	else if( m_maxArgs > -1 && (int)mArgs.count() > m_maxArgs )
	{
		printError( i18n("\"%1\" has a maximum of %n argument.",
			"\"%1\" has a maximum of %n arguments.", m_minArgs)
			.arg( text() ), manager, gui );
	}
	else if( !KApplication::kApplication()->authorizeKAction( name() ) )
	{
		printError( i18n("You are not authorized to perform the command \"%1\".").arg(text()), manager, gui );
	}
	else
	{
		m_processing = true;
		if( m_type == Kopete::CommandHandler::UserAlias ||
			m_type == Kopete::CommandHandler::SystemAlias )
		{
			QString formatString = m_formatString;

			// Translate %s to the whole string and %n to current nickname

			formatString.replace( QString::fromLatin1("%n"), manager->myself()->nickName() );
			formatString.replace( QString::fromLatin1("%s"), args );

			// Translate %1..%N to word1..wordN

			while( mArgs.count() > 0 )
			{
				formatString = formatString.arg( mArgs.front() );
				mArgs.pop_front();
			}

			kdDebug(14010) << "New Command after processing alias: " << formatString << endl;

			Kopete::CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/") + formatString, manager );
		}
		else
		{
			emit( handleCommand( args, manager ) );
		}
		m_processing = false;
	}
}

void Kopete::Command::printError( const QString &error, Kopete::ChatSession *manager, bool gui ) const
{
	if( gui )
	{
		KMessageBox::error( Kopete::UI::Global::mainWidget(), error, i18n("Command Error") );
	}
	else
	{
		Kopete::Message msg( manager->myself(), manager->members(), error,
			Kopete::Message::Internal, Kopete::Message::PlainText );
		manager->appendMessage( msg );
	}
}

#include "kopetecommand.moc"
