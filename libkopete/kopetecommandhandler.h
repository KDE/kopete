
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

#ifndef _KOPETECOMMANDHANDLER_H_
#define _KOPETECOMMANDHANDLER_H_

#include <qdict.h>
#include <kshortcut.h>
#include "kopetemessage.h"

class KopeteCommand;
class KopeteMessageManager;
class KProcess;
class KopetePlugin;
class KopeteProtocol;
class KopeteView;

struct CommandHandlerPrivate;

typedef QDict<KopeteCommand> CommandList;

/**
 * @author Jason Keirstead   <jason@keirstead.org>
 *
 * The KopeteCommandHandler can handle /action like messages
 */
class KopeteCommandHandler : public QObject
{
	friend class KopeteCommandGUIClient;

	Q_OBJECT

	public:
		/**
		 * an enum defining the type of a command
		 */
		enum CommandType { Normal, SystemAlias, UserAlias, Undefined };

		/**
		 * Returns a pointer to the command handler
		 */
		static KopeteCommandHandler *commandHandler();

		/**
		 * \brief Register a command with the command handler. 
		 *
		 * Command matching is case insensitive. All commands are registered,
		 * regardless of whether  or not they are already handled by another 
		 * handler. This is so that if the first plugin is unloaded, the next 
		 * handler in the sequence will handle the command. However, there are
		 * certain commands which are reserved (internally handled by the 
		 * KopeteCommandHandler). These commands can also be overridden by 
		 * registering a new duplicate command.
		 *
		 * @param parent The plugin who owns this command
		 * @param command The command we want to handle, not including the '/'
		 * @param handlerSlot The slot used to handle the command. This slot must
		 *   accept two parameters, a QString of arguments, and a KopeteMessageManager
		 *   pointer to the manager under which the command was sent.
		 * @param help An optional help string to be shown when the user uses
		 *   /help \<command\>
		 */
		void registerCommand( QObject *parent, const QString &command, const char* handlerSlot,
			const QString &help = QString::null, uint minArgs = 0, int maxArgs = -1,
			const KShortcut &cut = 0, const QString &pix = QString::null );

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
		 */
		void registerAlias( QObject *parent, const QString &alias, const QString &formatString,
			const QString &help = QString::null, CommandType = SystemAlias, uint minArgs = 0,
			int maxArgs = -1, const KShortcut &cut = 0, const QString &pix = QString::null );

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
		bool processMessage( KopeteMessage &msg, KopeteMessageManager *manager );

		/**
		 * \brief Process a message to see if any commands should be handled
		 *
		 * \see processMessage( KopeteMessage &msg, KopeteMessageManager *manager)
		 * \param msg A QString contain the message
		 * \param manager the KopeteMessageManager who will own the message
		 * \return true if the command was handled, false if the command was not handled.
		 */
		bool processMessage( const QString &msg, KopeteMessageManager *manager );

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

	private slots:
		void slotPluginLoaded( KopetePlugin * );
		void slotPluginDestroyed( QObject * );
		void slotExecReturnedData(KProcess *proc, char *buff, int bufflen );
		void slotExecFinished(KProcess *proc);
		void slotViewCreated( KopeteView *view );

		void slotHelpCommand( const QString & args, KopeteMessageManager *manager );
		void slotClearCommand( const QString & args, KopeteMessageManager *manager );
		void slotPartCommand( const QString & args, KopeteMessageManager *manager );
		void slotCloseCommand( const QString & args, KopeteMessageManager *manager );
		//void slotMeCommand( const QString & args, KopeteMessageManager *manager );
		void slotExecCommand( const QString & args, KopeteMessageManager *manager );
		void slotAwayCommand( const QString & args, KopeteMessageManager *manager );
		void slotAwayAllCommand( const QString & args, KopeteMessageManager *manager );
		void slotSayCommand( const QString & args, KopeteMessageManager *manager );

	private:
		/**
		 * Helper function. Returns all the commands that can be used by a KMM of this protocol
		 * (all non-protocol commands, plus this protocols commands)
		 */
		CommandList commands( KopeteProtocol * );

		/**
		 * Helper function for commands()
		 */
		void addCommands( CommandList &from, CommandList &to, CommandType type = Undefined );

		KopeteCommandHandler();
		~KopeteCommandHandler();

		static CommandHandlerPrivate *p;
};

#endif
