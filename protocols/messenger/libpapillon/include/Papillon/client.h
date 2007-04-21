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
 * @class Client client.h <Papillon/Client>
 * @brief Client to Windows Live Messenger service.
 *
 * This is the main interface between the client application (ex: Kopete, telepathy-papillon, etc.) and
 * the Windows Live Messenger service.
 *
 * @section Setup Setting up Client object
 * First of all, you must initialize a Connector object. A Connector object is a abstract representation of a socket.
 * For convience, libpapillon include a QtConnector object. After you must listen to connectioStatusChanged() signal to
 * be notified of connection status change, like connected and login. See the following example:
@code
Papillon::Client *client = new Client(new QtConnector(qObjectParent), qObjectParent);
connect(client, SIGNAL(connectionStatusChanged(Papillon::Client::ConnectionStatus)), this, SLOT(aSlot(Papillon::Client::ConnectionStatus)));
@endcode
 *
 * @section Login Logging to Windows Live Messenger
 * After setting up the Client object, you must set some information before attemping to connect
 * like the passport identifier, the password and optionally the initial presence. You can
 * set a different server using setServer(). An example of login:
@code
client->userContact()->setLoginInformation( "test@passport.com", "password" );
client->setServer("messenger.hotmail.com", 1863);
client->connectToServer( Papillon::Presence::Busy );
@endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT Client : public QObject
{
	Q_OBJECT
public:
	/**
	 * ConnectionStatus is the possible states of Client's connection status
	 */
	enum ConnectionStatus
	{
		/**
		 * Client is current disconnected
		 */
		Disconnected = 0,
		/**
		 * Client got a bad password
		 */
		LoginBadPassword,
		/**
		 * Client is currently connecting to Windows Live Messenger
		 */
		Connecting,
		/**
		 * Client is connected and now will proceed to login.
		 */
		Connected,
		/**
		 * Client is now logged in.
		 */
		LoggedIn
	};

	/**
	 * Create a new Client.
	 * @param connector Connector to use.
	 * @param parent QObject parent.
	 */
	explicit Client(Connector *connector, QObject *parent = 0);
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

	/**
	 * @brief Get the current connection status.
	 * @return Current connection status.
	 */
	Papillon::Client::ConnectionStatus connectionStatus() const;

signals:
	/**
	 * Emitted when Client's connection status has changed.
	 * @see ConnectionStatus
	 */
	void connectionStatusChanged(Papillon::Client::ConnectionStatus status);

	// TODO: Move those signals in Contact class
	/**
	 * Emitted when a contact change his online presence
	 * @param contactId Contact ID
	 * @param presence Contact's new online presence
	 */
	void contactPresenceChanged(const QString &contactId, Papillon::Presence::Status presence);

	/**
	 * Emitted when a contact has updated his status message.
	 * @param contactId Contact ID
	 * @param newStatusMessage Updated status message.
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
	 * You can optionally pass the initial presence of the user contact.
	 *
	 * You must set login information before calling this method. See @ref Login
	 *
	 * @param initialPresence Initial presence at login
	 */
	void connectToServer(Papillon::Presence::Status initialPresence = Papillon::Presence::Online);

	/**
	 * @brief Disconnect from Windows Live Messenger
	 * Close the connection and set the status to offline.
	 */
	void disconnectFromServer();

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
	 * A contact's presence has changed.
	 * @param contactId His contact ID
	 * @param presence His new presence.
	 */
	void slotContactPresenceChanged(const QString &contactId, Papillon::Presence::Status presence);

	/**
	 * @internal
	 * A contact's status message has changed.
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

	/**
	 * @internal
	 * We are connected to notification server
	 */
	void notificationConnected();

	/**
	 * @brief Start the login process.
	 * See @ref Login for more information.
	 */
	void login();
//END Private Normal slots

private:
	/**
	 * @internal
	 * Internal command used for debugging. (DO NOT USE THAT METHOD unless)
	 * Write a command on Notification server. 
	 * @param command the command.
	 */
	void writeCommand(Transfer *command);

	/**
	 * @internal
	 * Set a new connection status and emit connectionStatusChanged() signal.
	 * @param newStatus New connection status
	 */
	void setConnectionStatus(Papillon::Client::ConnectionStatus newStatus);

	class Private;
	Private *d;
};

}

#endif
