/*
    msndispatchsocket.h - Socket for the MSN Dispatch Server

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

#ifndef MSNDISPATCHSOCKET_H
#define MSNDISPATCHSOCKET_H

#include <msnsocket.h>

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * MSNDispatchSocket contains the connection to the MSN Dispatch Server.
 * The initial communication takes place using this server, after which
 * you are redirected to a notification server.
 */
class MSNDispatchSocket : public MSNSocket
{
	Q_OBJECT

public:
	MSNDispatchSocket( const QString &msnId );
	~MSNDispatchSocket();

	/**
	 * Don't shadow the full method with our new slot below, because the
	 * derived MSNNotifySocket still needs it.
	 * In an ideal world a simple 'using MSNSocket::connect( ... )' would
	 * be enough, but not all compilers support that :(
	 */
	void connect( const QString &server, uint port );

public slots:
	/**
	 * The dispatch server always connects to the same host, this method is
	 * for convenience.
	 */
	void connect();

signals:
	/**
	 * When the dispatch server sends us the notification server to use, this
	 * signal is emitted. After this the socket is automatically closed.
	 */
	void receivedNotificationServer( const QString &host, uint port );

protected:
	/**
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
	virtual void doneConnect();

	/**
	 * Handle an MSN error condition.
	 * This reimplementation handles the 'server busy' error by attempting a
	 * reconnect in about 10 seconds, but calls the parent's implementation
	 * for all other errors.
	 */
	virtual void handleError( uint code, uint id );

	/**
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id,
		const QString &data );

private:
	QString m_msnId;
	bool m_msgBoxShown;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

