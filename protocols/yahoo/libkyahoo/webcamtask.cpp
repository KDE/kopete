/*
    Kopete Yahoo Protocol
    Handles incoming webcam connections

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

#include "webcamtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qbuffer.h>
#include <qfile.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kstreamsocket.h>
using namespace KNetwork;

WebcamTask::WebcamTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

WebcamTask::~WebcamTask()
{
}

bool WebcamTask::take( Transfer* transfer )
{
	kdDebug(14181) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
 	if( t->service() == Yahoo::ServiceWebcam )
 		parseWebcamInformation( transfer );
// 	else
// 		parseMessage( transfer );

	return true;
}

bool WebcamTask::forMe( Transfer* transfer ) const
{
	kdDebug(14181) << k_funcinfo << endl;
	
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceWebcam )	
		return true;
	else
		return false;
}

void WebcamTask::requestWebcam( const QString &who )
{
	kdDebug(14181) << k_funcinfo << endl;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceWebcam);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId());
	t->setParam( 5, who );
	keyPending = who;

	send( t );
}

void WebcamTask::parseWebcamInformation( Transfer *transfer )
{
	kdDebug(14181) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return;


	YahooWebcamInformation info;
	info.sender = keyPending;
	info.server = t->firstParam( 102 );
	info.key = t->firstParam( 61 );
	info.status = InitialStatus;
	info.dataLength = 0;
	info.headerRead = false;
	
	kdDebug(14181) << k_funcinfo << "Got WebcamInformation: Sender: " << info.sender << " Server: " << info.server << " Key: " << info.key << endl;

	KStreamSocket *socket = new KStreamSocket( info.server, QString::number(5100) );
	socketMap[socket] = info;
	socket->enableRead( true );
	connect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage1Established() ) );
	connect( socket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
	connect( socket, SIGNAL( readyRead() ), this, SLOT( slotRead() ) );
	
	socket->connect();	
}

void WebcamTask::slotConnectionStage1Established()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	if( !socket )
		return;
	kdDebug(14181) << k_funcinfo << "Webcam connection Stage1 to the user " << socketMap[socket].sender << " established." << endl;
	disconnect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage1Established() ) );
	disconnect( socket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );

	socketMap[socket].status = ConnectedStage1;
	socket->writeBlock( QCString("<RVWCFG>").data(), 8 );

	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	QString s = QString("g=%1\r\n").arg(socketMap[socket].sender);

	// Header: 08 00 01 00 00 00 00	
	stream << (Q_INT8)0x08 << (Q_INT8)0x00 << (Q_INT8)0x01 << (Q_INT8)0x00 << (Q_INT32)s.length();
	stream.writeRawBytes( s.local8Bit(), s.length() );
	
	socket->writeBlock( buffer.data(), buffer.size() );
}

void WebcamTask::slotConnectionStage2Established()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	if( !socket )
		return;

	kdDebug(14181) << k_funcinfo << "Webcam connection Stage2 to the user " << socketMap[socket].sender << " established." << endl;
	disconnect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage2Established() ) );
	disconnect( socket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
	socketMap[socket].status = ConnectedStage2;

	// Send <REQIMG>-Packet
	socket->writeBlock( QCString("<REQIMG>").data(), 8 );

	// Send request information
	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	QString s = QString("a=2\r\nc=us\r\ne=21\r\nu=%1\r\nt=%2\r\ni=\r\ng=%3\r\no=w-2-5-1\r\np=1")
		.arg(client()->userId()).arg(socketMap[socket].key).arg(socketMap[socket].sender);	
	// Header: 08 00 01 00 00 00 00	
	stream << (Q_INT8)0x08 << (Q_INT8)0x00 << (Q_INT8)0x01 << (Q_INT8)0x00 << (Q_INT32)s.length();
	stream.writeRawBytes( s.local8Bit(), s.length() );	
	socket->writeBlock( buffer.data(), buffer.size() );
}

void WebcamTask::slotConnectionFailed( int error )
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	kdDebug(14181) << k_funcinfo << "Webcam connection to the user " << socketMap[socket].sender << " failed. Error " << error << " - " << socket->errorString() << endl;	
	socketMap.remove( socket );
}

void WebcamTask::slotRead()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	if( !socket )
		return;
	
	switch( socketMap[socket].status )
	{
		case ConnectedStage1:
			disconnect( socket, SIGNAL( readyRead() ), this, SLOT( slotRead() ) );
			kdDebug(14181) << k_funcinfo << "Connected into stage 1" << endl;
			connectStage2( socket );
		break;
		case ConnectedStage2:
			kdDebug(14181) << k_funcinfo << "Connected into stage 2" << endl;
			processData( socket );
		default:
		break;
	}
}

void WebcamTask::connectStage2( KStreamSocket *socket )
{
	kdDebug(14181) << k_funcinfo << endl;
	QByteArray data( socket->bytesAvailable() );
	socket->readBlock ( data.data (), data.size () );
	kdDebug(14181) << k_funcinfo << "Magic Byte:" << data[2] << endl;

	socketMap[socket].status = ConnectedStage2;
	if( data[2] == (Q_INT8)0x06 )
	{
		emit webcamNotAvailable(socketMap[socket].sender);
	}
	else if( data[2] == (Q_INT8)0x04 )
	{
		QString server;
		int i = 4;
		while( data[i] != (Q_INT8)0x00 )
			server += data[i++];
		kdDebug(14181) << k_funcinfo << "Server:" << server << endl;
// 		server = server.mid( 4, server.find( '0', 4) );
		
		kdDebug(14181) << k_funcinfo << "Connecting to " << server << endl;
		KStreamSocket *newSocket = new KStreamSocket( server, QString::number(5100) );
		socketMap[newSocket] = socketMap[socket];
		newSocket->enableRead( true );
		connect( newSocket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage2Established() ) );
		connect( newSocket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
		connect( newSocket, SIGNAL( readyRead() ), this, SLOT( slotRead() ) );
		
		newSocket->connect();	
	}
	socketMap.remove( socket );
	delete socket;
}

void WebcamTask::processData( KStreamSocket *socket )
{
// 	kdDebug(14181) << k_funcinfo << endl;
	QByteArray data( socket->bytesAvailable() );
	YahooWebcamInformation *info = &socketMap[socket];
	uint headerLength = 0;
	
	socket->readBlock ( data.data (), data.size () );
	if( data.size() <= 0 )
		return;
	if( !info->headerRead )
	{
		headerLength = data[0];	
		if( data.size() < headerLength )
			return;
		if( headerLength >= 8 )
		{
			kdDebug() << data[0] << data[1] << data[2] << data[3] << data[4] << data[5] << data[6] << data[7] << endl;
			info->reason = data[1];
			info->dataLength = yahoo_get32(data.data() + 4);
		}
		if( headerLength == 13 )
		{
			kdDebug() << data[8] << data[9] << data[10] << data[11] << data[12] << endl;
			info->timestamp = yahoo_get32(data.data() + 9);
			kdDebug(14181) << k_funcinfo << "PacketType: " << data[8] << " reason: " << info->reason << " timestamp: " << info->timestamp << endl;
			switch( data[8] )
			{
				case 0x00:
					if( info->timestamp == 0 )
					{
						emit webcamClosed( info->sender, 3 );
						cleanUpConnection( socket );
						return;
					}
				break;
				case 0x02: 
					info->type = Image;
				break;
				case 0x04:
					if( info->timestamp == 1 )
					{
						emit webcamPaused( info->sender );
						return;
					}
				break;
				case 0x07: 
					
					info->type = ConnectionClosed;
					emit webcamClosed( info->sender, info->reason );
					cleanUpConnection( socket );
					return;
				break;
			}
		}
		if( headerLength > 13 || headerLength <= 0)		//Parse error
			return;
		if( info->dataLength <= 0 )
			return;
		info->headerRead = true;		
		kdDebug(14181) << k_funcinfo << "Parsed Packet: HeaderLen: " << headerLength << " DataLen: " << info->dataLength << endl;
	}
	
	if( !info->buffer )
	{
		info->buffer = new QBuffer();
		info->buffer->open( IO_WriteOnly );
	}
	info->buffer->writeBlock( data.data() + headerLength, data.size() - headerLength );
	if( info->buffer->size() == static_cast<uint>(info->dataLength) )
	{
		info->buffer->close();
		QPixmap webcamImage;
		//webcamImage.loadFromData( info->buffer->buffer() );

		KTempFile jpcTmpImageFile;
		KTempFile bmpTmpImageFile;
		QFile *file = jpcTmpImageFile.file();;
		file->writeBlock((info->buffer->buffer()).data(), info->buffer->size());
		file->close();
		
		KProcess p;
		p << "jasper";
		p << "--input" << jpcTmpImageFile.name() << "--output" << bmpTmpImageFile.name() << "--output-format" << "bmp";
		
		p.start( KProcess::Block );
		if( p.exitStatus() != 0 )
		{
			kdDebug(14181) << " jasper exited with status " << p.exitStatus() << " " << info->sender << endl;
		}
		else
		{
			webcamImage.load( bmpTmpImageFile.name() );
			/******* UPTO THIS POINT ******/
			emit webcamImageReceived( info->sender, webcamImage );
		}
		QFile::remove(jpcTmpImageFile.name());
		QFile::remove(bmpTmpImageFile.name());

		kdDebug(14181) << k_funcinfo << "Image Received. Size: " << webcamImage.size() << endl;
	
		info->headerRead = false;
		delete info->buffer;
		info->buffer = 0L;
	}
}

void WebcamTask::cleanUpConnection( KStreamSocket *socket )
{
	socket->close();
	YahooWebcamInformation *info = &socketMap[socket];
	if( info->buffer )
		delete info->buffer;
	socketMap.remove( socket );
	delete socket;	
}

void WebcamTask::closeWebcam( const QString & who )
{
	kdDebug(14181) << k_funcinfo << endl;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		kdDebug(14181) << k_funcinfo << it.data().sender << " - " << who << endl;
		if( it.data().sender == who )
		{
			cleanUpConnection( it.key() );
			return;
		}
	}
	kdDebug(14181) << k_funcinfo << "Error. You tried to close a connection that didn't exist." << endl;
}

#include "webcamtask.moc"
