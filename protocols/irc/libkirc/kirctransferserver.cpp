/*
    kirctransfer.cpp - IRC transfer.

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

#include <kdebug.h>
#include <kextsock.h>

#include "kirctransferserver.h"

KIRCTransferServer::KIRCTransferServer( QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_socket( 0 ),
	  m_port( 0 ),
	  m_backlog( 1 )
{
}

KIRCTransferServer::KIRCTransferServer( Q_UINT16 port, int backlog, QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_socket( 0 ),
	  m_port( port ),
	  m_backlog( backlog )
{
}

KIRCTransferServer::KIRCTransferServer(QString userName,
			KIRCTransfer::Type type,
			QString fileName, Q_UINT32 fileSize,
			QObject *parent, const char *name)
	: QObject( parent, name ),
	  m_socket(0),
	  m_port(0),
	  m_backlog(1),
	  m_userName(userName),
	  m_type(type),
	  m_fileName(fileName),
	  m_fileSize(fileSize)
{
	initServer();
}

bool KIRCTransferServer::initServer()
{
	if (!m_socket)
	{
		m_socket = new KExtendedSocket();

		QObject::connect(m_socket, SIGNAL(readyAccept()),
				this, SLOT(readyAccept()));
		QObject::connect(m_socket, SIGNAL(connectionFailed(int)),
				this, SLOT(connectionFailed(int)));

		if (!m_socket->setTimeout(2*60)) // FIXME: allow configuration of this.
			kdDebug(14120) << k_funcinfo << "Failed to set timeout." << endl;
//		if (!m_socket->setPort(m_port))
//		if (!m_socket->setPort(m_port))
		if (!m_socket->setPort(666))
			kdDebug(14120) << k_funcinfo << "Failed to set port." << endl;
		m_socket->setSocketFlags(KExtendedSocket::noResolve
					|KExtendedSocket::passiveSocket
					|KExtendedSocket::anySocket );
//		m_socket->listen(m_backlog);
		m_socket->listen(1);
		m_socket->setBlockingMode(true);

//		bool success;
//		m_port = m_socket->bindPort().toInt(&success);
//		if(!success)
//			kdDebug(14120) << k_funcinfo << "Failed to set port number:" << m_socket->bindPort() << endl;
		m_port = 666;
	}
	return (m_socket->socketStatus() != KExtendedSocket::error);
}

bool KIRCTransferServer::initServer( Q_UINT16 port, int backlog )
{
	if (m_socket)
	{
		m_port = port;
		m_backlog = backlog;
	}
	return initServer();
}

void KIRCTransferServer::readyAccept()
{
	KExtendedSocket *socket;
	m_socket->accept( socket );
	KIRCTransfer *transfer = new KIRCTransfer((KIRC *)0, m_userName, m_type, m_fileName, m_fileSize); // FIXME: remove the NULL if possible.
	transfer->setSocket(socket);
	transfer->initiate();
	emit incomingNewTransfer(transfer);
}

void KIRCTransferServer::connectionFailed(int error)
{
	if (error!=0)
	{
		kdDebug(14120) << k_funcinfo << "Connection failed with " << m_userName << endl;
		deleteLater();
	}
}
/*
void KIRCTransfer::initClient()
{
	if(!m_socket)
	{
		connect(m_socket, SIGNAL(connectionClosed()),
			this, SLOT(slotConnectionClosed()));
		connect(m_socket, SIGNAL(delayedCloseFinished()),
			this, SLOT(slotConnectionClosed()));
		connect(m_socket, SIGNAL(error(int)),
			this, SLOT(slotError(int)));
		connect(m_socket, SIGNAL(readyRead()),
			this, SLOT(readyReadFileOut));

		m_socket->enableRead( true );
		m_socket->enableWrite( true );
	}
}
*/
#include "kirctransferserver.moc"

// vim: set noet ts=4 sts=4 sw=4:
