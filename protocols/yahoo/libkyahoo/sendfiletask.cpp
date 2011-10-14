/*
    Kopete Yahoo Protocol
    Send a file

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendfiletask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "libyahoo.h"
#include "client.h"
#include <qstring.h>
#include <qtimer.h>
#include <QTime>
#include <kdebug.h>
#include <klocale.h>
#include <k3streamsocket.h>
#include <kio/global.h>
#include <krandom.h>

using namespace KNetwork;
using namespace KYahoo;


/* Buffer size to send file data. Automatically resized. Won't grow up past BUFFER_SIZE_MAX. Code will try
   to send as much data as it can in one shot */
static const int BUFFER_SIZE_INITIAL = 1024;
static const int BUFFER_SIZE_MAX = (64 * 1024);


SendFileTask::SendFileTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_transmitted = 0;
	m_socket = 0;

	QTime epoch(0, 0, 0);
	qsrand(epoch.secsTo(QTime::currentTime()));
}

SendFileTask::~SendFileTask()
{
	m_socket->deleteLater();
	m_socket = 0;
}

bool SendFileTask::forMe( const Transfer *transfer ) const
{
	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	if(!t)
		return false;

	if((t->service() == Yahoo::ServiceFileTransfer7 ||
	    t->service() == Yahoo::ServiceFileTransfer7Accept) &&
	   t->firstParam(265) == m_yahooTransferId)
	{
		return true;
	}

	return false;
}

bool SendFileTask::take(Transfer* transfer)
{
	if( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	kDebug(YAHOO_RAW_DEBUG) << t->service();

	if(t->service() == Yahoo::ServiceFileTransfer7)
		parseFileTransfer(t);
	else if(t->service() == Yahoo::ServiceFileTransfer7Accept)
		parseTransferAccept(t);

	return true;
}

void SendFileTask::parseFileTransfer( const Transfer *transfer )
{
	kDebug(YAHOO_RAW_DEBUG);

	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	if(!t)
		return;

	if(t->firstParam(222).toInt() == 4)
	{
		emit declined();
	}
	else if(t->firstParam(222).toInt() == 3)
	{
		sendFileTransferInfo();
	}
	else
	{
		setError();
		emit error(m_transferId, 0, i18n("Unknown error"));
	}
}

void SendFileTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	m_file.setFileName( m_url.toLocalFile() );

	m_yahooTransferId = newYahooTransferId();

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7);
	t->setId( client()->sessionID() );

	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 5, m_target.toLocal8Bit() );
	t->setParam( 265,  m_yahooTransferId.toLocal8Bit() );
	t->setParam( 222, 1 );
	t->setParam( 266, 1 );
	t->setParam( 302, 268 );
	t->setParam( 300, 268 );
	t->setParam( 27, m_url.fileName().toLocal8Bit() );
	t->setParam( 28, m_file.size());
	t->setParam( 301, 268 );
	t->setParam( 303, 268 );

	send( t );
}

void SendFileTask::sendFileTransferInfo()
{
	kDebug(YAHOO_RAW_DEBUG);

	KResolverResults results = KResolver::resolve("relay.msg.yahoo.com", QString::number(80));
	if(results.count() > 0)
	{
		m_relayHost = results.first().address().toString();
		m_relayHost.chop(3); // Remove the :80 from the end
	}
	else
	{
		emit error(m_transferId, 0, i18n("Unable to connect to file transfer server"));
		setError();
		return;
	}

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Info);
	t->setId( client()->sessionID() );

	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 5, m_target.toLocal8Bit() );
	t->setParam( 265, m_yahooTransferId.toLocal8Bit() );
	t->setParam( 27, m_url.fileName().toLocal8Bit() );
	t->setParam( 249, 3 );
	t->setParam( 250, m_relayHost.toLocal8Bit() );

	send( t );
}

void SendFileTask::parseTransferAccept(const Transfer *transfer)
{	
	kDebug(YAHOO_RAW_DEBUG);

	const YMSGTransfer *t = static_cast<const YMSGTransfer*>(transfer);

	// Disconnected
	if(t->status() == Yahoo::StatusDisconnected)
	{
		setError();
		return;
	}

	m_token = t->firstParam(251);
	kDebug(YAHOO_RAW_DEBUG) << "Token: " << m_token;

	m_socket = new KStreamSocket( m_relayHost, QString::number(80) );
	m_socket->setBlocking( false );
	m_socket->enableWrite(true);
	connect( m_socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(connectSucceeded()) );
	connect( m_socket, SIGNAL(gotError(int)), this, SLOT(connectFailed(int)) );
	connect( m_socket, SIGNAL(readyWrite()), this, SLOT(transmitHeader()) );

	m_socket->connect();

}

void SendFileTask::connectFailed( int i )
{
	QString err = KSocketBase::errorString(m_socket->error());
	kDebug(YAHOO_RAW_DEBUG) << i << ": " << err;
	emit error( m_transferId, i, err );
	setError();
}

void SendFileTask::connectSucceeded()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	if ( m_file.open(QIODevice::ReadOnly ) )
	{
		kDebug(YAHOO_RAW_DEBUG) << "File successfully opened. Reading...";
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error opening file: " << m_file.errorString();
		client()->notifyError( i18n( "An error occurred while sending the file." ), m_file.errorString(), Client::Error );
		m_socket->close();
		emit error( m_transferId, m_file.error(), m_file.errorString() );
		setError();
		return;
	}

	kDebug(YAHOO_RAW_DEBUG) << "Sizes: File (" << m_url << "): " << m_file.size();
	QString header = QString::fromLatin1("POST /relay?token=") + 
		QUrl::toPercentEncoding(m_token) +
		QString::fromLatin1("&sender=%1&recver=%2 HTTP/1.1\r\n"
					     "Cache-Control: no-cache\r\n"
					     "Cookie: T=%3; Y=%4\r\n"
					     "Host: %5\r\n"
					     "Content-Length: %6\r\n"
					     "User-Agent: Mozilla/5.0\r\n"
					     "Connection: Close\r\n\r\n")
		.arg(client()->userId()).arg(m_target)
		.arg(client()->tCookie()).arg(client()->yCookie())
		.arg(m_relayHost)
		.arg(QString::number(m_file.size()));
	m_buffer = header.toLocal8Bit();
	m_bufferOutPos = 0;
	m_bufferInPos = m_buffer.size();
}


