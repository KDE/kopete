/*
    kirctransfer.cpp - IRC transfer.

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

#include <kdebug.h>
#include <kextsock.h>

#include <qfile.h>

#include "kirctransfer.h"

KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				Type type,
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(0),
	  m_initiated(false),
	  m_file(0), m_fileName(QString::null), m_fileSize(0), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
}

KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				QHostAddress , Q_UINT16,
				KIRCTransfer::Type type,
				QString file, Q_UINT32 file_size,
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(0),
	  m_initiated(false),
	  m_file(0), m_fileName(QString::null), m_fileSize(0), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
}

KIRCTransfer::~KIRCTransfer()
{
	if(m_socket)
	{
		m_socket->close();
		m_socket->deleteLater();
	}
	// m_file is automatically closed on destroy.
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

bool KIRCTransfer::initiate()
{
	if(m_initiated)
	{
		kdDebug(14121) << k_funcinfo << "Transfer allready initiated" << endl;
		return false;
	}

	if(m_socket)
	{
		m_initiated = true;

		m_file.setName(m_fileName);

		if(m_socket->status())
			m_socket->connect();

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
			break;
		case FileIncoming:
			m_file.open(IO_WriteOnly);
			connect(m_socket, SIGNAL(readyRead()),
				this, SLOT(readyReadFileIncoming()));
			break;
		case FileOutgoing:
			m_file.open(IO_ReadOnly);
			connect(m_socket, SIGNAL(readyRead()),
				this, SLOT(readyReadFileOutgoing()));
//			QSignal for sending data
			break;
		default:
			kdDebug(14121) << k_funcinfo << "Closing transfer: Unknown extra initiation for type:" << m_type << endl;
			m_socket->close();
			return false;
			break;
		}

		m_socket->enableRead(true);
		m_socket->enableWrite(true);
		return true;
	}
	else
		kdDebug(14121) << k_funcinfo << "Socket not set" << endl;
	return false;
}

bool KIRCTransfer::setSocket( KExtendedSocket *socket )
{
	if (!m_socket)
	{
		m_socket = socket;
		return true;
	}
	else
		kdDebug(14121) << k_funcinfo << "Socket allready set" << endl;
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
	m_bufferLength = m_socket->readBlock(m_buffer, sizeof(m_buffer));
	if (m_bufferLength != -1)
	{
		m_fileSizeCur += m_file.writeBlock(m_buffer, m_bufferLength);
		if(m_fileSizeCur > m_fileSizeAck)
		{
			m_fileSizeAck = m_fileSizeAck;
			m_socketDataStream << m_fileSizeAck;
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
		m_socketDataStream >> m_fileSizeAck;
		emitSignals();
	}
}

void KIRCTransfer::emitSignals()
{
	if(m_receivedBytesLimit)
		emit received( m_receivedBytes * 100 / m_receivedBytesLimit );
	emit receivedBytes( m_receivedBytes );

	if(m_sentBytesLimit)
		emit sent( m_sentBytes * 100 / m_sentBytesLimit );
	emit sentBytes( m_sentBytes );
}

#include "kirctransfer.moc"

// vim: set noet ts=4 sts=4 sw=4:
