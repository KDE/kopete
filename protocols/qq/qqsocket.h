/*
    qqsocket.h - Base class for the sockets used in QQ

	Copyright (c) 2006         Hui Jin				  <blueangel.jin@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef QQSOCKET_H
#define QQSOCKET_H

#include <qobject.h>
#include <QList>
#include <kopeteprotocol.h>

#include "kopete_export.h"

namespace KNetwork {
    class KBufferedSocket;
}


/**
 * @author Hui Jin <blueangle.jin@gmail.com>
 *
 * QQSocket encapsulates the common functionality, this is a preliminary 
 * implementation, only targets to UDP communication only.
 * The notification sever is inherited from this class.
 */
class QQ_EXPORT QQSocket : public QObject
{
	Q_OBJECT

public:
	QQSocket(QObject* parent=0l);
	~QQSocket();

	void sendPacket( const QByteArray& data );

	/* To make things more managable, only UDP is supported in this version */
	enum OnlineStatus { Connecting, Connected, Disconnecting, Disconnected };
	enum LookupStatus { Processing, Success, Failed };
	enum Transport { TcpTransport, HttpTransport };
	enum ErrorType { ErrorNormal, ErrorInternal, ErrorInformation, ErrorSorry };
	

	OnlineStatus onlineStatus() { return m_onlineStatus; }

	/*
	 * return the local ip.
	 */
	QString getLocalIP();

public slots:
	void connect( const QString &server, uint port );
	virtual void disconnect();

signals:
	/**
	 * The online status has changed
	 */
	void onlineStatusChanged( QQSocket::OnlineStatus status );

	/**
	 * The connection failed
	 */
	void connectionFailed();

	/**
	 * The connection was closed
	 */
	void socketClosed();

	/**
	 * A error has occurred. Handle the display of the message.
	 */
	void errorMessage( int type, const QString &msg );

protected:
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
	 * Handle an QQ error condition.
	 * The default implementation displays a generic error message and
	 * closes the connection. Override to allow more graceful handling and
	 * possibly recovery.
	 */
	virtual void handleError( uint code, uint id );

	/**
	 * Handle an QQ incoming packet.
	 * This method is pure virtual and *must* be overridden in derived
	 * classes.
	 */
	virtual void handleIncomingPacket( const QByteArray& data ) = 0;

	const QString &server() { return m_server; }
	uint port() { return m_port; }

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
	void slotSocketClosed();

protected slots:
	virtual void slotReadyWrite();

protected:
	/**
	 * The id of the message sent to the QQ server. This ID will increment
	 * for each subsequent message sent.
	 */
	uint m_id;

private:
	/**
	 * Queue of pending commands (should be mostly empty, but is needed to
	 * send more than one command to the server)
	 */
	QList<QByteArray> m_sendQueue;
	QList<QByteArray> m_buffer;

	KNetwork::KBufferedSocket *m_socket;
	OnlineStatus m_onlineStatus;

	QString m_server;
	uint m_port;

};

#endif

