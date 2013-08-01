/*
    kopetecommand.cpp - Command

    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2005      by Michel Hermier <michel.hermier@wanadoo.fr>
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecommand.h"

#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

#include <kauthorized.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kicon.h>
#include <kmessagebox.h>

#include <qstringlist.h>

class Kopete::Command::Private
{
public:
	QString command;
	QString help;
	QString formatString;
	uint minArgs;
	int maxArgs;
	bool processing;
	Kopete::Command::Types types;
	QObject* parent;
};

Kopete::Command::Command( QObject *parent, const QString &command, const char* handlerSlot,
		const QString &help, Kopete::CommandHandler::CommandType type, const QString &formatString,
		uint minArgs, int maxArgs, const KShortcut &cut, const QString &pix )
	: KAction( KIcon(pix), command[0].toUpper() + command.right( command.length() - 1).toLower(), 0 )
	, d(new Private)
{
	setObjectName( command.toLower() + QString::fromLatin1("_command") );
	setShortcut( cut );
	d->parent = parent;
	connect(this, SIGNAL(triggered(bool)),this, SLOT(slotAction()));
	connect(parent,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	init( command, handlerSlot, help, type, formatString, minArgs, maxArgs );
}

Kopete::Command::~Command()
{
	delete d;
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

	if(m_type == Kopete::CommandHandler::Normal )
	{
		QObject::connect(this, SIGNAL(handleCommand(QString,Kopete::ChatSession*)),
			d->parent, slot );
	}
}

void Kopete::Command::slotAction()
{
	Kopete::ChatSession *manager = Kopete::ChatSessionManager::self()->activeView()->msgManager();

	QString args;
	if( m_minArgs > 0 )
	{
		args = KInputDialog::getText( i18n("Enter Arguments"), i18n("Enter the arguments to %1:", m_command) );
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
		printError( i18n("Alias \"%1\" expands to itself.", text() ), manager, gui );
	}
	else if( mArgs.count() < m_minArgs )
	{
		printError( i18np("\"%2\" requires at least %1 argument.",
			"\"%2\" requires at least %1 arguments.", m_minArgs,
			  text() ), manager, gui );
	}
	else if( m_maxArgs > -1 && (int)mArgs.count() > m_maxArgs )
	{
		printError( i18np("\"%2\" has a maximum of %1 argument.",
			"\"%2\" has a maximum of %1 arguments.", m_minArgs,
			  text() ), manager, gui );
	}
	else if( !KAuthorized::authorizeKAction( objectName() ) )
	{
		printError( i18n("You are not authorized to perform the command \"%1\".", text()), manager, gui );
	}
	else
	{
		m_processing = true;
		if( m_type == Kopete::CommandHandler::UserAlias ||
			m_type == Kopete::CommandHandler::SystemAlias )
		{
			QString formatString = m_formatString;

			// Translate %s to the whole string and %n to current display name

			formatString.replace( QString::fromLatin1("%n"), manager->myself()->displayName() );
			formatString.replace( QString::fromLatin1("%s"), args );

			// Translate %1..%N to word1..wordN
			while( mArgs.count() > 0 )
			{
				formatString = formatString.arg( mArgs.front() );
				mArgs.pop_front();
			}

			kDebug(14010) << "New Command after processing alias: " << formatString;

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
		Kopete::Message msg( manager->myself(), manager->members() );
		msg.setPlainBody(error);
		msg.setDirection( Kopete::Message::Internal );

		manager->appendMessage( msg );
	}
}

#include "kopetecommand.moc"
