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
#include "sendnotifytask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qbuffer.h>
#include <qfile.h>
#include <qtimer.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <kstreamsocket.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KNetwork;

WebcamTask::WebcamTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	transmittingData = false;
	transmissionPending = false;
	timestamp = 1;
}

WebcamTask::~WebcamTask()
{
}

bool WebcamTask::take( Transfer* transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);
	
 	if( t->service() == Yahoo::ServiceWebcam )
 		parseWebcamInformation( t );
// 	else
// 		parseMessage( transfer );

	return true;
}

bool WebcamTask::forMe( Transfer* transfer ) const
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceWebcam);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().local8Bit());
	t->setParam( 5, who.local8Bit() );
	keyPending = who;

	send( t );
}

void WebcamTask::parseWebcamInformation( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	YahooWebcamInformation info;
	info.sender = keyPending;
	info.server = t->firstParam( 102 );
	info.key = t->firstParam( 61 );
	info.status = InitialStatus;
	info.dataLength = 0;
	info.buffer = 0L;
	info.headerRead = false;
	if( info.sender == client()->userId() )
	{
		transmittingData = true;
		info.direction = Outgoing;
	}
	else
		info.direction = Incoming;
	
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Got WebcamInformation: Sender: " << info.sender << " Server: " << info.server << " Key: " << info.key << endl;

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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Webcam connection Stage1 to the user " << socketMap[socket].sender << " established." << endl;
	disconnect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage1Established() ) );
	disconnect( socket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
	socketMap[socket].status = ConnectedStage1;
	

	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	QString s;
	if( socketMap[socket].direction == Incoming )
	{
		socket->writeBlock( QCString("<RVWCFG>").data(), 8 );
		s = QString("g=%1\r\n").arg(socketMap[socket].sender);
	}
	else
	{
		socket->writeBlock( QCString("<RUPCFG>").data(), 8 );
		s = QString("f=1\r\n");
	}

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

	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Webcam connection Stage2 to the user " << socketMap[socket].sender << " established." << endl;
	disconnect( socket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage2Established() ) );
	disconnect( socket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
	socketMap[socket].status = ConnectedStage2;

	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	QString s;


	if( socketMap[socket].direction == Incoming )
	{
		// Send <REQIMG>-Packet
		socket->writeBlock( QCString("<REQIMG>").data(), 8 );
		// Send request information
		s = QString("a=2\r\nc=us\r\ne=21\r\nu=%1\r\nt=%2\r\ni=\r\ng=%3\r\no=w-2-5-1\r\np=1")
			.arg(client()->userId()).arg(socketMap[socket].key).arg(socketMap[socket].sender);
		// Header: 08 00 01 00 00 00 00	
		stream << (Q_INT8)0x08 << (Q_INT8)0x00 << (Q_INT8)0x01 << (Q_INT8)0x00 << (Q_INT32)s.length();
	}
	else
	{
		// Send <REQIMG>-Packet
		socket->writeBlock( QCString("<SNDIMG>").data(), 8 );
		// Send request information
		s = QString("a=2\r\nc=us\r\nu=%1\r\nt=%2\r\ni=%3\r\no=w-2-5-1\r\np=2\r\nb=KopeteWebcam\r\nd=\r\n")
		.arg(client()->userId()).arg(socketMap[socket].key).arg(socket->localAddress().nodeName());
		// Header: 08 00 05 00 00 00 00	01 00 00 00 01
		stream << (Q_INT8)0x0d << (Q_INT8)0x00 << (Q_INT8)0x05 << (Q_INT8)0x00 << (Q_INT32)s.length()
			<< (Q_INT8)0x01 << (Q_INT8)0x00 << (Q_INT8)0x00 << (Q_INT8)0x00 << (Q_INT8)0x01;
	}
	socket->writeBlock( buffer.data(), buffer.size() );
	socket->writeBlock( s.local8Bit(), s.length() );
}

