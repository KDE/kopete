/*
    Kopete Yahoo Protocol
    Handles incoming webcam connections

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

#include "webcamtask.h"
#include "sendnotifytask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include "webcamimgformat.h"

#include <QBuffer>
#include <QTimer>
#include <QPixmap>
#include <k3streamsocket.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KNetwork;
using namespace KYahoo;

WebcamTask::WebcamTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
	transmittingData = false;
	transmissionPending = false;
	timestamp = 1;
}

WebcamTask::~WebcamTask()
{
}

bool WebcamTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);
	
 	if( t->service() == Yahoo::ServiceWebcam )
 		parseWebcamInformation( t );
// 	else
// 		parseMessage( transfer );

	return true;
}

bool WebcamTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceWebcam )	
		return true;
	else
		return false;
}

void WebcamTask::requestWebcam( const QString &who )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceWebcam);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit());

	if (!who.isEmpty())
		t->setParam( 5, who.toLocal8Bit() );
	keysPending.append(who);

	send( t );
}

void WebcamTask::parseWebcamInformation( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YahooWebcamInformation info;
	if (!keysPending.isEmpty())
		info.sender = keysPending.takeFirst();

	info.server = t->firstParam( 102 );
	info.key = t->firstParam( 61 );
	info.status = InitialStatus;
	info.dataLength = 0;
	info.buffer = 0L;
	info.headerRead = false;
	if(info.sender.isEmpty()){
		info.sender = t->firstParam( 4 );
	}
	if( info.sender == client()->userId() )
	{
		transmittingData = true;
		info.direction = Outgoing;
	}
	else
	{
		info.direction = Incoming;
	}
	
	kDebug(YAHOO_RAW_DEBUG) << "Got WebcamInformation: Sender: " << info.sender << " Server: " << info.server << " Key: " << info.key;

	KStreamSocket *socket = new KStreamSocket( info.server, QString::number(5100) );
	socketMap[socket] = info;
	socket->enableRead( true );
	connect( socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(slotConnectionStage1Established()) );
	connect( socket, SIGNAL(gotError(int)), this, SLOT(slotConnectionFailed(int)) );
	connect( socket, SIGNAL(readyRead()), this, SLOT(slotRead()) );
	
	socket->connect();
	
}

void WebcamTask::slotConnectionStage1Established()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	if( !socket )
		return;
	kDebug(YAHOO_RAW_DEBUG) << "Webcam connection Stage1 to the user " << socketMap[socket].sender << " established.";
	disconnect( socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(slotConnectionStage1Established()) );
	disconnect( socket, SIGNAL(gotError(int)), this, SLOT(slotConnectionFailed(int)) );
	socketMap[socket].status = ConnectedStage1;
	

	QByteArray buffer;
	QDataStream stream( &buffer, QIODevice::WriteOnly );
	QString s;
	if( socketMap[socket].direction == Incoming )
	{
		socket->write( QByteArray("<RVWCFG>") );
		s = QString("g=%1\r\n").arg(socketMap[socket].sender);
	}
	else
	{
		socket->write( QByteArray("<RUPCFG>") );
		s = QString("f=1\r\n");
	}

	// Header: 08 00 01 00 00 00 00	
	stream << (qint8)0x08 << (qint8)0x00 << (qint8)0x01 << (qint8)0x00 << (qint32)s.length();
	stream.writeRawData( s.toLocal8Bit(), s.length() );
	
	socket->write( buffer.data(), buffer.size() );
}

void WebcamTask::slotConnectionStage2Established()
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	if( !socket )
		return;

	kDebug(YAHOO_RAW_DEBUG) << "Webcam connection Stage2 to the user " << socketMap[socket].sender << " established.";
	disconnect( socket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(slotConnectionStage2Established()) );
	disconnect( socket, SIGNAL(gotError(int)), this, SLOT(slotConnectionFailed(int)) );
	socketMap[socket].status = ConnectedStage2;

	QByteArray buffer;
	QDataStream stream( &buffer, QIODevice::WriteOnly );
	QString s;

	
	if( socketMap[socket].direction == Incoming )
	{
		// Send <REQIMG>-Packet
		socket->write( QByteArray("<REQIMG>") );
		// Send request information
		s = QString("a=2\r\nc=us\r\ne=21\r\nu=%1\r\nt=%2\r\ni=\r\ng=%3\r\no=w-2-5-1\r\np=1")
			.arg(client()->userId()).arg(socketMap[socket].key).arg(socketMap[socket].sender);
		// Header: 08 00 01 00 00 00 00	
		stream << (qint8)0x08 << (qint8)0x00 << (qint8)0x01 << (qint8)0x00 << (qint32)s.length();
	}
	else
	{
		// Send <REQIMG>-Packet
		socket->write( QByteArray("<SNDIMG>") );
		// Send request information
		s = QString("a=2\r\nc=us\r\nu=%1\r\nt=%2\r\ni=%3\r\no=w-2-5-1\r\np=2\r\nb=KopeteWebcam\r\nd=\r\n")
		.arg(client()->userId()).arg(socketMap[socket].key).arg(socket->localAddress().nodeName());
		// Header: 08 00 05 00 00 00 00	01 00 00 00 01
		stream << (qint8)0x0d << (qint8)0x00 << (qint8)0x05 << (qint8)0x00 << (qint32)s.length()
			<< (qint8)0x01 << (qint8)0x00 << (qint8)0x00 << (qint8)0x00 << (qint8)0x01;
	}
	socket->write( buffer.data(), buffer.size() );
	socket->write( s.toLocal8Bit(), s.length() );
}

void WebcamTask::slotConnectionFailed( int error )
{
	KStreamSocket* socket = const_cast<KStreamSocket*>( dynamic_cast<const KStreamSocket*>( sender() ) );
	kDebug(YAHOO_RAW_DEBUG) << "Webcam connection to the user " << socketMap[socket].sender << " failed. Error " << error << " - " << socket->errorString();
	client()->notifyError( i18n("Webcam connection to the user %1 could not be established.\n\nPlease relogin and try again.", socketMap[socket].sender), QString("%1 - %2").arg(error).arg( socket->errorString()), Client::Error );
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
			disconnect( socket, SIGNAL(readyRead()), this, SLOT(slotRead()) );
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
	kDebug(YAHOO_RAW_DEBUG) ;
	QByteArray data;
	data.reserve( socket->bytesAvailable() );
	data = socket->readAll();
	kDebug(YAHOO_RAW_DEBUG) << "Magic Byte:" << data[2];

	socketMap[socket].status = ConnectedStage2;

	QString server;
	int i = 4;
	KStreamSocket *newSocket;
	switch( (const char)data[2] )
	{
	case (qint8)0x06:
		emit webcamNotAvailable(socketMap[socket].sender);
		break;
	case (qint8)0x04:
	case (qint8)0x07:
		while( (const char)data[i] != (qint8)0x00 )
			server += data[i++];
		kDebug(YAHOO_RAW_DEBUG) << "Server:" << server;
		if( server.isEmpty() )
		{
			emit webcamNotAvailable(socketMap[socket].sender);
			break;
		}
		
		kDebug(YAHOO_RAW_DEBUG) << "Connecting to " << server;
		newSocket = new KStreamSocket( server, QString::number(5100) );
		socketMap[newSocket] = socketMap[socket];
		newSocket->enableRead( true );
		connect( newSocket, SIGNAL(connected(KNetwork::KResolverEntry)), this, SLOT(slotConnectionStage2Established()) );
		connect( newSocket, SIGNAL(gotError(int)), this, SLOT(slotConnectionFailed(int)) );
		connect( newSocket, SIGNAL(readyRead()), this, SLOT(slotRead()) );
		if( socketMap[newSocket].direction == Outgoing )
		{
			newSocket->enableWrite( true );
			connect( newSocket, SIGNAL(readyWrite()), this, SLOT(transmitWebcamImage()) );
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
	QByteArray data;
	data.reserve( socket->bytesAvailable() );
	
	data = socket->readAll();
	if( data.size() <= 0 )
	{
		kDebug(YAHOO_RAW_DEBUG) << "No data read.";
		return;
	}
	
	parseData( data, socket );
}

void WebcamTask::parseData( QByteArray &data, KStreamSocket *socket )
{
	int headerLength = 0;
	int read = 0;
	YahooWebcamInformation *info = &socketMap[socket];
	if( !info->headerRead )
	{
		headerLength = data[0];	
		kDebug(YAHOO_RAW_DEBUG) << "headerLength " << headerLength;
		if( data.size() < headerLength )
			return;			
		if( headerLength >= 8 )
		{
			kDebug() << data[0] << data[1] << data[2] << data[3] << data[4] << data[5] << data[6] << data[7];
			info->reason = data[1];
			info->dataLength = yahoo_get32(data.data() + 4);
		}
		if( headerLength == 13 )
		{
			kDebug() << data[8] << data[9] << data[10] << data[11] << data[12];
			info->timestamp = yahoo_get32(data.data() + 9);
			kDebug(YAHOO_RAW_DEBUG) << "PacketType: " << data[8] << " reason: " << info->reason << " timestamp: " << info->timestamp;
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
					kDebug(YAHOO_RAW_DEBUG) << "Ready for Transmission";
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
					// Send  very first Invitation packets
					for(it = pendingInvitations.begin(); it != pendingInvitations.end(); it++)
					{
						kDebug(YAHOO_RAW_DEBUG) << "send primary invitation";
						SendNotifyTask *snt = new SendNotifyTask( parent() );
						snt->setTarget( *it );
						snt->setType( SendNotifyTask::NotifyWebcamInvite );
						snt->go( true );
						it = pendingInvitations.erase( it );
						it--;
						info->status = SendingEmpty;
						emit stopTransmission();
						sendEmptyWebcamImage();
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
			kDebug(YAHOO_RAW_DEBUG) << "More data to read...";
			QByteArray newData;
			newData.reserve( data.size() - headerLength );
			QDataStream stream( &newData, QIODevice::WriteOnly );
			stream.writeRawData( data.data() + headerLength, data.size() - headerLength );
			parseData( newData, socket );
			return;
		}
		kDebug(YAHOO_RAW_DEBUG) << "Parsed Packet: HeaderLen: " << headerLength << " DataLen: " << info->dataLength;
	}
	
	if( info->dataLength <= 0 )
	{
		kDebug(YAHOO_RAW_DEBUG) << "No data to read. (info->dataLength <= 0)";
		if( info->headerRead )
			info->headerRead = false;
		return;
	}
	if( headerLength >= data.size() )
	{
		kDebug(YAHOO_RAW_DEBUG) << "No data to read. (headerLength >= data.size())";
		return;		//Nothing to read here...
	}
	if( !info->buffer )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Buffer created";
		info->buffer = new QBuffer();
		info->buffer->open( QIODevice::WriteOnly );
	}
	kDebug(YAHOO_RAW_DEBUG) << "data.size() " << data.size() << " headerLength " << headerLength << " buffersize " << info->buffer->size();
	read = headerLength + info->dataLength - info->buffer->size();
	info->buffer->write( data.data() + headerLength, data.size() - headerLength );//info->dataLength - info->buffer->size() );
	kDebug(YAHOO_RAW_DEBUG) << "read " << data.size() - headerLength << " Bytes, Buffer is now " << info->buffer->size();
	if( info->buffer->size() >= static_cast<uint>(info->dataLength) )
	{	
		info->buffer->close();
		QString who;
		switch( info->type )
		{
		case UserRequest:
			{
			who.append( info->buffer->buffer() );
			who = who.mid( 2, who.indexOf('\n') - 3);
			kDebug(YAHOO_RAW_DEBUG) << "User wants to view webcam: " << who << " len: " << who.length() << " Index: " << accessGranted.indexOf( who );
			if( accessGranted.indexOf( who ) >= 0 )
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
			kDebug(YAHOO_RAW_DEBUG) << "New Watcher of webcam: " << who;
			emit viewerJoined( who );
		break;
		case WatcherLeft:
			who.append( info->buffer->buffer() );
			who = who.left( who.length() - 1 );
			kDebug(YAHOO_RAW_DEBUG) << "A Watcher left: " << who << " len: " << who.length();
			accessGranted.removeAll( who );
			emit viewerLeft( who );
		break;
		case Image:
			{
			QPixmap webcamImage;
			//webcamImage.loadFromData( info->buffer->buffer() );
			if (WebcamImgFormat::instance())
			{
				if (WebcamImgFormat::instance()->fromYahoo(webcamImage, info->buffer->buffer().constData(), info->buffer->buffer().size()))
				{
					kDebug(YAHOO_RAW_DEBUG) << "Image Received and converted. Size: " << webcamImage.size();
					emit webcamImageReceived( info->sender, webcamImage );
				} else
					kDebug(YAHOO_RAW_DEBUG) << "Failed to convert incoming Yahoo webcam image";
			} else
				kDebug(YAHOO_RAW_DEBUG) << "Failed to initialize WebcamImgFormat helper";
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
		kDebug(YAHOO_RAW_DEBUG) << "More data to read..." << data.size() - read;
		QByteArray newData;
		newData.reserve( data.size() - read );
		QDataStream stream( &newData, QIODevice::WriteOnly );
		stream.writeRawData( data.data() + read, data.size() - read );
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
	kDebug(YAHOO_RAW_DEBUG) ;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		kDebug(YAHOO_RAW_DEBUG) << it.value().sender << " - " << who;
		if( it.value().sender == who )
		{
			cleanUpConnection( it.key() );
			return;
		}
	}
	kDebug(YAHOO_RAW_DEBUG) << "Error. You tried to close a connection that did not exist.";
	client()->notifyError( i18n( "An error occurred closing the webcam session. " ), i18n( "You tried to close a connection that did not exist." ), Client::Debug );
}


// Sending 

void WebcamTask::registerWebcam()
{	
	kDebug(YAHOO_RAW_DEBUG) ;
	
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceWebcam);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit());
	keysPending.append(client()->userId());

	send( t );
}

void WebcamTask::addPendingInvitation( const QString &userId )
{
	kDebug(YAHOO_RAW_DEBUG) << "Inviting " << userId << " to watch the webcam.";
	pendingInvitations.append( userId );
	accessGranted.append( userId );

}

void WebcamTask::grantAccess( const QString &userId )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.value().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error. No outgoing socket found.";
		return;
	}
	QByteArray ar;
	QDataStream stream( &ar, QIODevice::WriteOnly );
	QString user = QString("u=%1").arg(userId);

	stream << (qint8)0x0d << (qint8)0x00 << (qint8)0x05 << (qint8)0x00 << (qint32)user.length()
	<< (qint8)0x00 << (qint8)0x00 << (qint8)0x00 << (qint8)0x00 << (qint8)0x01;
	socket->write( ar.data(), ar.size() );
	socket->write( user.toLocal8Bit(), user.length() );
}

void WebcamTask::closeOutgoingWebcam()
{
	kDebug(YAHOO_RAW_DEBUG) ;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.value().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error. No outgoing socket found.";
		return;
	}
	
	cleanUpConnection( socket );
	transmittingData = false;
}

void WebcamTask::sendEmptyWebcamImage()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.value().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error. No outgoing socket found.";
		return;
	}
	if( socketMap[socket].status != SendingEmpty )
		return;	

	pictureBuffer.resize( 0 );
	transmissionPending = true;
	doPendingInvitations();
	QTimer::singleShot( 1000, this, SLOT(sendEmptyWebcamImage()) );

}

void WebcamTask::doPendingInvitations()
{
	QStringList::iterator itb;
	for(itb = pendingInvitations.begin(); itb != pendingInvitations.end(); itb++)
		{
		kDebug(YAHOO_RAW_DEBUG) << "send invitation when no users";
		SendNotifyTask *snt = new SendNotifyTask( parent() );
		snt->setTarget( *itb );
		snt->setType( SendNotifyTask::NotifyWebcamInvite );
		snt->go( true );
		itb = pendingInvitations.erase( itb );
		itb--;
		}
	
}

void WebcamTask::sendWebcamImage( const QByteArray &image )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	pictureBuffer = image;
	transmissionPending = true;
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	doPendingInvitations();
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.value().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error. No outgoing socket found.";
		return;
	}

	socket->enableWrite( true );
}

void WebcamTask::transmitWebcamImage()
{
	if( !transmissionPending )
		return;
	kDebug(YAHOO_RAW_DEBUG) << "arraysize: " << pictureBuffer.size();

	// Find outgoing socket
	KStreamSocket *socket = 0L;
	SocketInfoMap::Iterator it;
	for( it = socketMap.begin(); it != socketMap.end(); it++ )
	{
		if( it.value().direction == Outgoing )
		{
			socket = it.key();
			break;
		}
	}
	if( !socket )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Error. No outgoing socket found.";
		return;
	}

	socket->enableWrite( false );
	QByteArray buffer;
	QDataStream stream( &buffer, QIODevice::WriteOnly );
	stream << (qint8)0x0d << (qint8)0x00 << (qint8)0x05 << (qint8)0x00 << (qint32)pictureBuffer.size()
			<< (qint8)0x02 << (qint32)timestamp++;
	socket->write( buffer.data(), buffer.size() );
	if( pictureBuffer.size() )
		socket->write( pictureBuffer.data(), pictureBuffer.size() );
	
	transmissionPending = false;
}
#include "webcamtask.moc"
