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

bool KIRCTransferServer::initServer()
{
	if(!m_socket)
	{
		m_socket = new KExtendedSocket();

		QObject::connect(m_socket, SIGNAL(readyAccept()),
				this, SLOT(readyAccept()));
		m_socket->setPort(m_port);
		m_socket->setSocketFlags(KExtendedSocket::noResolve
					|KExtendedSocket::passiveSocket
					|KExtendedSocket::anySocket );
		m_socket->listen( m_backlog );
		m_socket->setBlockingMode(TRUE);
	}
	return (m_socket->socketStatus() != KExtendedSocket::error);
}

bool KIRCTransferServer::initServer( Q_UINT16 port, int backlog )
{
	if(m_socket)
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
//	initClient(); // Check for peer adress ?
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
