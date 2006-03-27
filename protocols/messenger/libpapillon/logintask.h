/*
   logintask.h - Windows Live Messenger Login Task

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONLOGINTASK_H
#define PAPILLONLOGINTASK_H

#include <task.h>
#include <papillon_macros.h>

namespace Papillon 
{

class Transfer;
class TweenerHandler;
/**
 * Login on Windows Live Messenger.
 *
 * First you create the Login Task, you must set the user info.
 * @code
 * LoginTask *login = new LoginTask( connection->rootTask() );
 * login->setUserInfo("example@passport.com", "password");
 * connect(login, SIGNAL(finished(Papillon::Task*)), ...);
 * connect(login, SIGNAL(redirection(const QString &, quint16)), ...);
 * login->go();
 * @endcode
 *
 * Use success() to check if the login was successful.
 *
 * Use loginState() to check for the error if the login wasn't succesful.
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class PAPILLON_EXPORT LoginTask : public Task
{
	Q_OBJECT
public:
	/**
	 * Current state of the login process.
	 */
	enum LoginState 
	{
		/**
	 	 * Begin state, sending the version command.
		 */
		StateVersion,
		/**
		 * Sending the CVR command.
		 */
		StateCVR,
		/**
		 * Ask for the tweener ticket.
		 */
		StateTweenerInvite,
		/**
		 * Now confirm the tweener.
		 */
		StateTweenerConfirmed,
		/**
		 * Login was sucessful.
		 */
		StateFinish,
		/**
		 * A unknow error occured.
		 */
		StateError,
		/**
		 * Nexus service returned a bad passport error.
		 */
		StateBadPassword
	};

	/**
	 * Create a new Login Task.
	 * @param parent parent Task
	 */
	LoginTask(Task *parent);
	/**
	 * d-tor (duh)
	 */
	virtual ~LoginTask();

	/**
	 * Set the user info to log with.
	 * @param passportId passport id to log with.
	 * @param password user password.
	 * TODO: Use a QSecureArray to pass the password.
	 */
	void setUserInfo(const QString &passportId, const QString &password);

	/**
	 * Inherited from Task.
	 * Proceed the given transfer according to the current LoginState.
	 * Move to the next login step if required.
	 * @param transfer given Transfer.
	 * @return true if we proceed this transfer.
	 */
	virtual bool take(Transfer *transfer);

	/**
	 * Get the current state of the Login Task.
	 * @return the current LoginState.
	 */
	LoginState loginState() const;

protected:
	/**
	 * Inherited from Task.
	 * Helper method to check if the Transfer is for us.
	 * @param transfer give Transfer.
	 * @return true if the Transfer is for us.
	 */
	virtual bool forMe(Transfer *transfer);
	/**
	 * Inherited from Task.
	 * Start the login process.
	 */
	virtual void onGo();

signals:
	/**
	 * Emitted when the login need to redirect to another server.
	 * Resulting slot should close the notification connection, connect to the new one and recreate a new Login Task.
	 */
	void redirection(const QString &server, quint16 port);

private slots:
	void sendVersionCommand();
	void sendCvrCommand();
	void sendTweenerInviteCommand();
	void sendTweenerConfirmation();
	void ticketReceived(TweenerHandler *tweenerHandler);

private:
	class Private;
	Private *d;
};

}

#endif