void WebcamTask::slotConnectionFailed( int error )
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	client()->notifyError( i18n("Webcam connection to the user %1 could not be established.\n\nPlease relogin and try again.")
			.arg(socketMap[socket].sender), QString("%1 - %2").arg(error).arg( socket->errorString()), Client::Error );
	socketMap.remove( socket );
	socket->deleteLater();
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
			connectStage2( socket );
		break;
		case ConnectedStage2:
		case Sending:
		case SendingEmpty:
			processData( socket );
		default:
		break;
	}
}

void WebcamTask::connectStage2( KStreamSocket *socket )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	QByteArray data( socket->bytesAvailable() );
	socket->readBlock ( data.data (), data.size () );
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Magic Byte:" << data[2] << endl;

	socketMap[socket].status = ConnectedStage2;

	QString server;
	int i = 4;
	KStreamSocket *newSocket;
	switch( (const char)data[2] )
	{
	case (Q_INT8)0x06:
		emit webcamNotAvailable(socketMap[socket].sender);
		break;
	case (Q_INT8)0x04:
	case (Q_INT8)0x07:
		while( (const char)data[i] != (Q_INT8)0x00 )
			server += data[i++];
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Server:" << server << endl;
		if( server.isEmpty() )
		{
			emit webcamNotAvailable(socketMap[socket].sender);
			break;
		}
		
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Connecting to " << server << endl;
		newSocket = new KStreamSocket( server, QString::number(5100) );
		socketMap[newSocket] = socketMap[socket];
		newSocket->enableRead( true );
		connect( newSocket, SIGNAL( connected( const KResolverEntry& ) ), this, SLOT( slotConnectionStage2Established() ) );
		connect( newSocket, SIGNAL( gotError(int) ), this, SLOT( slotConnectionFailed(int) ) );
		connect( newSocket, SIGNAL( readyRead() ), this, SLOT( slotRead() ) );
		if( socketMap[newSocket].direction == Outgoing )
		{
			newSocket->enableWrite( true );
			connect( newSocket, SIGNAL( readyWrite() ), this, SLOT( transmitWebcamImage() ) );
		}
		
		newSocket->connect();	
		break;
	default:
		break;
	}
	socketMap.remove( socket );
	delete socket;
}

void WebcamTask::processData( KStreamSocket *socket )
{
	QByteArray data( socket->bytesAvailable() );
	
	socket->readBlock ( data.data (), data.size () );
	if( data.size() <= 0 )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "No data read." << endl;
		return;
	}
	
	parseData( data, socket );
}

