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

// TODO APIDOX, add a reference about connector model.
/**
 * @brief Client to Windows Live Messenger service.
 * This is the main interface between the client application (ex: Kopete, telepathy-papillon, etc.) and
 * the Windows Live Messenger service.
 *
 * Papillon::Client need a valid Papillon::Connector objet to be used.
 * Set the client info with setClientInfo() before attempt to connect to Windows Live Messenger service.
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
	 * @brief Set client Info
	 * @param passportId Client Passport ID
	 * @param password Client password.
	 * TODO: Use a ClientInfo data class.
	 * TODO: Use QSecureArray for password.
	 */
	void setClientInfo(const QString &passportId, const QString &password);

	/**
	 * @brief Get the Passport auth ticket.
	 * This is used to identity us when doing SOAP requests.
	 * @return the Passport auth ticket.
	 */
	QString passportAuthTicket() const;

	/**
	 * @brief Get the contact list instance.
	 *
	 * Initially, the contact list isn't loaded. Fetching of the 
	 * contact list will be called when necessary.
	 *
	 * @return the ContactList instance.
	 */
	ContactList *contactList();

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
	// FIXME: Maybe merge connectToServer and setInitialOnlineStatus
	// FIXME: Maybe remove login or put it in private section
	/**
	 * @brief Connect to Windows Live Messenger
	 * If no arguments are passed, it use the default server and port used by
	 * Windows Live Messenger official client.
	 * Note that it doesn't login to the server. it just connects.
	 * @param server The Windows Live Messenger server, if you want to override it.
	 * @param port the Windows Live Messenger server port, if you want to override it.
	 */
	void connectToServer(const QString &server = QString(), quint16 port = 0);
	/**
	 * @brief Start the login process.
	 * Make sure that you setup client information with setClientInfo()
	 */
	void login();

	/**
	 * @brief Set the initial online status.
	 * This is the first online status that will be set on server.
	 *
	 * @param status the initial online status
	 */
	void setInitialOnlineStatus(Papillon::OnlineStatus::Status status);
	
	// TODO: Move these methods to ClientInfo or UserContact
	/**
	 * @brief Change our current online status
	 * @param status Given online status
	 */
	void changeOnlineStatus(Papillon::OnlineStatus::Status status);

	/**
	 * @brief Set the personal information to be updated on server.
	 * @param type The type of the personal information it need to update on server.
	 * @param value New value for the given personal information. Set an empty string to reset the value.
	 */
	void setPersonalInformation(Papillon::ClientInfo::PersonalInformation type, const QString &value);

// Slots from tasks
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
