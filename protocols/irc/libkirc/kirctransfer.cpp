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

using namespace KIRC;

Transfer::Transfer(	Engine *engine, QString nick,// QString nick_peer_adress
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

Transfer::Transfer(	Engine *engine, QString nick,// QString nick_peer_adress
			Transfer::Type type,
			QString fileName, Q_UINT32 fileSize, // put this in a QVariant ?
			QObject *parent, const char *name )
	: QObject( parent, name ),
	  m_engine(engine), m_nick(nick),
	  m_type(type), m_socket(0),
	  m_initiated(false),
	  m_file(0), m_fileName(fileName), m_fileSize(fileSize), m_fileSizeCur(0), m_fileSizeAck(0),
	  m_receivedBytes(0), m_receivedBytesLimit(0), m_sentBytes(0), m_sentBytesLimit(0)
{
}

Transfer::Transfer(	Engine *engine, QString nick,// QString nick_peer_adress
			QHostAddress hostAdress, Q_UINT16 port, // put this in a QVariant ?
			Transfer::Type type,
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
}
/*
Transfer::Transfer(	Engine *engine, QString nick,// QString nick_peer_adress
				Transfer::Type type, QVariant properties,
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
Transfer::~Transfer()
{
	closeSocket();
	// m_file is automatically closed on destroy.
}

Transfer::Status Transfer::status() const
{
	if(m_socket)
	{
//		return (Transfer::Status)m_socket->socketStatus();
		return Connected;
	}
	return Error_NoSocket;
}

void Transfer::slotError( int error )
{
	// Connection in progress.. This is a signal fired wrong
	if (m_socket->socketStatus () != KExtendedSocket::connecting)
	{
		abort(KExtendedSocket::strError(m_socket->status(), m_socket->systemError()));
//		closeSocket();
	}
}

bool Transfer::initiate()
{
	QTimer *timer = 0;

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

	connect(this, SIGNAL(complete()),
		this, SLOT(closeSocket()));
	connect(this, SIGNAL(abort(QString)),
		this, SLOT(closeSocket()));

//	connect(m_socket, SIGNAL(connectionClosed()),
//		this, SLOT(slotConnectionClosed()));
//	connect(m_socket, SIGNAL(delayedCloseFinished()),
//		this, SLOT(slotConnectionClosed()));
	connect(m_socket, SIGNAL(error(int)), // FIXME: connection failed: No such signal KExtendedSocket::error(int)
		this, SLOT(slotError(int)));

	switch( m_type )
	{
	case Chat:
		kdDebug(14121) << k_funcinfo << "Stting up a chat." << endl;
		connect(m_socket, SIGNAL(readyRead()),
			this, SLOT(readyReadFileIncoming()));
		break;
	case FileIncoming:
		kdDebug(14121) << k_funcinfo << "Stting up an incoming file transfer." << endl;
		m_file.open(IO_WriteOnly);
		connect(m_socket, SIGNAL(readyRead()),
			this, SLOT(readyReadFileIncoming()));
		break;
	case FileOutgoing:
		kdDebug(14121) << k_funcinfo << "Stting up an outgoing file transfer." << endl;
		m_file.open(IO_ReadOnly);
		connect(m_socket, SIGNAL(readyRead()),
			this, SLOT(readyReadFileOutgoing()));
//		timer = new QTimer(this);
//		connect(timer, SIGNAL(timeout()),
//			this, SLOT(writeFileOutgoing()));
//		timer->start(1000, false);
		writeFileOutgoing(); // send a first packet.
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
	// As far as I understand, buffer (socket buffer at least) should be flushed while event-looping.
	// But I'm not really sure of this, so I force the flush.
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()),
		this, SLOT(flush()));
	timer->start(1000, FALSE); // flush the streams at every seconds

	return true;
}

bool Transfer::setSocket( KExtendedSocket *socket )
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

void Transfer::closeSocket()
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
 void Transfer::flush()
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

void Transfer::userAbort(QString msg)
{
	emit abort(msg);
}

void Transfer::setCodec( QTextCodec *codec )
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

void Transfer::writeLine( const QString &line )
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

void Transfer::readyReadLine()
{
	if( m_socket->canReadLine() )
	{
		QString msg = m_socket_textStream.readLine();
		emit readLine(msg);
	}
}

void Transfer::readyReadFileIncoming()
{
	kdDebug(14121) << k_funcinfo << endl;

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
	else if(m_bufferLength == -1)
		abort("Error while reading socket.");
}

void Transfer::readyReadFileOutgoing()
{
	kdDebug(14121) << k_funcinfo << "Available bytes:" << m_socket->bytesAvailable() << endl;

	bool hadData = false;
	Q_UINT32 fileSizeAck = 0;

//	if (m_socket->bytesAvailable() >= sizeof(fileSizeAck)) // BUGGY: bytesAvailable() that allways return 0 on unbuffered sockets.
	{
		m_socketDataStream >> fileSizeAck;
		hadData = true;
	}

	if (hadData)
	{
		checkFileTransferEnd(fileSizeAck);
		writeFileOutgoing();
	}
}

void Transfer::writeFileOutgoing()
{
	kdDebug(14121) << k_funcinfo << endl;

	if (m_fileSizeAck < m_fileSize)
	{
		m_bufferLength = m_file.readBlock(m_buffer, sizeof(m_buffer));
		if (m_bufferLength > 0)
		{
			Q_UINT32 read = m_socket->writeBlock(m_buffer, m_bufferLength); // should check written == read

//			if(read != m_buffer_length)
//				buffer is not cleared still

			m_fileSizeCur += read;
//			m_socket->flush(); // Should think on using this
			emit fileSizeCurrent( m_fileSizeCur );
		}
		else if(m_bufferLength == -1)
			abort("Error while reading file.");
	}
}

void Transfer::checkFileTransferEnd(Q_UINT32 fileSizeAck)
{
	kdDebug(14121) << k_funcinfo << "Acknowledged:" << fileSizeAck << endl;

	m_fileSizeAck = fileSizeAck;
	emit fileSizeAcknowledge(m_fileSizeAck);

	if(m_fileSizeAck > m_fileSize)
		abort(i18n("Acknowledge size is greater than the expected file size"));

	if(m_fileSizeAck == m_fileSize)
		emit complete();
}

#include "kirctransfer.moc"
