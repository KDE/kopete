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

#include "kopetemessagemanagerfactory.h"
#include "kopeteview.h"
#include "kopetecommand.h"
#include "kopeteuiglobal.h"

KopeteCommand::KopeteCommand( QObject *parent, const QString &command, const char* handlerSlot,
	const QString &help, KopeteCommandHandler::CommandType type, const QString &formatString,
	uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
	: KAction( command[0].upper() + command.right( command.length() - 1).lower(), pix, cut, parent,
	( command.lower() + QString::fromLatin1("_command") ).latin1() )
{
	init( command, handlerSlot, help, type, formatString, minArgs, maxArgs );
}

void KopeteCommand::init( const QString &command, const char* slot, const QString &help,
	KopeteCommandHandler::CommandType type, const QString &formatString, uint minArgs, int maxArgs )
{
	m_command = command;
	m_help = help;
	m_type = type;
	m_formatString = formatString;
	m_minArgs = minArgs;
	m_maxArgs = maxArgs;

	if(  m_type == KopeteCommandHandler::Normal )
	{
		QObject::connect( this, SIGNAL( handleCommand( const QString &, KopeteMessageManager *) ),
			parent(), slot );
	}

	QObject::connect( this, SIGNAL( activated() ), this, SLOT( slotAction() ) );
}

void KopeteCommand::slotAction()
{
	KopeteMessageManager *manager = KopeteMessageManagerFactory::factory()->activeView()->msgManager();

	QString args;
	if( m_minArgs > 0 )
	{
		args = KInputDialog::getText( i18n("Enter Arguments"), i18n("Enter the arguments to %1:").arg(m_command) );
		if( args.isNull() )
			return;
	}

	processCommand( args, manager, true );
}

void KopeteCommand::processCommand( const QString &args, KopeteMessageManager *manager, bool gui )
{
	QStringList mArgs = KopeteCommandHandler::parseArguments( args );
	if( mArgs.count() < m_minArgs )
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
		if( m_type == KopeteCommandHandler::UserAlias ||
			m_type == KopeteCommandHandler::SystemAlias )
		{
			QString formatString = m_formatString;
			if( formatString.contains( QString::fromLatin1("%s") ) )
				formatString.replace( QString::fromLatin1("%s"), args );
			else
			{
				while( mArgs.count() > 0 )
				{
					formatString = formatString.arg( mArgs.front() );
					mArgs.pop_front();
				}
			}

			kdDebug(14010) << "New Command after processing alias: " << formatString << endl;

			KopeteCommandHandler::commandHandler()->processMessage( QString::fromLatin1("/") + formatString, manager );
		}
		else
		{
			emit( handleCommand( args, manager ) );
		}
	}
}

void KopeteCommand::printError( const QString &error, KopeteMessageManager *manager, bool gui ) const
{
	if( gui )
	{
		KMessageBox::error( Kopete::UI::Global::mainWidget(), error, i18n("Command Error") );
	}
	else
	{
		KopeteMessage msg( manager->user(), manager->members(), error,
			KopeteMessage::Internal, KopeteMessage::PlainText );
		manager->appendMessage( msg );
	}
}

#include "kopetecommand.moc"
