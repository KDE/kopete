/*
    msndispatchsocket.h - Socket for the MSN Dispatch Server

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart       <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    Portions of this code are taken from KMerlin,
              (c) 2001      by Olaf Lueg             <olueg@olsd.de>

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

#include "msnauthsocket.h"

class MSNAccount;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * MSNDispatchSocket contains the connection to the MSN Dispatch Server.
 * The initial communication takes place using this server, after which
 * you are redirected to a notification server.
 */
class MSNDispatchSocket : public MSNAuthSocket
{
	Q_OBJECT

public:
	MSNDispatchSocket( MSNAccount *account, const QString &msnId, QObject *parent = 0L );
	~MSNDispatchSocket();

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
	 * Handle an MSN command response line.
	 */
	virtual void parseCommand( const QString &cmd, uint id, const QString &data );

private:
	MSNAccount *m_account;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

