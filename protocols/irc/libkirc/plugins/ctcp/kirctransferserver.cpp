/*
    kirctransfer.cpp - IRC transfer.

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

#include "kirctransferserver.moc"

#include "kirctransferhandler.h"

#include <kdebug.h>
#include <kextsock.h>

using namespace KIrc;

/*
TransferServer::TransferServer( QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_socket( 0 ),
	  m_port( 0 ),
	  m_backlog( 1 )
{
}
*/
TransferServer::TransferServer(quint16 port, int backlog, QObject *parent)
	: QObject( parent ),
	  m_socket( 0 ),
	  m_port( port ),
	  m_backlog( backlog )
{
}

TransferServer::TransferServer(Engine *engine, QString nick,// QString nick_peer_adress,
			Transfer::Type type,
			QString fileName, quint32 fileSize,
			QObject *parent)
	: QObject( parent ),
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
			kDebug(14120) << "Failed to set port to" << m_port;
		m_socket->setSocketFlags(KExtendedSocket::noResolve
					|KExtendedSocket::passiveSocket
					|KExtendedSocket::inetSocket );

		if (!m_socket->setTimeout(2*60)) // FIXME: allow configuration of this.
			kDebug(14120) << "Failed to set timeout.";

		QObject::connect(m_socket, SIGNAL(readyAccept()),
				this, SLOT(readyAccept()));
		QObject::connect(m_socket, SIGNAL(connectionFailed(int)),
				this, SLOT(connectionFailed(int)));

		m_socket->listen(m_backlog);
		m_socket->setBlockingMode(true);

		const KInetSocketAddress *localAddress = static_cast<const KInetSocketAddress *>(m_socket->localAddress());
		if (!localAddress)
		{
			kDebug(14120) << "Not a KInetSocketAddress.";
			deleteLater();
			return false;
		}

		m_port = localAddress->port();
	}
	return (m_socket->socketStatus() != KExtendedSocket::error);
}

bool TransferServer::initServer( quint16 port, int backlog )
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
		kDebug(14120) << "Connection failed with " << m_nick;
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