void WebcamTask::parseData( QByteArray &data, KStreamSocket *socket )
{
	uint headerLength = 0;
	uint read = 0;
	YahooWebcamInformation *info = &socketMap[socket];
	if( !info->headerRead )
	{
		headerLength = data[0];	
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "headerLength " << headerLength << endl;
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
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "PacketType: " << data[8] << " reason: " << info->reason << " timestamp: " << info->timestamp << endl;
			QStringList::iterator it;
			switch( data[8] )
			{
				case 0x00:
					if( info->direction == Incoming )
					{
						if( info->timestamp == 0 )
						{
							emit webcamClosed( info->sender, 3 );
							cleanUpConnection( socket );
						}
					}
					else
					{
						info->type = UserRequest;
						info->headerRead = true;
					}
				break;
				case 0x02: 
					info->type = Image;
					info->headerRead = true;
				break;
				case 0x04:
					if( info->timestamp == 1 )
					{
						emit webcamPaused( info->sender );
					}
				break;
				case 0x05:
					kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Ready for Transmission. " << info->timestamp << " watchers." << endl;
					if( info->timestamp == 1 )
					{
						info->status = Sending;
						emit readyForTransmission();
					}
					else if( info->timestamp == 0 )
					{
						info->status = SendingEmpty;
						emit stopTransmission();
						sendEmptyWebcamImage();
					}
					
					// Send Invitation packets
					for(it = pendingInvitations.begin(); it != pendingInvitations.end(); it++)
					{
						SendNotifyTask *snt = new SendNotifyTask( parent() );
						snt->setTarget( *it );
						snt->setType( SendNotifyTask::NotifyWebcamInvite );
						snt->go( true );
						it = pendingInvitations.remove( it );
						it--;
					}
				break;
				case 0x07: 
					
					info->type = ConnectionClosed;
					emit webcamClosed( info->sender, info->reason );
					cleanUpConnection( socket );
				case 0x0c:
					info->type = NewWatcher;
					info->headerRead = true;
				break;
				case 0x0d:
					info->type = WatcherLeft;
					info->headerRead = true;
				break;
			}
		}
		if( headerLength > 13 || headerLength <= 0)		//Parse error
			return;
		if( !info->headerRead && data.size() > headerLength )
		{
			// More headers to read
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "More data to read..." << endl;
			QByteArray newData( data.size() - headerLength );
			QDataStream stream( newData, IO_WriteOnly );
			stream.writeRawBytes( data.data() + headerLength, data.size() - headerLength );
			parseData( newData, socket );
			return;
		}
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Parsed Packet: HeaderLen: " << headerLength << " DataLen: " << info->dataLength << endl;
	}
	
	if( info->dataLength <= 0 )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "No data to read. (info->dataLength <= 0)" << endl;
		if( info->headerRead )
			info->headerRead = false;
		return;
	}
	if( headerLength >= data.size() )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "No data to read. (headerLength >= data.size())" << endl;
		return;		//Nothing to read here...
	}
	if( !info->buffer )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Buffer created" << endl;
		info->buffer = new QBuffer();
		info->buffer->open( IO_WriteOnly );
	}
	
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "data.size() " << data.size() << " headerLength " << headerLength << " buffersize " << info->buffer->size() << endl;
	read = headerLength + info->dataLength - info->buffer->size();
	info->buffer->writeBlock( data.data() + headerLength, data.size() - headerLength );//info->dataLength - info->buffer->size() );
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "read " << data.size() - headerLength << " Bytes, Buffer is now " << info->buffer->size() << endl;
	if( info->buffer->size() >= static_cast<uint>(info->dataLength) )
	{	
		info->buffer->close();
		QString who;
		switch( info->type )
		{
		case UserRequest:
			{
			who.append( info->buffer->buffer() );
			who = who.mid( 2, who.find('\n') - 3);
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "User wants to view webcam: " << who << " len: " << who.length() << " Index: " << accessGranted.findIndex( who ) << endl;
			if( accessGranted.findIndex( who ) >= 0 )
			{
				grantAccess( who );
			}
			else
				emit viewerRequest( who );
			}
		break;
		case NewWatcher:
			who.append( info->buffer->buffer() );
			who = who.left( who.length() - 1 );
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "New Watcher of webcam: " << who << endl;
			emit viewerJoined( who );
		break;
		case WatcherLeft:
			who.append( info->buffer->buffer() );
			who = who.left( who.length() - 1 );
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "A Watcher left: " << who << " len: " << who.length() << endl;
			accessGranted.remove( who );
			emit viewerLeft( who );
		break;
		case Image:
			{
			QPixmap webcamImage;
			//webcamImage.loadFromData( info->buffer->buffer() );
	
			KTempFile jpcTmpImageFile;
			KTempFile bmpTmpImageFile;
			QFile *file = jpcTmpImageFile.file();
			file->writeBlock((info->buffer->buffer()).data(), info->buffer->size());
			file->close();
			
			KProcess p;
			p << "jasper";
			p << "--input" << jpcTmpImageFile.name() << "--output" << bmpTmpImageFile.name() << "--output-format" << "bmp";
			
			p.start( KProcess::Block );
			if( p.exitStatus() != 0 )
			{
				kdDebug(YAHOO_RAW_DEBUG) << " jasper exited with status " << p.exitStatus() << " " << info->sender << endl;
			}
			else
			{
				webcamImage.load( bmpTmpImageFile.name() );
				/******* UPTO THIS POINT ******/
				emit webcamImageReceived( info->sender, webcamImage );
			}
			QFile::remove(jpcTmpImageFile.name());
			QFile::remove(bmpTmpImageFile.name());
	
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Image Received. Size: " << webcamImage.size() << endl;
			}
		break;
		default:
		break;
		}
		
		info->headerRead = false;
		delete info->buffer;
		info->buffer = 0L;
	}
	if( data.size() > read )
	{
		// More headers to read
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "More data to read..." << data.size() - read << endl;
		QByteArray newData( data.size() - read );
		QDataStream stream( newData, IO_WriteOnly );
		stream.writeRawBytes( data.data() + read, data.size() - read );
		parseData( newData, socket );
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << it.data().sender << " - " << who << endl;
		if( it.data().sender == who )
		{
			cleanUpConnection( it.key() );
			return;
		}
	}
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. You tried to close a connection that didn't exist." << endl;
	client()->notifyError( i18n( "An error occured closing the webcam session. " ), i18n( "You tried to close a connection that didn't exist." ), Client::Debug );
}


