/*
    kirctransferhandler.h - DCC Handler

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

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

class KIRCTransferHandler
	: public QObject
{
	Q_OBJECT

public:
	static KIRCTransferHandler *self()
		{ return &sm_self; }

	KIRCTransferServer *server();
	KIRCTransferServer *server( Q_UINT16 port, int backlog  = 1 );

	KIRCTransferServer *createServer(KIRC *engine, QString m_userName,
			KIRCTransfer::Type type,
			QString fileName, Q_UINT32 fileSize);

	KIRCTransfer *createClient(
		KIRC *engine, QString nick,// QString nick_peer_adress,
		QHostAddress peer_address, Q_UINT16 peer_port,
		KIRCTransfer::Type type,
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
	void transferServerCreated( KIRCTransferServer *server );
	void transferCreated( KIRCTransfer *transfer );

private:
//	KIRCTransferHandler();
	static KIRCTransferHandler sm_self;

	KIRCTransferServer *m_server;
//	QPtrList<KIRCTransferServer> m_servers;
//	QPtrList<KIRCTransfer> m_clients;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
