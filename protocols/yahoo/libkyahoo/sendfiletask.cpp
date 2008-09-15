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
#include <kdebug.h>
#include <klocale.h>
#include <k3streamsocket.h>
#include <kio/global.h>

using namespace KNetwork;

SendFileTask::SendFileTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_transmitted = 0;
	m_socket = 0;
}

SendFileTask::~SendFileTask()
{
	m_socket->deleteLater();
	m_socket = 0;
}

void SendFileTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QTimer::singleShot( 0, this, SLOT(initiateUpload()) );
}

void SendFileTask::initiateUpload()
{	
	kDebug(YAHOO_RAW_DEBUG) ;
	m_socket = new KStreamSocket( "filetransfer.msg.yahoo.com", QString::number(80) );
	m_socket->setBlocking( true );
	connect( m_socket, SIGNAL( connected( const KNetwork::KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( m_socket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

	m_socket->connect();
}

void SendFileTask::connectFailed( int i )
{
	QString err = m_socket->errorString();
	kDebug(YAHOO_RAW_DEBUG) << i << ": " << err;
	emit error( m_transferId, i, err );
	setError();
}

void SendFileTask::connectSucceeded()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	YMSGTransfer t( Yahoo::ServiceFileTransfer );

	m_file.setFileName( m_url.path() );

	t.setId( client()->sessionID() );
	t.setParam( 0, client()->userId().toLocal8Bit());
	t.setParam( 5, m_target.toLocal8Bit());
	t.setParam( 28, m_file.size() );	
	t.setParam( 27, m_url.fileName().toLocal8Bit() );
	t.setParam( 14, "" );
	QByteArray buffer;
	QByteArray paket;
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

	paket = t.serialize();
	kDebug(YAHOO_RAW_DEBUG) << "Sizes: File (" << m_url << "): " << m_file.size() << " - paket: " << paket.size();
	QString header = QString::fromLatin1("POST http://filetransfer.msg.yahoo.com:80/notifyft HTTP/1.1\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;B=fckeert1kk1nl&b=2\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: filetransfer.msg.yahoo.com:80\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n").arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()).arg(m_file.size()+4+paket.size());
	stream.writeRawData( header.toLocal8Bit(), header.length() );
	stream.writeRawData( paket.data(), paket.size() );
	stream << (qint8)0x32 << (qint8)0x39 << (qint8)0xc0 << (qint8)0x80;

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

#include "sendfiletask.moc"

