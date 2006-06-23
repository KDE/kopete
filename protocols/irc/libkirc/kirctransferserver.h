/*
    kirctransfer.h - DCC Handler

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

#ifndef KIRCTRANSFERSERVER_H
#define KIRCTRANSFERSERVER_H

#include "kirctransfer.h"

#include <qobject.h>

class KExtendedSocket;

class QFile;
class QTextCodec;

namespace KIRC
{

class TransferServer
	: public QObject
{
	Q_OBJECT

public:
//	TransferServer(QObject *parent = 0, const char *name = 0);
	TransferServer(Q_UINT16 port, int backlog = 1, QObject *parent = 0, const char *name = 0);
	TransferServer(KIRC::Engine *engine, QString nick,// QString nick_peer_adress,
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize,
			QObject *parent = 0, const char *name = 0);

	~TransferServer();

	int port()
	{ return m_port; }

protected:
	bool initServer();
	bool initServer( Q_UINT16 port, int backlog = 1 );

signals:
	void incomingNewTransfer(Transfer *transfer);

protected slots:
	void readyAccept();
	void connectionFailed(int error);

private:
	KExtendedSocket *	m_socket;
	Q_UINT16		m_port;
	int			m_backlog;

	// The following will be deprecated ...
	KIRC::Engine *		m_engine;
	QString			m_nick;
	Transfer::Type	m_type;
	QString			m_fileName;
	Q_UINT32		m_fileSize;
	// by
	// QPtrList<Transfer> m_pendingTransfers;
	// QPtrList<Transfer> m_activeTransfers;

};

}

#endif
