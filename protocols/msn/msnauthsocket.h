/*
    msnauthsocket.h - Socket that does the initial handshake as used by
                       both MSNAuthSocket and MSNNotifySocket

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

#ifndef MSNAUTHSOCKET_H
#define MSNAUTHSOCKET_H

#include <msnsocket.h>

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 * MSNAuthSocket contains a connection to the MSN Servers that require
 * handshaking. Both the Dispatch Server and the Notification Server do this,
 * and most of the code is shared. MSNDispatchServer now provides only a
 * _very_ thin wrapper over this class, whereas MSNNotifyServer will truly
 * extend it.
 */
class MSNAuthSocket : public MSNSocket
{
	Q_OBJECT

public:
	MSNAuthSocket( const QString &msnId );
	~MSNAuthSocket();

	QString msnId() { return m_msnId; }

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

private slots:
	void reconnect();

private:
	QString m_msnId;
	bool m_msgBoxShown;
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

