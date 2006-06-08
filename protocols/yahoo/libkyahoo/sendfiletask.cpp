/*
    Kopete Yahoo Protocol
    Send a file

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>

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
#include <kstreamsocket.h>
#include <kio/global.h>

using namespace KNetwork;

SendFileTask::SendFileTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QTimer::singleShot( 0, this, SLOT(initiateUpload()) );
}

void SendFileTask::initiateUpload()
{	
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_socket = new KStreamSocket( "filetransfer.msg.yahoo.com", QString::number(80) );
	m_socket->setBlocking( true );
	connect( m_socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( m_socket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

	m_socket->connect();
}

void SendFileTask::connectFailed( int i )
{
	QString err = m_socket->errorString();
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << i << ": " << err << endl;
	emit error( m_transferId, i, err );
	setSuccess( false );
}

void SendFileTask::connectSucceeded()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	YMSGTransfer t( Yahoo::ServiceFileTransfer );

	m_file.setName( m_url.path() );

	t.setId( client()->sessionID() );
	t.setParam( 0, client()->userId().local8Bit());
	t.setParam( 5, m_target.local8Bit());
	t.setParam( 28, m_file.size() );	
	t.setParam( 27, m_url.fileName().local8Bit() );
	t.setParam( 14, "" );
	QByteArray buffer;
	QByteArray paket;
	QDataStream stream( buffer, IO_WriteOnly );

	if ( m_file.open(IO_ReadOnly ) )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "File successfully opened. Reading..." << endl;
	}
	else
	{
		client()->notifyError( i18n( "An error occured sending the file." ), m_file.errorString(), Client::Error );
		setSuccess( false );
		return;
	}

	paket = t.serialize();
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Sizes: File (" << m_url << "): " << m_file.size() << " - paket: " << paket.size() << endl;
	QString header = QString::fromLatin1("POST http://filetransfer.msg.yahoo.com:80/notifyft HTTP/1.1\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;B=fckeert1kk1nl&b=2\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: filetransfer.msg.yahoo.com:80\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n").arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()).arg(m_file.size()+4+paket.size());
	stream.writeRawBytes( header.local8Bit(), header.length() );
	stream.writeRawBytes( paket.data(), paket.size() );
	stream << (Q_INT8)0x32 << (Q_INT8)0x39 << (Q_INT8)0xc0 << (Q_INT8)0x80;

	if( !m_socket->writeBlock( buffer, buffer.size() ) )
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	int read = 0;
	int written = 0;	
	char buf[1024];

	m_socket->enableWrite( false );
	read = m_file.readBlock( buf, 1024 );
	written = m_socket->writeBlock( buf, read );
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "read:" << read << " written: " << written << endl;

	m_transmitted += read;
	emit bytesProcessed( m_transferId, m_transmitted );

	if( written != read )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Upload Failed!" << endl;
		emit error( m_transferId, m_socket->error(), m_socket->errorString() );
		setSuccess( false );
		return;
	}
	if( m_transmitted == m_file.size() )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Upload Successful: " << m_transmitted << endl;
		emit complete( m_transferId );
		setSuccess( true );
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

void SendFileTask::setFileUrl( KURL url )
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
	
	setSuccess( false );
}

#include "sendfiletask.moc"

