
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
#include <kaction.h>
#include "kopetecommandhandler.h"

namespace Kopete
{

class ChatSession;

class Command : public KAction
{
	Q_OBJECT

	public:
		/**
		 * Creates a Kopete::Command object
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command we want to handle, not including the '/'
		 * @param handlerSlot The slot used to handle the command. This slot must
		 *   accept two parameters, a QString of arguments, and a Kopete::ChatSession
		 *   pointer to the Manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help <i>command</i>
		 * @param type If this command is an alias, and what type
		 * @param formatString The formatString of the alias if any
		 * @param minArgs Minimum number of arguments
		 * @param maxArgs Maximum number of arguments
		 * @param cut The shortcut for the command
		 * @param pix The icon to use for the command
		 */
		 Command( QObject *parent, const QString &command, const char* handlerSlot,
		 	const QString &help = QString::null, CommandHandler::CommandType type = CommandHandler::Normal, const QString &formatString = QString::null,
			uint minArgs = 0, int maxArgs = -1, const KShortcut &cut = 0,
			const QString &pix = QString::null );

		/**
		 * Process this command
		 */
		void processCommand( const QString &args, ChatSession *manager, bool gui = false );

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
		 const CommandHandler::CommandType type() const { return m_type; };

	signals:
		/**
		 * Emitted whenever a command is handled by this object. When a command
		 * has been handled, all processing on it stops by the command handler
		 * (a command cannot be handled twice)
		 */
		void handleCommand( const QString &args, Kopete::ChatSession *manager );

	private slots:
		/**
		 * Connected to our activated() signal
		 */
		void slotAction();

	private:
		void init( const QString &command, const char* slot, const QString &help,
			CommandHandler::CommandType type, const QString &formatString,
			uint minArgs, int maxArgs );

		void printError( const QString &error, ChatSession *manager, bool gui = false ) const;

		QString m_command;
		QString m_help;
		QString m_formatString;
		uint m_minArgs;
		int m_maxArgs;
		bool m_processing;
		CommandHandler::CommandType m_type;
};

}

#endif
