/*
   client.h - Papillon Client to Windows Live Messenger.

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
#ifndef PAPILLONCLIENT_H
#define PAPILLONCLIENT_H

#include <QtCore/QObject>
#include <papillon_macros.h>

namespace Papillon
{

class Connector;
class Connection;
class SecureStream;
class Task;
class Transfer;
class MimeHeader;

// TODO APIDOX, add a reference about connector model.
/**
 * @brief Client to Windows Live Messenger.
 * This is the main interface between the client application (ex: Kopete, papillon-telepathy, etc...) and
 * the Windows Live Messenger service.
 *
 * Papillon::Client need a valid Papillon::Connector objet to be used.
 * Set the client info with setClientInfo() before attempt to connect to Windows Live Messenger service.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
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

signals:
	/**
	 * Emitted when Client is connected, but not logged, to Windows Live Messenger's service.
	 */
	void connected();
	/**
	 * Emitted when Client got disconnected from Windows Live Messenger's service.
	 */
	void disconnected();

public slots:
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
