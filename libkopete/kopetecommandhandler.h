
/*
    kopetecommandhandler.h - Command Handler

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

#ifndef _KOPETECOMMANDHANDLER_H_
#define _KOPETECOMMANDHANDLER_H_

#include "kopetemessage.h"

#include <qptrlist.h>
#include <qdict.h>
#include <qmap.h>

class KopetePlugin;
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
		 *   accept two paramaters, a QString of arguments, and a KopeteMessageManager
		 *   pointer to the Manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help <command>
		 */
		 KopeteCommand( KopetePlugin *parent, const QString &command, const char* handlerSlot,
		 	const QString &help );

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
		KopetePlugin *m_plugin;
};

typedef QDict<KopeteCommand> CommandList;
typedef QMap<KopetePlugin*, CommandList> PluginCommandMap;

class KopeteCommandHandler : public QObject
{
	Q_OBJECT

	public:
		KopeteCommandHandler();
		~KopeteCommandHandler();

		/**
		 * Returns a pointer to the command handler
		 */
		static KopeteCommandHandler *commandHandler();

		/**
		 * Registers a command with the command handler. Command matching is
		 * case insensitive. All commands are registered, regardless of whether
		 * or not they are already handled by another handler. This is so that
		 * if the first plugin is unloaded, the next handler in the sequence
		 * will handle the command. However, there are certain commands which
		 * are reserved, which will not be registered. These commands include
		 * "help", "clear", and "exec", and others. Reserved commands can be
		 * discovered using @ref reserved()
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command we want to handle, not including the '/'
		 * @param handlerSlot The slot used to handle the command. This slot must
		 *   accept two paramaters, a QString of arguments, and a KopeteMessageManager
		 *   pointer to the manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help <command>
		 */
		void registerCommand( KopetePlugin *parent, const QString &command, const char* handlerSlot,
			const QString &help = QString::null );

		/**
		 * Unregisters a command. When a plugin unloads, all commands are
		 * automaticlly unregistered and deleted. This function should only
		 * be called in the case of a plugin which loads and unloads commands
		 * dynamicly.
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command to unload
		 */
		void unregisterCommand( KopetePlugin *parent, const QString &command );

		/**
		 * Processes a message to see if any commands should be handled
		 *
		 * @param msg The message to process
		 * @param manager The manager who owns this message
		 * @return True if the command was handled, false if not
		 */
		bool processMessage( KopeteMessage &msg, KopeteMessageManager *manager );

		/**
		 * Parses a string of command arguments into a QStringList. Quoted
		 * blocks within the arguments string are treated as one argument.
		 */
		static QStringList parseArguments( const QString &args );

		/**
		 * Used to discover if a command is already handled by a handler
		 *
		 * @param command The command to check
		 * @return True if the command is already being handled, False if not
		 */
		bool commandHandled( const QString &command );

		/**
		 * Returns the list of reserved commands.
		 *
		 * @return A list of commands reserved for internal Kopete use
		 */
		const QStringList &reserved() const { return reservedCommands; };

	private slots:
		void slotPluginLoaded( KopetePlugin * );
		void slotPluginDestroyed( QObject * );

	private:
		QStringList reservedCommands;
		void reservedCommand( const QString &command, const QStringList &args, KopeteMessageManager *manager );
		CommandList commands( KopeteProtocol * );
		PluginCommandMap pluginCommands;
		static KopeteCommandHandler *s_handler;
};

#endif
