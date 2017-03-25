/*
    kirctransfer.h - DCC Handler

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

#ifndef KIRCTRANSFERSERVER_H
#define KIRCTRANSFERSERVER_H

#include "kirctransfer.h"

#include <qobject.h>

class KExtendedSocket;

namespace KIrc {
class TransferServer : public QObject
{
    Q_OBJECT

public:
//	explicit TransferServer(QObject *parent = 0);
    explicit TransferServer(quint16 port, int backlog = 1, QObject *parent = 0);
    TransferServer(KIrc::Engine *engine, QString nick,// QString nick_peer_adress,
                   Transfer::Type type, QString fileName, quint32 fileSize, QObject *parent = 0);

    ~TransferServer();

    int port()
    {
        return m_port;
    }

protected:
    bool initServer();
    bool initServer(quint16 port, int backlog = 1);

signals:
    void incomingNewTransfer(Transfer *transfer);

protected slots:
    void readyAccept();
    void connectionFailed(int error);

private:
    KExtendedSocket *m_socket;
    quint16 m_port;
    int m_backlog;

    // The following will be deprecated ...
    KIrc::Engine *m_engine;
    QString m_nick;
    Transfer::Type m_type;
    QString m_fileName;
    quint32 m_fileSize;
    // by
    // QPtrList<Transfer> m_pendingTransfers;
    // QPtrList<Transfer> m_activeTransfers;
};
}

#endif
