/*
    Kopete Yahoo Protocol
    sendpicturetask.cpp - Send our picture or information about it

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendpicturetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qfile.h>
#include <qdatastream.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <k3bufferedsocket.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KNetwork;
using namespace KYahoo;

SendPictureTask::SendPictureTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	m_socket = 0;
}

SendPictureTask::~SendPictureTask()
{
	delete m_socket;
}

void SendPictureTask::onGo()
{
	switch( m_type )
	{
		case UploadPicture:
			initiateUpload();
		break;
		case SendChecksum:
			sendChecksum();
		break;
		case SendInformation:
			sendInformation();
		break;
		case SendStatus:
			sendStatus();
		break;
	}
}

void SendPictureTask::initiateUpload()
{	
	kDebug(YAHOO_RAW_DEBUG) ;
	m_socket = new KBufferedSocket( "filetransfer.msg.yahoo.com", QString::number(80) );
	connect( m_socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(connectSucceeded()) );
	connect( m_socket, SIGNAL(gotError(int)), this, SLOT(connectFailed(int)) );
	connect( m_socket, SIGNAL(readyRead()), this, SLOT(readResult()) );

	m_socket->connect();
}

void SendPictureTask::connectFailed( int i)
{
	kDebug(YAHOO_RAW_DEBUG) << i << ": " << static_cast<const KBufferedSocket*>( sender() )->errorString();

	client()->notifyError(i18n("The picture was not successfully uploaded"), QString("%1 - %2").arg(i).arg(static_cast<const KBufferedSocket*>( sender() )->errorString()), Client::Error );
	setError();
}

void SendPictureTask::connectSucceeded()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	YMSGTransfer t(Yahoo::ServicePictureUpload);

	QFile file( m_path );

	t.setId( client()->sessionID() );
	t.setParam( 1, client()->userId().toLocal8Bit());
	t.setParam( 38, 604800);
	t.setParam( 0, client()->userId().toLocal8Bit());
	t.setParam( 28, file.size() );	
	t.setParam( 27, m_fileName.toLocal8Bit() );
	t.setParam( 14, "" );
	QByteArray buffer;
	QByteArray paket;
	QDataStream stream( &buffer, QIODevice::WriteOnly );

	if ( file.open(QIODevice::ReadOnly ) )
	{
		kDebug(YAHOO_RAW_DEBUG) << "File successfully opened. Reading...";
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error opening file: " << file.errorString();
		client()->notifyError(i18n("Error opening file: %1", m_path), file.errorString(), Client::Error );
		return;
	}

	paket = t.serialize();
	kDebug(YAHOO_RAW_DEBUG) << "Sizes: File (" << m_path << "): " << file.size() << " - paket: " << paket.size();
	QString header = QString::fromLatin1("POST /notifyft HTTP/1.1\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: filetransfer.msg.yahoo.com\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n").arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()).arg(file.size()+4+paket.size());
	stream.writeRawData( header.toLocal8Bit(), header.length() );
	stream.writeRawData( paket.data(), paket.size() );
	stream << (qint8)0x32 << (qint8)0x39 << (qint8)0xc0 << (qint8)0x80;
	stream.writeRawData( file.readAll(), file.size() );

	kDebug(YAHOO_RAW_DEBUG) << "Buffersize: " << buffer.size();
	if( m_socket->write( buffer, buffer.size() ) )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Successful!";
		m_socket->enableRead( true );
// 		setSuccess();
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Upload Failed!";
		setError();
	}
}

void SendPictureTask::readResult()
{
	kDebug(YAHOO_RAW_DEBUG) << m_socket->bytesAvailable();
	m_socket->enableRead( false );
	QByteArray buf;
	buf.resize( m_socket->bytesAvailable() );
	m_socket->read( buf.data(), m_socket->bytesAvailable() );

	if( buf.indexOf( "error", 0 ) >= 0 )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Picture upload failed";
		setError();
	}
	else
	{
		kDebug(YAHOO_RAW_DEBUG) << "Picture upload acknowledged.";
		setSuccess();
	}

}

void SendPictureTask::sendChecksum()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePictureChecksum);
	t->setId( client()->sessionID() );
	t->setParam(1, client()->userId().toLocal8Bit());
	if( !m_target.isEmpty() )
		t->setParam( 5, m_target.toLocal8Bit() );
	t->setParam(192, m_checksum);
	t->setParam(212, 1);
	send( t );
	
	setSuccess();
}

void SendPictureTask::sendInformation()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePicture);
	t->setId( client()->sessionID() );
	t->setParam(1, client()->userId().toLocal8Bit());
	t->setParam(13, 2 );
	t->setParam(5, m_target.toLocal8Bit() );
	t->setParam(20, m_url.toLocal8Bit() );
	t->setParam(192, m_checksum);

	send( t );
	
	setSuccess();
}

void SendPictureTask::sendStatus()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePictureStatus);
	t->setId( client()->sessionID() );
	t->setParam(3, client()->userId().toLocal8Bit());
	t->setParam(213, m_status );

	send( t );
	
	setSuccess();
}

void SendPictureTask::setType( Type type )
{
	m_type = type;
}

void SendPictureTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendPictureTask::setFilename( const QString &filename )
{
	m_fileName = filename;
}

void SendPictureTask::setFilesize( int filesize )
{
	m_fileSize = filesize;
}

void SendPictureTask::setPath( const QString &path )
{
	m_path = path;
}

void SendPictureTask::setChecksum( int checksum )
{
	m_checksum = checksum;
}

void SendPictureTask::setStatus( int status )
{
	m_status = status;
}

void SendPictureTask::setUrl( const QString &url )
{
	m_url = url;
}

#include "sendpicturetask.moc"
