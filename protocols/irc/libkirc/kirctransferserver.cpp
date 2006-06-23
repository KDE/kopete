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

#include "kirctransferhandler.h"

#include "kirctransferserver.h"

using namespace KIRC;

/*
TransferServer::TransferServer( QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_socket( 0 ),
	  m_port( 0 ),
	  m_backlog( 1 )
{
}
*/
TransferServer::TransferServer(Q_UINT16 port, int backlog, QObject *parent, const char *name)
	: QObject( parent, name ),
	  m_socket( 0 ),
	  m_port( port ),
	  m_backlog( backlog )
{
}

TransferServer::TransferServer(Engine *engine, QString nick,// QString nick_peer_adress,
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize,
			QObject *parent, const char *name)
	: QObject( parent, name ),
	  m_socket(0),
	  m_port(0),
	  m_backlog(1),
	  m_engine(engine),
	  m_nick(nick),
	  m_type(type),
	  m_fileName(fileName),
	  m_fileSize(fileSize)
{
	initServer();
}

TransferServer::~TransferServer()
{
	if (m_socket)
		delete m_socket;
}

bool TransferServer::initServer()
{
	if (!m_socket)
	{
		QObject::connect(this, SIGNAL(incomingNewTransfer(Transfer *)),
				TransferHandler::self(), SIGNAL(transferCreated(Transfer *)));

		m_socket = new KExtendedSocket();

//		m_socket->setHost(m_socket->localAddress()->nodeName());
		if (!m_socket->setPort(m_port))
			kdDebug(14120) << k_funcinfo << "Failed to set port to" << m_port << endl;
		m_socket->setSocketFlags(KExtendedSocket::noResolve
					|KExtendedSocket::passiveSocket
					|KExtendedSocket::inetSocket );

		if (!m_socket->setTimeout(2*60)) // FIXME: allow configuration of this.
			kdDebug(14120) << k_funcinfo << "Failed to set timeout." << endl;

		QObject::connect(m_socket, SIGNAL(readyAccept()),
				this, SLOT(readyAccept()));
		QObject::connect(m_socket, SIGNAL(connectionFailed(int)),
				this, SLOT(connectionFailed(int)));

		m_socket->listen(m_backlog);
		m_socket->setBlockingMode(true);

		const KInetSocketAddress *localAddress = static_cast<const KInetSocketAddress *>(m_socket->localAddress());
		if (!localAddress)
		{
			kdDebug(14120) << k_funcinfo << "Not a KInetSocketAddress." << endl;
			deleteLater();
			return false;
		}

		m_port = localAddress->port();
	}
	return (m_socket->socketStatus() != KExtendedSocket::error);
}

bool TransferServer::initServer( Q_UINT16 port, int backlog )
{
	if (m_socket)
	{
		m_port = port;
		m_backlog = backlog;
	}
	return initServer();
}

void TransferServer::readyAccept()
{
	KExtendedSocket *socket;
	m_socket->accept( socket );
	Transfer *transfer = new Transfer(m_engine, m_nick, m_type, m_fileName, m_fileSize);
	transfer->setSocket(socket);
	transfer->initiate();
	emit incomingNewTransfer(transfer);
}

void TransferServer::connectionFailed(int error)
{
	if (error!=0)
	{
		kdDebug(14120) << k_funcinfo << "Connection failed with " << m_nick << endl;
		deleteLater();
	}
}
/*
void Transfer::initClient()
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