// Sending 

void WebcamTask::registerWebcam()
{	
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceWebcam);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().local8Bit());
	keyPending  = client()->userId();

	send( t );
}

void WebcamTask::addPendingInvitation( const QString &userId )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Inviting " << userId << " to watch the webcam." << endl;
	pendingInvitations.append( userId );
	accessGranted.append( userId );
}

void WebcamTask::grantAccess( const QString &userId )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.data().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. No outgoing socket found." << endl;
		return;
	}
	QByteArray ar;
	QDataStream stream( ar, IO_WriteOnly );
	QString user = QString("u=%1").arg(userId);

	stream << (Q_INT8)0x0d << (Q_INT8)0x00 << (Q_INT8)0x05 << (Q_INT8)0x00 << (Q_INT32)user.length()
	<< (Q_INT8)0x00 << (Q_INT8)0x00 << (Q_INT8)0x00 << (Q_INT8)0x00 << (Q_INT8)0x01;
	socket->writeBlock( ar.data(), ar.size() );
	socket->writeBlock( user.local8Bit(), user.length() );
}

void WebcamTask::closeOutgoingWebcam()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.data().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. No outgoing socket found." << endl;
		return;
	}
	
	cleanUpConnection( socket );
	transmittingData = false;
}

void WebcamTask::sendEmptyWebcamImage()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.data().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. No outgoing socket found." << endl;
		return;
	}
	if( socketMap[socket].status != SendingEmpty )
		return;	

	pictureBuffer.resize( 0 );
	transmissionPending = true;

	QTimer::singleShot( 1000, this, SLOT(sendEmptyWebcamImage()) );

}

void WebcamTask::sendWebcamImage( const QByteArray &image )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	pictureBuffer.duplicate( image );
	transmissionPending = true;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.data().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. No outgoing socket found." << endl;
		return;
	}

	socket->enableWrite( true );
}

void WebcamTask::transmitWebcamImage()
{
	if( !transmissionPending )
		return;
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "arraysize: " << pictureBuffer.size() << endl;

	// Find outgoing socket
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.data().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Error. No outgoing socket found." << endl;
		return;
	}

	socket->enableWrite( false );
	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	stream << (Q_INT8)0x0d << (Q_INT8)0x00 << (Q_INT8)0x05 << (Q_INT8)0x00 << (Q_INT32)pictureBuffer.size()
			<< (Q_INT8)0x02 << (Q_INT32)timestamp++;
	socket->writeBlock( buffer.data(), buffer.size() );
	if( pictureBuffer.size() )
		socket->writeBlock( pictureBuffer.data(), pictureBuffer.size() );
	
	transmissionPending = false;
}
#include "webcamtask.moc"
