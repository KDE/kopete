/*
    msnsocket.h - Base class for the sockets used in MSN

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001 by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNSOCKET_H
#define MSNSOCKET_H

#include <qobject.h>

class KExtendedSocket;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * MSNSocket encapsulates the common functionality shared by the Dispatch
 * Server, the Notification Server and the Switchboard Server. It is
 * inherited by the various specialized classes.
 */
class MSNSocket : public QObject
{
	Q_OBJECT

public:
	MSNSocket();
	~MSNSocket();

	/**
	 * Asynchronously read a block of data of the specified size. When the
	 * data is available, the blockRead() signal will be emitted with the
	 * data as parameter.
	 * NOTE: As the block queue takes precedence over the line-based
	 *       command-processing this method can effectively block all
	 *       communications when passed a wrong length!
	 */
	void readBlock(uint len);

	/**
	 * OnlineStatus encapsulates the 4 states a connection can be in,
	 * Connecting, Connected, Disconnecting, Disconnected. Connecting
	 * and Disconnecting are in the default implementation not used,
	 * because the socket connect is an atomic operation and not yet
	 * performed asynchronously.
	 * In derived classes, like the Notification Server, this state is
	 * actively used, because merely having a socket connection established
	 * by no means indicates we're actually online - the rest of the
	 * handshake likely has to follow first.
	 */
	enum OnlineStatus { Connecting, Connected, Disconnecting, Disconnected };

	void connect( const QString &server, uint port );
	virtual void disconnect();

	OnlineStatus onlineStatus() { return m_onlineStatus; }

signals:
	/**
	 * A block read is ready.
	 * After this the normal line-based reads go on again
	 */
	void blockRead( const QString &block );

	/**
	 * The online status has changed
	 */
	void onlineStatusChanged( MSNSocket::OnlineStatus status );

protected:
	/**
	 * Send an MSN command to the socket
	 */
	void sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addNewLine = true, bool addId = true );

	/**
	 * Set the online status. Emits onlineStatusChanged.
	 */
	void setOnlineStatus( OnlineStatus status );

	/**
	 * This method is called directly before the socket will actually connect.
	 * Override in derived classes to setup whatever is needed before connect.
	 */
	virtual void aboutToConnect();

	/**
	 * Directly after the connect, this method is called. The default
	 * implementation sets the OnlineStatus to Connected, be sure to override
	 * this if a handshake is required.
	 */
	virtual void doneConnect();

	/**
	 * Directly after the disconnect, this method is called before the actual
	 * cleanup takes place. The socket is close here. Cleanup internal
	 * variables here.
	 */
	virtual void doneDisconnect();

	/**
	 * Handle an MSN error condition.
	 * The default implementation displays a generic error message and
	 * closes the connection. Override to allow more graceful handling and
	 * possibly recovery.
	 */
	virtual void handleError( uint code, uint id );

	/**
	 * Handle an MSN command response line.
	 * This method is pure virtual and *must* be overridden in derived
	 * classes.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data ) = 0;

	const QString &server() { return m_server; }
	uint port() { return m_port; }

private slots:
	void slotDataReceived();
	//void slotSocketError(int error);

	/**
	 * Check if new lines of data are available and process the first line
	 */
	void slotReadLine();

private:
	/**
	 * Check if we're waiting for a block of raw data. Emits blockRead()
	 * when the data is available.
	 * Returns true when still waiting and false when there is no pending
	 * read, or when the read is succesfully handled.
	 */
	bool pollReadBlock();

	/**
	 * The id of the message sent to the MSN server. This ID will increment
	 * for each subsequent message sent.
	 */
	uint m_id;

	/**
	 * Parse a single line of data.
	 * Will call either parseCommand or handleError depending on the type of
	 * data received.
	 */
	void parseLine( const QString &str );

	KExtendedSocket *m_socket;
	OnlineStatus m_onlineStatus;

	QString m_buffer;
	QString m_server;
	uint m_port;

	/**
	 * The size of the requested block for block-based reads
	 */
	uint m_waitBlockSize;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

