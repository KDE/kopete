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

#include <qfile.h>

#include "kirctransfer.h"

KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				Type type,
				QObject *parent, const char *name )
	: QObject( parent, name ),
//	  m_engine(engine), m_nick(nick),
	  m_type(type),
	  m_socket(0),
	  m_file(0),
	  m_file_size_cur(0), m_file_size_ack(0),
	  m_received_bytes_limit(0), m_received_bytes(0),
	  m_sent_bytes_limit(0), m_sent_bytes(0)
{
}

KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				QHostAddress , Q_UINT16,
				KIRCTransfer::Type type,
				QFile *file, Q_UINT32 file_size,
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_type(type),
	  m_received_bytes_limit(0), m_received_bytes(0),
	  m_sent_bytes_limit(0), m_sent_bytes(0)
{
}

KIRCTransfer::~KIRCTransfer()
{
	switch( m_type )
	{
	case Chat:
		break;
	case FileOutgoing:
	case FileIncoming:
		if( m_file )
			m_file->close();
		break;
	default:
		break;
	}
}
/*
KIRCTransfer::Status KIRCTransfer::status()
{
	if(m_socket)
	{
		return (KIRCTransfer::Status)m_socket->socketStatus();
	}
	return KExtendedSocket::error;
}
*/
void KIRCTransfer::connectToPeer()
{
	if(m_socket)
		m_socket->connect();
}

bool KIRCTransfer::setSocket( KExtendedSocket *socket )
{
	if (!m_socket)
	{
		m_socket = socket;

		m_socket->enableRead( true );
		m_socket->enableWrite( true );

		connect(m_socket, SIGNAL(connectionClosed()),
			this, SLOT(slotConnectionClosed()));
		connect(m_socket, SIGNAL(delayedCloseFinished()),
			this, SLOT(slotConnectionClosed()));
		connect(m_socket, SIGNAL(error(int)),
			this, SLOT(slotError(int)));

		switch( m_type )
		{
		case Chat:
			connect(m_socket, SIGNAL(readyRead()),
				this, SLOT(readyReadFileIncoming()));
			return true;
		case FileIncoming:
			if( !m_file )
				break;
			connect(m_socket, SIGNAL(readyRead()),
				this, SLOT(readyReadFileIncoming()));
			return true;
		case FileOutgoing:
			if( !m_file )
				break;
			connect(m_socket, SIGNAL(readyRead()),
				this, SLOT(readyReadFileOutgoing()));
//			QSignal for sending data
			return true;
		default:
//			Unknown extra initializer for type.
			break;
		}

//		moved upper as all signal should be called by the event loop.
//		m_socket->enableRead( true );
//		m_socket->enableWrite( true );
//		return true;
	}
//	setAutoDelete( true );
	return false;
}

void KIRCTransfer::setCodec( QTextCodec *codec )
{
	switch( m_type )
	{
	case Chat:
		m_socket_textStream.setCodec( codec );
		break;
	default:
//		operation not permitted on this type.
		break;
	}
}

void KIRCTransfer::writeLine( const QString &line )
{
	switch( m_type )
	{
	case Chat:
		break;
	default:
//		operation not permitted on this type.
		break;
	}
}

void KIRCTransfer::readyReadLine()
{
	if( m_socket->canReadLine() )
	{
		QString msg = m_socket_textStream.readLine();
		emit readLine(msg);
	}
}

void KIRCTransfer::readyReadFileIncoming()
{
	m_buffer_length = m_socket->readBlock(m_buffer, sizeof(m_buffer));
	if (m_buffer_length != -1)
	{
		m_file_size_cur += m_file->writeBlock(m_buffer, m_buffer_length);
		if(m_file_size_cur > m_file_size_ack)
		{
			m_file_size_ack = m_file_size_ack;
			m_socket_dataStream << m_file_size_ack;
		}
	}
	emitSignals();
}

void KIRCTransfer::writeFileOutgoing()
{
	// should check m_filesize_ack == m_file_size_cur
//	if (m_file_size_ack < m_file_size)
//	{
//		m_buffer_length = m_file->readBlock(m_buffer, sizeof(m_buffer));
//		if (m_buffer_length != -1)
//		{
//			Q_LONG m_socket->writeBlock(m_buffer, m_buffer_length);
//		}
//	}
//	else if (m_file_size_ack > m_file_size)
//		abort the client something strage happend
}

void KIRCTransfer::readyReadFileOutgoing()
{
//	if( m_socket->canread( sizeof(m_file_size_ack) ) )
	{
		m_socket_dataStream >> m_file_size_ack;
		emitSignals();
	}
}

void KIRCTransfer::emitSignals()
{
	if(m_received_bytes_limit)
		emit received( m_received_bytes * 100 / m_received_bytes_limit );
	emit receivedBytes( m_received_bytes );

	if(m_sent_bytes_limit)
		emit sent( m_sent_bytes * 100 / m_sent_bytes_limit );
	emit sentBytes( m_sent_bytes );
}

#include "kirctransfer.moc"

// vim: set noet ts=4 sts=4 sw=4:
