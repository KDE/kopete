/*
    kirctransferhandler.h - DCC Handler

    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCTRANSFERHANDLER_H
#define KIRCTRANSFERHANDLER_H

#include <qhostaddress.h>

#include "kirctransfer.h"
#include "kirctransferserver.h"

class QFile;
class QTextCodec;

class KExtendedSocket;

namespace KIRC
{

class TransferHandler
	: public QObject
{
	Q_OBJECT

public:
	static TransferHandler *self();

	TransferServer *server();
	TransferServer *server( Q_UINT16 port, int backlog  = 1 );

	TransferServer *createServer(KIRC::Engine *engine, QString m_userName,
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize);

	Transfer *createClient(
		KIRC::Engine *engine, QString nick,// QString nick_peer_adress,
		QHostAddress peer_address, Q_UINT16 peer_port,
		Transfer::Type type,
		QString file = QString::null, Q_UINT32 fileSize = 0 );

//	void registerServer( DCCServer * );
//	QPtrList<DCCServer> getRegisteredServers();
//	static QPtrList<DCCServer> getAllRegisteredServers();
//	void unregisterServer( DCCServer * );

//	void registerClient( DCCClient * );
//	QPtrList<DCCClient> getRegisteredClients();
//	static QPtrList<DCCClient> getAllRegisteredClients();
//	void unregisterClient( DCCClient * );

signals:
	void transferServerCreated(KIRC::TransferServer *server);
	void transferCreated(KIRC::Transfer *transfer);

private:
//	TransferHandler();

	TransferServer *m_server;
//	QPtrList<TransferServer> m_servers;
//	QPtrList<Transfer> m_clients;
};

}

#endif
