/*
    kirctransferhandler.h - DCC Handler

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

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

namespace KIrc {
class TransferHandler : public QObject
{
    Q_OBJECT

public:
    static TransferHandler *self();

    TransferServer *server();
    TransferServer *server(quint16 port, int backlog = 1);

    TransferServer *createServer(KIrc::Engine *engine, QString m_userName, Transfer::Type type, QString fileName, quint32 fileSize);

    Transfer *createClient(
        KIrc::Engine *engine, QString nick,// QString nick_peer_adress,
        QHostAddress peer_address, quint16 peer_port, Transfer::Type type, QString file = QString(), quint32 fileSize = 0);

//	void registerServer( DCCServer * );
//	QPtrList<DCCServer> getRegisteredServers();
//	static QPtrList<DCCServer> getAllRegisteredServers();
//	void unregisterServer( DCCServer * );

//	void registerClient( DCCClient * );
//	QPtrList<DCCClient> getRegisteredClients();
//	static QPtrList<DCCClient> getAllRegisteredClients();
//	void unregisterClient( DCCClient * );

signals:
    void transferServerCreated(KIrc::TransferServer *server);
    void transferCreated(KIrc::Transfer *transfer);

private:
//	TransferHandler();

    TransferServer *m_server;
//	QPtrList<TransferServer> m_servers;
//	QPtrList<Transfer> m_clients;
};
}

#endif
