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
	m_socket->setBlocking( true );
	connect( m_socket, SIGNAL( connected( const KNetwork::KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( m_socket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

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

	QByteArray buffer;
	QDataStream stream( &buffer, QIODevice::WriteOnly );

	if ( m_file.open(QIODevice::ReadOnly ) )
	{
		kDebug(YAHOO_RAW_DEBUG) << "File successfully opened. Reading...";
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error opening file: " << m_file.errorString();
		client()->notifyError( i18n( "An error occurred while sending the file." ), m_file.errorString(), Client::Error );
		setError();
		return;
	}

	kDebug(YAHOO_RAW_DEBUG) << "Sizes: File (" << m_url << "): " << m_file.size();
	QString header = QString::fromLatin1("POST /relay?token=%1&sender=%2&recver=%3 HTTP/1.1\r\n"
					     "Cache-Control: no-cache\r\n"
					     "Cookie: T=%4; Y=%5\r\n"
					     "Host: %6\r\n"
					     "Content-Length: %7\r\n"
					     "User-Agent: Mozilla/5.0\r\n"
					     "Connection: Close\r\n\r\n")
		.arg(m_token).arg(client()->userId()).arg(m_target)
		.arg(client()->tCookie()).arg(client()->yCookie())
		.arg(m_relayHost)
		.arg(QString::number(m_file.size()));
	kDebug() << header;
	stream.writeRawData( header.toLocal8Bit(), header.length() );

	if( !m_socket->write( buffer ) )
	{
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		m_socket->close();
	}
	else
	{
		connect( m_socket, SIGNAL(readyWrite()), this, SLOT(transmitData()) );
		m_socket->enableWrite( true );
	}
}

void SendFileTask::transmitData()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	int read = 0;
	int written = 0;	
	char buf[1024];

	m_socket->enableWrite( false );
	read = m_file.read( buf, 1024 );
	written = m_socket->write( buf, read );
	kDebug(YAHOO_RAW_DEBUG) << "read:" << read << " written: " << written;

	m_transmitted += read;
	emit bytesProcessed( m_transferId, m_transmitted );

	if( written != read )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Failed!";
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		setError();
		return;
	}
	if( m_transmitted == m_file.size() )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Successful: " << m_transmitted;
		emit complete( m_transferId );
		setSuccess();
		m_socket->close();
	}
	else
	{
		m_socket->enableWrite( true );
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

