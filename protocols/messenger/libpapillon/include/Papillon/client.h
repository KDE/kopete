/*
   client.h - Papillon Client to Windows Live Messenger.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONCLIENT_H
#define PAPILLONCLIENT_H

#include <QtCore/QObject>
#include <Papillon/Macros>
#include <Papillon/Enums>

namespace Papillon
{

class Connector;
class Connection;
class SecureStream;
class Task;
class Transfer;
class MimeHeader;
class StatusMessage;
class ContactList;
class UserContact;

// TODO APIDOX, add a reference about connector model.
/**
 * @brief Client to Windows Live Messenger service.
 *
 * This is the main interface between the client application (ex: Kopete, telepathy-papillon, etc.) and
 * the Windows Live Messenger service.
 *
 * @section Setup Setting up Client object
 * First of all, you must initialize a Connector object. A Connector object is a abstract representation of a socket.
 * For convience, libpapillon include a QtConnector object, like the follwing example:
@code
Papillon::Client *client = new Client(new QtConnector(qObjectParent), qObjectParent);
@endcode
 *
 * @section Login Logging to Windows Live Messenger
 * After setting up the Client object, you must set some information before attemping to connect
 * like the passport identifier, the password and optionally the initial online status. You can
 * set a different server using setServer(). An example of login:
@code
connect(client, SIGNAL(connected()), this, SLOT(papillonConnected()));
client->userContact()>setLoginInformation( "test@passport.com", "password" );
client->setServer("messenger.hotmail.com", 1863);
client->connectToServer( Papillon::OnlineStatus::Busy );
@endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT Client : public QObject
{
	Q_OBJECT
public:
	/**
	 * Create a new Client.
	 * @param connector Connector to use.
	 * @param parent QObject parent.
	 */
	Client(Connector *connector, QObject *parent = 0);
	/**
	 * d-tor.
	 */
	~Client();

	/**
	 * @brief Create a new SecureStream ready to be used.
	 *
	 * You must delete the SecureStream instance yourself.
	 *
	 * @return the new SecureStream instance.
	 */
	SecureStream *createSecureStream();

	/**
	 * @brief Create and a setup a new connection
	 * @return the new Connection instance.
	 */
	Connection *createConnection();

	/**
	 * @brief Get the user contact.
	 * @return UserContact instance for this Client.
	 */
	UserContact *userContact();

	/**
	 * @brief Get the contact list instance.
	 *
	 * Initially, the contact list isn't loaded. Fetching of the 
	 * contact list will be called when necessary.
	 *
	 * @return the ContactList instance.
	 */
	ContactList *contactList();

	/**
	 * @brief Get the current connection to Notification server (NS)
	 *
	 * Use its rootTask when creating tasks for the notification server yourself.
	 * @return Notification server Connection instance.
	 */
	Connection *notificationConnection();

signals:
	/**
	 * Emitted when Client is connected, but not logged, to Windows Live Messenger's service.
	 */
	void connected();
	/**
	 * Emitted when Client got disconnected from Windows Live Messenger's service.
	 */
	void disconnected();

	// TODO: Move those signals in Contact class
	/**
	 * Emitted when a contact change his status
	 * @param contactId Contact ID
	 * @param status Contact's new online status.
	 */
	void contactStatusChanged(const QString &contactId, Papillon::OnlineStatus::Status status);

	/**
	 * Emitted when a contact has updated his status message.
	 * @param contactId Contact ID
	 * @param statusMessage Updated status message.
	 */
	void contactStatusMessageChanged(const QString &contactId, const Papillon::StatusMessage &newStatusMessage);

public slots:
	/**
	 * @brief Set an alternative login server.
	 *
	 * Use this method if you want to change the default server.
	 * @param server Alternative server IP address or domain name.
	 * @param port Alternative server TCP port.
	 */
	void setServer(const QString &server, quint16 port);

	/**
	 * @brief Connect to Windows Live Messenger
	 *
	 * You can optionally pass the initial online status of the user contact.
	 *
	 * You must set login information before calling this method. See @ref Login
	 *
	 * @param initialStatus Initial login status
	 */
	void connectToServer(Papillon::OnlineStatus::Status initialStatus = Papillon::OnlineStatus::Online);

	// FIXME: Maybe remove login or put it in private section
	/**
	 * @brief Start the login process.
	 * Make sure that you setup client information with setClientInfo()
	 */
	void login();
	
//BEGIN Private Task slots
private slots:
	/**
	 * Result of Login process.
	 * @param task the LoginTask.
	 */
	void loginResult(Papillon::Task *task);
	/**
	 * @internal
	 * Redirect notification connection given by login task.
	 * @param server new Notification server.
	 * @param port new Notification server port.
	 */
	void loginRedirect(const QString &server, quint16 port);

	/**
	 * @internal
	 * Set the passport auth ticket.
	 * @param profileMessage initial profile "message"
	 */
	void gotInitalProfile(const Papillon::MimeHeader &profileMessage);

	/**
	 * @internal
	 * A contact status changed.
	 * @param contactId His contact ID
	 * @param status His new online status.
	 */
	void slotContactStatusChanged(const QString &contactId, Papillon::OnlineStatus::Status status);

	/**
	 * @internal
	 * A contact status message changed.
	 * @param contactId His contact ID
	 * @param newStatusMessage His new status message.
	 */
	void slotContactStatusMessageChanged(const QString &contactId, const Papillon::StatusMessage &newStatusMessage);
//END Private Task slots

//BEGIN Private Normal slots
private slots:
	/**
	 * @internal
	 * Init the watch tasks for Notification connection.
	 * Called after being connected to the server.
	 */	
	void initNotificationTasks();
//END Private Normal slots

private:
	/**
	 * @internal
	 * Internal command used for debugging. (DO NOT USE THAT METHOD unless)
	 * Write a command on Notification server. 
	 * @param command the command.
	 */
	void writeCommand(Transfer *command);

	class Private;
	Private *d;
};

}

#endif
