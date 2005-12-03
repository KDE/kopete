/*
    Kopete Yahoo Protocol
    sendpicturetask.cpp - Send our picture or information about it

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

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
#include <qcstring.h>
#include <qdatastream.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kbufferedsocket.h>

SendPictureTask::SendPictureTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

SendPictureTask::~SendPictureTask()
{
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
		case SendStatus:
			sendStatus();
		break;
	}
}

void SendPictureTask::initiateUpload()
{	
	kdDebug(14181) << k_funcinfo << endl;
	KBufferedSocket* yahooSocket = new KBufferedSocket( "filetransfer.msg.yahoo.com", QString::number(80) );
	connect( yahooSocket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( connectSucceeded() ) );
	connect( yahooSocket, SIGNAL( gotError(int) ), this, SLOT( connectFailed(int) ) );

	yahooSocket->connect();
}

void SendPictureTask::connectFailed( int i)
{
	kdDebug(14181) << k_funcinfo << i << ": " << dynamic_cast<const KStreamSocket*>( sender() )->errorString() << endl;
}

void SendPictureTask::connectSucceeded()
{
	kdDebug(14181) << k_funcinfo << endl;
	KStreamSocket* socket = const_cast<KBufferedSocket*>( dynamic_cast<const KBufferedSocket*>( sender() ) );
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePictureUpload);

	QFile file( m_path );

	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId());
	t->setParam( 38, 604800);
	t->setParam( 0, client()->userId());
	t->setParam( 28, file.size() );	
	t->setParam( 27, m_fileName );
	t->setParam( 14, QString::fromLatin1("") );
	QByteArray buffer;
	QByteArray paket;
	QDataStream stream( buffer, IO_WriteOnly );

	if ( file.open(IO_ReadOnly ) )
	{
		kdDebug(14181) << k_funcinfo << "File successfully opened. Reading..." << endl;
	}
	else
	{
		kdDebug(14181) << k_funcinfo << "Error opening file: " << file.errorString() << endl;
		return;
	}

	paket = t->serialize();
	kdDebug(14181) << k_funcinfo << "Sizes: File (" << m_path << "): " << file.size() << " - paket: " << paket.size() << endl;
	QString header = QString::fromLatin1("POST /notifyft HTTP/1.1\r\n"
			"Referer: blubb-dwsgqbxiu\r\n"
			"Cookie: Y=%1; T=%2; C=%3 ;B=fckeert1kk1nl&b=2\r\n"
			"User-Agent: Mozilla/4.0 (compatible; MSIE 5.5)\r\n"
			"Host: filetransfer.msg.yahoo.com\r\n"
			"Content-length: %4\r\n"
			"Cache-Control: no-cache\r\n\r\n").arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()).arg(file.size()+4+paket.size());
	stream.writeRawBytes( header.local8Bit(), header.length() );
	stream.writeRawBytes( paket.data(), paket.size() );
	stream << (Q_INT8)0x32 << (Q_INT8)0x39 << (Q_INT8)0xc0 << (Q_INT8)0x80;
	stream.writeRawBytes( file.readAll(), file.size() );

	kdDebug(14181) << k_funcinfo << "Buffersize: " << buffer.size() << endl;
	if( socket->writeBlock( buffer, buffer.size() ) )
		kdDebug(14181) << k_funcinfo << "Upload Successful!" << endl;
	else
		kdDebug(14181) << k_funcinfo << "Upload Failed!" << endl;
	socket->close();

}

void SendPictureTask::slotUploadFinished( KIO::Job *job )
{
	kdDebug(14181) << k_funcinfo << endl;
	if ( job->error() )
		setSuccess( false );
	else
		setSuccess( true );
}

void SendPictureTask::sendChecksum()
{
	kdDebug(14180) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePictureChecksum);
	t->setId( client()->sessionID() );
	t->setParam(1, client()->userId());
	if( !m_target.isEmpty() )
		t->setParam( 5, m_target );
	t->setParam(192, m_checksum);
	t->setParam(212, 1);
	send( t );
	
	setSuccess( true );
}

void SendPictureTask::sendInformation()
{
	kdDebug(14180) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePicture);
	t->setId( client()->sessionID() );
	t->setParam(1, client()->userId());
	t->setParam(4, client()->userId());
	t->setParam(13, 2 );
	t->setParam(5, m_target );
	t->setParam(20, m_url );
	t->setParam(192, m_checksum);

	send( t );
	
	setSuccess( true );
}

void SendPictureTask::sendStatus()
{
	kdDebug(14180) << k_funcinfo << endl;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePictureUpdate);
	t->setId( client()->sessionID() );
	t->setParam(1, client()->userId());
	t->setParam(5, m_target );
	t->setParam(206, m_status);

	send( t );
	
	setSuccess( true );
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