void SendFileTask::transmitHeader()
{
	Q_ASSERT(m_bufferOutPos <= m_buffer.size());
	const int remaining = m_bufferInPos - m_bufferOutPos;

	if (remaining <= 0)
	{
		// Go to next step.
		disconnect( m_socket, SIGNAL(readyWrite()), this, SLOT(transmitHeader()) );
		connect( m_socket, SIGNAL(readyWrite()), this, SLOT(transmitData()) );
		m_buffer.clear();
		m_bufferOutPos = 0;
		m_bufferInPos = 0;
		m_buffer.resize(BUFFER_SIZE_INITIAL);
		transmitData(); // since we're just in the situation of being ready to transmit.
		return;
	}


	kDebug(YAHOO_RAW_DEBUG) << "Trying to send header part: " << m_buffer.mid(m_bufferOutPos);

	const qint64 written = m_socket->write(m_buffer.constData() + m_bufferOutPos, remaining);
	kDebug(YAHOO_RAW_DEBUG) << "  sent " << written << " bytes";

	if (written <= 0)
	{
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		m_socket->close();
		setError();
		return;
	}

	m_bufferOutPos += written;
}



bool SendFileTask::checkTransferEnd()
{
	if (m_transmitted >= m_file.size())
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Successful: " << m_transmitted;
		emit complete( m_transferId );
		setSuccess();
		m_socket->close();
		return true;
	}
	return false;
}



bool SendFileTask::fillSendBuffer()
{
	if (checkTransferEnd())
		return true;

	Q_ASSERT(m_buffer.size() >= m_bufferOutPos);
	Q_ASSERT(m_buffer.size() >= m_bufferInPos);

	if (m_bufferOutPos >= m_bufferInPos)
	{
		m_bufferOutPos = m_bufferInPos = 0;
	} else 
	{
		// we'd like to go with the maximum speed on the netw interface. reading from local file is speedy enough (+ caching at disk block layers)
		m_bufferInPos = m_buffer.size() - m_bufferOutPos;
		memmove(m_buffer.data(), m_buffer.constData() + m_bufferOutPos, m_bufferInPos);
		m_bufferOutPos = 0;
	}


	const int to_read = m_buffer.size() - m_bufferInPos;
	if (to_read <= 0)
	{
		return false; // buffer full. probably shouldn't happen
	}
	const qint64 file_read = m_file.read(m_buffer.data() + m_bufferInPos, to_read);

	// kDebug(YAHOO_RAW_DEBUG) << "reading from file: want" << to_read << "got to read" << file_read << " bytes";

	if (file_read < 0)
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Failed (reading file)!";

		m_buffer.clear();
		m_buffer.reserve(0); // free memory until this task is reused or freed.

		emit error( m_transferId, m_file.error(), m_file.errorString() );
		setError();
		return true;
	}

	m_bufferInPos += file_read;

	return false;
}


void SendFileTask::transmitData()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	if (fillSendBuffer())
		return;

	Q_ASSERT(m_bufferOutPos <= m_bufferInPos);

	const int remaining = m_bufferInPos - m_bufferOutPos;

	const qint64 written = m_socket->write(m_buffer.constData() + m_bufferOutPos, remaining);
	if (written <= 0)
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Failed (sending data)! toSend=" << remaining << "sent=" << written;
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		setError();
		return;
	}
	// kDebug(YAHOO_RAW_DEBUG) << "sending file content: toSend=" << remaining << "sent=" << written;

	m_transmitted += written;
	m_bufferOutPos += written;
        emit bytesProcessed( m_transferId, m_transmitted );
	if (checkTransferEnd())
		return;
	// see if we should do a bit of buffer resizing
	if ((m_buffer.size() < BUFFER_SIZE_MAX) &&
		(written >= remaining) && (written >= m_buffer.size()))
	{
		// double its size
		int oldC = m_buffer.size();
		m_buffer.resize(MIN(2 * oldC, BUFFER_SIZE_MAX));
		// kDebug(YAHOO_RAW_DEBUG) << "buffer resized from " << oldC << " to " << m_buffer.size();
	}
}


void SendFileTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendFileTask::setMessage( const QString &msg )
{
	m_msg = msg;
}

void SendFileTask::setFileUrl( KUrl url )
{
	m_url = url;

}

void SendFileTask::setTransferId( unsigned int transferId )
{
	m_transferId = transferId;
}

void SendFileTask::canceled( unsigned int id )
{
	if( m_transferId != id )
		return;
	
	if( m_socket )
		m_socket->close();
	
	setError();
}

QString SendFileTask::newYahooTransferId()
{
	// Adapted from libpurple/protocols/yahoo/yahoo_filexfer.c yahoo_xfer_new_xfer_id()

	QString newId;

	for(int i = 0; i < 22; i++)
	{
		char j = qrand() % 61;

		if(j < 26)
			newId += j + 'a';
		else if(j < 52)
			newId += j - 26 + 'A';
		else
			newId += j - 52 + '0';
	}

	newId += "$$";

	kDebug() << "New Yahoo Transfer Id: " << newId;

	return newId;
}

#include "sendfiletask.moc"

