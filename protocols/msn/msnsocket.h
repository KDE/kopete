/*
    msnsocket.h - Base class for the sockets used in MSN

    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

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
#include <qvaluelist.h>

#include <kopete_export.h>

namespace KNetwork {
  class KBufferedSocket;
  class KServerSocket;
}

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * MSNSocket encapsulates the common functionality shared by the Dispatch
 * Server, the Notification Server and the Switchboard Server. It is
 * inherited by the various specialized classes.
 */
class KDE_EXPORT MSNSocket : public QObject
{
	Q_OBJECT

public:
	MSNSocket(QObject* parent=0l);
	~MSNSocket();

	/**
	 * Asynchronously read a block of data of the specified size. When the
	 * data is available, the blockRead() signal will be emitted with the
	 * data as parameter.
	 *
	 * NOTE: As the block queue takes precedence over the line-based
	 *       command-processing this method can effectively block all
	 *       communications when passed a wrong length!
	 */
	void readBlock( uint len );

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
	enum LookupStatus { Processing, Success, Failed };

	OnlineStatus onlineStatus() { return m_onlineStatus; }

	/*
	 * return the local ip.
	 * Used for filetransfer
	 */
	QString getLocalIP();

public slots:
	void connect( const QString &server, uint port );
	virtual void disconnect();


	/**
	 * Send an MSN command to the socket
	 *
	 * For debugging it's convenient to have this method public, but using
	 * it outside this class is deprecated for any other use!
	 *
	 * The size of the body (if any) is automatically added to the argument
	 * list and shouldn't be explicitly specified! This size is in bytes
	 * instead of characters to reflect what actually goes over the wire.
	 *
	 * if the param binary is set to true, then, the body is send as a binary message
	 *
	 * return the id
	 */
	int sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addId = true, const QByteArray &body = QByteArray() , bool binary=false );

signals:
	/**
	 * A block read is ready.
	 * After this the normal line-based reads go on again
	 */
	void blockRead( const QString &block );
	void blockRead( const QByteArray &block );

	/**
	 * The online status has changed
	 */
	void onlineStatusChanged( MSNSocket::OnlineStatus status );

	/**
	 * The connection failed
	 */
	void connectionFailed();

	/**
	 * The connection was closed
	 */
	void socketClosed();

	/**
	 * The socket just sent a command from the queue. This signal is used in
	 * the notify socket to reset the keepalive timer.
	 */
	void commandSent();

protected:
	/**
	 * Convenience method: escape spaces with '%20' for use in the protocol.
	 * Doesn't escape any other sequence.
	 */
	QString escape( const QString &str );

	/**
	 * And the other way round...
	 */
	QString unescape( const QString &str );

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

	/**
	 * Used in MSNFileTransferSocket
	 */
	virtual void bytesReceived( const QByteArray & );
	bool accept( KNetwork::KServerSocket * );
	void sendBytes( const QByteArray &data );

	const QString &server() { return m_server; }
	uint port() { return m_port; }

	/**
	 * The last confirmed ID by the server
	 */
	//uint m_lastId;

private slots:
	void slotDataReceived();
	/**
	 * If the socket emits a connectionFailed() then this slot is called
	 * to handle the error.
	 */
	void slotSocketError( int error );

	/*
	 * Calls connectDone() when connection is successfully established.
	 */
	void slotConnectionSuccess();

	/**
	 * Sets m_lookupProgress to 'Finished' if count > 0 or 'Failed' if count = 0.
	 */
	void slotHostFound( );

	/**
	 * Check if new lines of data are available and process the first line
	 */
	void slotReadLine();

	void slotSocketClosed();

protected slots:
	virtual void slotReadyWrite();

private:
	/**
	 * Check if we're waiting for a block of raw data. Emits blockRead()
	 * when the data is available.
	 * Returns true when still waiting and false when there is no pending
	 * read, or when the read is successfully handled.
	 */
	bool pollReadBlock();

	/**
	 * The id of the message sent to the MSN server. This ID will increment
	 * for each subsequent message sent.
	 */
	uint m_id;

	/**
	 * Queue of pending commands (should be mostly empty, but is needed to
	 * send more than one command to the server)
	 */
	QValueList<QCString> m_sendQueue;

	/**
	 * Parse a single line of data.
	 * Will call either parseCommand or handleError depending on the type of
	 * data received.
	 */
	void parseLine( const QString &str );

	KNetwork::KBufferedSocket *m_socket;
	OnlineStatus m_onlineStatus;

	QString m_server;
	uint m_port;

	/**
	 * The size of the requested block for block-based reads
	 */
	uint m_waitBlockSize;

	class Buffer : public QByteArray
	{
	public:
		Buffer( unsigned size = 0 );
		~Buffer();
		void add( char *str, unsigned size );
		QByteArray take( unsigned size );
	};

	Buffer m_buffer;
};

#endif

