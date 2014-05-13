/*
    kopetecommandhandler.h - Command Handler

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

#ifndef KOPETECOMMANDHANDLER_H
#define KOPETECOMMANDHANDLER_H

#include <QtCore/QMultiHash>

#include <kshortcut.h>
#include <kprocess.h>
#include "kopetemessage.h"

#include "kopete_export.h"

struct CommandHandlerPrivate;

class KopeteView;
class KopeteCommandGUIClient;

namespace Kopete
{

class ChatSession;
class Plugin;
class Protocol;
class Command;

typedef QMultiHash<QString, Command*> CommandList;

/**
 * @author Jason Keirstead   <jason@keirstead.org>
 *
 * The Kopete::CommandHandler can handle /action like messages
 */
class KOPETE_EXPORT CommandHandler : public QObject
{
	friend class ::KopeteCommandGUIClient;

	Q_OBJECT

	public:
		/**
		 * an enum defining the type of a command
		 */
		enum CommandType { Normal, SystemAlias, UserAlias, Undefined };

		/**
		 * Returns a pointer to the command handler
		 */
		static CommandHandler *commandHandler();

		/**
		 * \brief Register a command with the command handler.
		 *
		 * Command matching is case insensitive. All commands are registered,
		 * regardless of whether  or not they are already handled by another
		 * handler. This is so that if the first plugin is unloaded, the next
		 * handler in the sequence will handle the command. However, there are
		 * certain commands which are reserved (internally handled by the
		 * Kopete::CommandHandler). These commands can also be overridden by
		 * registering a new duplicate command.
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command we want to handle, not including the '/'
		 * @param handlerSlot The slot used to handle the command. This slot must
		 *   accept two parameters, a QString of arguments, and a Kopete::ChatSession
		 *   pointer to the manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help \<command\>
		 * @param minArgs the minimum number of arguments for this command
		 * @param maxArgs the maximum number of arguments this command takes
		 * @param cut a default keyboard shortcut
		 * @param pix icon name, the icon will be shown in menus
		 */
		void registerCommand( QObject *parent, const QString &command, const char* handlerSlot,
			const QString &help = QString(), uint minArgs = 0, int maxArgs = -1,
			const KShortcut &cut = KShortcut(), const QString &pix = QString() );

		/**
		 * \brief Register a command alias.
		 *
		 * @param parent The plugin who owns this alias
		 * @param alias The command for the alias
		 * @param formatString This is the string that will be transformed into another
		 *    command. The formatString should begin with an already existing command,
		 *    followed by any other arguments. The variables %1, %2... %9 will be substituted
		 *    with the arguments passed into the alias. The variable %s will be substituted with
		 *    the entire argument string
		 * @param help An optional help string to be shown when the user uses
		 *   /help \<command\>
		 * @param minArgs the minimum number of arguments for this command
		 * @param maxArgs the maximum number of arguments this command takes
		 * @param cut a default keyboard shortcut
		 * @param pix icon name, the icon will be shown in menus
		 */
		void registerAlias( QObject *parent,
			const QString &alias,
			const QString &formatString,
			const QString &help = QString(),
			CommandType = SystemAlias,
			uint minArgs = 0,
			int maxArgs = -1,
			const KShortcut &cut = KShortcut(),
			const QString &pix = QString() );

		/**
		 * \brief Unregister a command.
		 *
		 * When a plugin unloads, all commands are automaticlly unregistered and deleted.
		 * This function should only be called in the case of a plugin which loads and
		 * unloads commands dynamically.
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command to unload
		 */
		void unregisterCommand( QObject *parent, const QString &command );

		/**
		 * \brief Unregister an alias.
		 *
		 * \see unregisterCommand( QObject *parent, const QString &command )
		 * @param parent The plugin who owns this alias
		 * @param alias The alais to unload
		 */
		void unregisterAlias( QObject *parent, const QString &alias );

		/**
		 * \brief Process a message to see if any commands should be handled
		 *
		 * @param msg The message to process
		 * @param manager The manager who owns this message
		 * @return True if the command was handled, false if not
		 */
		bool processMessage( Message &msg, ChatSession *manager );

		/**
		 * \brief Process a message to see if any commands should be handled
		 *
		 * \see processMessage( Kopete::Message &msg, Kopete::ChatSession *manager)
		 * \param msg A QString contain the message
		 * \param manager the Kopete::ChatSession who will own the message
		 * \return true if the command was handled, false if the command was not handled.
		 */
		bool processMessage( const QString &msg, ChatSession *manager );

		/**
		 * Parses a string of command arguments into a QStringList. Quoted
		 * blocks within the arguments string are treated as one argument.
		 */
		static QStringList parseArguments( const QString &args );

		/**
		 * \brief Check if a command is already handled
		 *
		 * @param command The command to check
		 * @return True if the command is already being handled, False if not
		 */
		bool commandHandled( const QString &command );

		/**
		 * \brief Check if a command is already handled by a spesific protocol
		 *
		 * @param command The command to check
		 * @param protocol The protocol to check
		 * @return True if the command is already being handled, False if not
		 */
		bool commandHandledByProtocol( const QString &command, Protocol *protocol);

	private slots:
		void slotPluginLoaded( Kopete::Plugin * );
		void slotPluginDestroyed( QObject * );
		void slotExecError( QProcess::ProcessError error );
		void slotExecFinished();
		void slotExecSendMessage( KProcess *proc, const QString &buffer );
		void slotViewCreated( KopeteView *view );

		void slotHelpCommand( const QString & args, Kopete::ChatSession *manager );
		void slotClearCommand( const QString & args, Kopete::ChatSession *manager );
		void slotPartCommand( const QString & args, Kopete::ChatSession *manager );
		void slotCloseCommand( const QString & args, Kopete::ChatSession *manager );
		void slotOpenLastUrl( const QString & args, Kopete::ChatSession *manager );
		//void slotMeCommand( const QString & args, Kopete::ChatSession *manager );
		void slotExecCommand( const QString & args, Kopete::ChatSession *manager );
		void slotAwayCommand( const QString & args, Kopete::ChatSession *manager );
		void slotAwayAllCommand( const QString & args, Kopete::ChatSession *manager );
		void slotSayCommand( const QString & args, Kopete::ChatSession *manager );

	private:
		/**
		 * Helper function. Returns all the commands that can be used by a KMM of this protocol
		 * (all non-protocol commands, plus this protocols commands)
		 */
		CommandList commands( Protocol * );

		/**
		 * Helper function for commands()
		 */
		void addCommands( CommandList &from, CommandList &to, CommandType type = Undefined );

		CommandHandler();
		~CommandHandler();

		static CommandHandlerPrivate *p;
};

}

#endif
