
/*
    kopetecommand.h - Command

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

#ifndef __KOPETECOMMAND_H__
#define __KOPETECOMMAND_H__

#include <qobject.h>
#include "kopetecommandhandler.h"

class KopeteMessageManager;

class KopeteCommand : public QObject
{
	Q_OBJECT

	public:
		/**
		 * Creates a KopeteCommand object
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command we want to handle, not including the '/'
		 * @param handlerSlot The slot used to handle the command. This slot must
		 *   accept two parameters, a QString of arguments, and a KopeteMessageManager
		 *   pointer to the Manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help <command>
		 * @param type If this command is an alias, and what type
		 * @param formatString The formatString of the alias if any
		 */
		 KopeteCommand( QObject *parent, const QString &command, const char* handlerSlot,
		 	const QString &help = QString::null, KopeteCommandHandler::CommandType type = KopeteCommandHandler::Normal, 
			const QString &formatString = QString::null );

		/**
		 * Process this command
		 */
		void processCommand( const QString &args, KopeteMessageManager *manager );

		/**
		 * Returns the command this object handles
		 */
		 const QString &command() const { return m_command; };

		 /**
		  * Returns the help string for this command
		  */
		 const QString &help() const { return m_help; };
		 
		 /**
		  * Returns the type of the command
		  */
		 const KopeteCommandHandler::CommandType type() const { return m_type; };

	signals:
		/**
		 * Emitted whenever a command is handled by this object. When a command
		 * has been handled, all processing on it stops by the command handler
		 * (a command cannot be handled twice)
		 */
		void handleCommand( const QString &args, KopeteMessageManager *manager );

	private:
		QString m_command;
		QString m_help;
		QString m_formatString;
		KopeteCommandHandler::CommandType m_type;
};

#endif
