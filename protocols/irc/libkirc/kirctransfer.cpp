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
#include <klocale.h>

#include <qfile.h>
#include <qtimer.h>

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
				KIRCTransfer::Type type,
				QString fileName, Q_UINT32 fileSize, // put this in a QVariant ?
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(0),
	  m_initiated(false),
	  m_file(0), m_fileName(fileName), m_fileSize(fileSize), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
	connect(this, SIGNAL(complete()),
		this, SLOT(closeSocket()));

	connect(this, SIGNAL(abort(QString)),
		this, SLOT(closeSocket()));
}

KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				QHostAddress hostAdress, Q_UINT16 port, // put this in a QVariant ?
				KIRCTransfer::Type type,
				QString fileName, Q_UINT32 fileSize, // put this in a QVariant ?
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(0),
	  m_initiated(false),
	  m_file(0), m_fileName(fileName), m_fileSize(fileSize), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
	setSocket(new KExtendedSocket(hostAdress.toString(), port));

	connect(this, SIGNAL(complete()),
		this, SLOT(closeSocket()));

	connect(this, SIGNAL(abort(QString)),
		this, SLOT(closeSocket()));
}
/*
KIRCTransfer::KIRCTransfer(	KIRC *engine, QString nick,// QString nick_peer_adress
				KIRCTransfer::Type type, QVariant properties,
				QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(properties[socket]),
	  m_initiated(false),
	  m_file(0), m_fileName(properties[fileName]), m_fileSize(properties[fileSize]), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
	if(!properites["socket"].isNull())
		setSocket(properites["socket"]);
	else if(!properites["hostAddress"].isNull() && !properites["hostPort"].isNull())
		setSocket(new KExtendedSocket(properites["hostAddress"], properites["hostPort"]));

	connect(this, SIGNAL(complete()),
		this, SLOT(closeSocket()));

	connect(this, SIGNAL(abort(QString)),
		this, SLOT(closeSocket()));
}
*/
KIRCTransfer::~KIRCTransfer()
{
	closeSocket();
	// m_file is automatically closed on destroy.
}

KIRCTransfer::Status KIRCTransfer::status() const
{
	if(m_socket)
	{
//		return (KIRCTransfer::Status)m_socket->socketStatus();
		return Connected;
	}
	return Error_NoSocket;
}

void KIRCTransfer::slotError( int error )
{
	// Connection in progress.. This is a signal fired wrong
	if (m_socket->socketStatus () != KExtendedSocket::connecting)
	{
		abort(KExtendedSocket::strError(m_socket->status(), m_socket->systemError()));
//		closeSocket();
	}
}

bool KIRCTransfer::initiate()
{
	if(m_initiated)
	{
		kdDebug(14121) << k_funcinfo << "Transfer allready initiated" << endl;
		return false;
	}

	if(!m_socket)
	{
		kdDebug(14121) << k_funcinfo << "Socket not set" << endl;
		return false;
	}

	m_initiated = true;

	m_file.setName(m_fileName);

//	connect(m_socket, SIGNAL(connectionClosed()),
//		this, SLOT(slotConnectionClosed()));
//	connect(m_socket, SIGNAL(delayedCloseFinished()),
//		this, SLOT(slotConnectionClosed()));
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
//		QSignal for sending data
		break;
	default:
		kdDebug(14121) << k_funcinfo << "Closing transfer: Unknown extra initiation for type:" << m_type << endl;
		m_socket->close();
		return false;
		break;
	}

//	if(status()==Idle)
	if(m_socket->status()==KExtendedSocket::nothing)
		m_socket->connect();

	m_socket->enableRead(true);
	m_socket->enableWrite(true);

	m_socketDataStream.setDevice(m_socket);

	// I wonder if calling this is really necessary
	// As far as I understand, buffer (socket butffer at least) should be flushed while event-looping.
	// But I'm not really sure of this, so I force the flush.
	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(flush()) );
	timer->start( 1000, FALSE ); // flush the streams at every seconds

	return true;
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

void KIRCTransfer::closeSocket()
{
	if(m_socket)
	{
		m_socket->close();
//		m_socket->reset();
		m_socket->deleteLater();
	}
	m_socket = 0;
}

/*
 * This slot ensure that all the stream are flushed.
 * This slot is called periodically internaly.
 */
 void KIRCTransfer::flush()
{
	/*
	 * Enure the incoming file content in case of a crash.
	 */
	if(m_file.isOpen() && m_file.isWritable())
		m_file.flush();

	/*
	 * Ensure that non interactive streams outputs (i.e file transfer acknowledge by example)
	 * are sent (Don't stay in a local buffer).
	 */
	if(m_socket && status() == Connected)
		m_socket->flush();
}

void KIRCTransfer::userAbort(QString msg)
{
	emit abort(msg);
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
//		m_socket.flush();
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
	if(m_bufferLength > 0)
	{
		int written = m_file.writeBlock(m_buffer, m_bufferLength);
		if(m_bufferLength == written)
		{
			m_fileSizeCur += written;
			m_fileSizeAck = m_fileSizeCur;
			m_socketDataStream << m_fileSizeAck;
			checkFileTransferEnd(m_fileSizeAck);
			return;
		}
		else
			// Something bad happened while writting.
			abort(m_file.errorString());
	}

	if(m_bufferLength == -1)
	{
		abort("Error while reading socket");
	}
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
//			m_socket->flush(); // Should think on using this
//			emit fileSizeCurrent( m_fileSizeCur );
//		}
//	}
//	else if (m_file_size_ack > m_file_size)
//		abort( "" );
}

void KIRCTransfer::readyReadFileOutgoing()
{
//	if(m_socket->canread(sizeof(m_file_size_ack)))
	{
		m_socketDataStream >> m_fileSizeAck;
		checkFileTransferEnd(m_fileSizeAck);
	}
}

void KIRCTransfer::checkFileTransferEnd( Q_UINT32 fileSizeAck )
{
	m_fileSizeAck = fileSizeAck;
	emit fileSizeAcknowledge(m_fileSizeAck);

	if(m_fileSizeAck > m_fileSize)
		abort(i18n("Acknowledge size greater then expected file size"));

	if(m_fileSizeAck == m_fileSize)
		emit complete();
}

#include "kirctransfer.moc"

// vim: set noet ts=4 sts=4 sw=4:
