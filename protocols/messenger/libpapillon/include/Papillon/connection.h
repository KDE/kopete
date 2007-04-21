/*
   connection.h - Connection with a Messenger service.

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
#ifndef PAPILLONCONNECTION_H
#define PAPILLONCONNECTION_H

#include <QtCore/QObject>
#include <Papillon/Macros>

namespace Papillon 
{

class ClientStream;
class Transfer;
class Task;
class Client;
/**
 * @class Connection connection.h <Papillon/Connection>
 * @brief Connection encapsulate a connection to a Windows Live Messenger service.
 *
 * Contains the transaction ID reference count, the root Task. Dispatch also the transfers through the task.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT Connection : public QObject
{
	Q_OBJECT
public:
	/**
	 * Create a new connection
	 * @param stream the ClientStream (socket and core protocol encapsulation)
	 * @param parent QObject parent
	 */
	explicit Connection(ClientStream *stream, QObject *parent = 0);
	/**
	 * d-tor
	 */
	~Connection();

	/**
	 * Get the root task for this conenction.
	 * Root task is the parent Task for all task of this connection.
	 */
	Task *rootTask();

	/**
	 * Increment the transaction id and return its value.
	 * @return the current transaction id.
	 */
	int transactionId();

	/**
	 * Check if this connection is active.
	 * @return true if the connection is active.
	 */
	bool isConnected();

	/**
	 * Get the Client associated with this connection.
	 * @return the Client pointer.
	 */
	Client *client();
	/**
	 * Set the Client pointer.
	 * @param client Client pointer.
	 */
	void setClient(Client *client);

signals:
	/**
	 * Emiited when the connection gets opened.
	 */
	 void connected();
	/**
	 * Emitted when this connection gets disconnected.
	 */
	void disconnected();

public slots:
	/**
	 * Connect to the given service.
	 * @param server Hostname or IP address of the service
	 * @param port TCP port of the service.
	 */
	void connectToServer(const QString &server, quint16 port);
	/**
	 * End the connection.
	 */
	void disconnectFromServer();

	/**
	 * Send a transfer to the current Messenger service.
	 */
	void send(Transfer *transfer);

private slots:
	/**
	 * @internal
	 * Called when the stream has received a new Papillon::Transfer.
	 */
	void transferReceived();
	/**
	 * @internal
	 * Dispatch a transfer through the task.
	 * After, the transfer gets deleted.
	 */
	void dispatchTransfer(Transfer *currentTransfer);

	/**
	 * @internal
	 * Called when the connection is established to the service(server).
	 */
	void slotConnected();
	/**
	 * @internal
	 * Called when the connection gets disconnected (duh)
	 */
	void slotDisconnected();

private:
	class Private;
	Private *d;
};

}

#endif
