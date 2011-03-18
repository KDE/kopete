/*
	kopetesockettimeoutwatcher.cpp - Kopete Socket Timeout Watcher

	Copyright (c) 2009 Roman Jarosz <kedgedev@centrum.cz>
	Kopete (c) 2009 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/
#include "kopetesockettimeoutwatcher.h"

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#endif

#include <kdebug.h>

#include <QAbstractSocket>
#include <QTimer>
#include <QHostAddress>

namespace Kopete
{

QSet<QAbstractSocket*> SocketTimeoutWatcher::watchedSocketSet;

SocketTimeoutWatcher* SocketTimeoutWatcher::watch( QAbstractSocket* socket, quint32 msecTimeout )
{
	if (watchedSocketSet.contains(socket))
	{
		kDebug() << "Socket is already being watched " << socket;
		return 0;
	}

	return new Kopete::SocketTimeoutWatcher( socket, msecTimeout );
}

SocketTimeoutWatcher::SocketTimeoutWatcher( QAbstractSocket* socket, quint32 msecTimeout )
: QObject(socket), mSocket(socket), mActive(true), mTimeoutThreshold(msecTimeout)
{
	watchedSocketSet.insert( mSocket );
	mAckCheckTimer = new QTimer();

	connect( socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten()) );
	connect( socket, SIGNAL(disconnected()), mAckCheckTimer, SLOT(stop()) );
	connect( socket, SIGNAL(error(QAbstractSocket::SocketError)), mAckCheckTimer, SLOT(stop()) );

	connect( mAckCheckTimer, SIGNAL(timeout()), this, SLOT(ackTimeoutCheck()) );
	mAckCheckTimer->setInterval( mTimeoutThreshold );
}

SocketTimeoutWatcher::~SocketTimeoutWatcher()
{
	watchedSocketSet.remove( mSocket );
	delete mAckCheckTimer;
}

void SocketTimeoutWatcher::bytesWritten()
{
	if ( !mActive )
		return;

	if ( mSocket->socketDescriptor() != -1 )
	{
		if ( !mAckCheckTimer->isActive() )
			mAckCheckTimer->start();
	}
	else
	{
		if ( mAckCheckTimer->isActive() )
			mAckCheckTimer->stop();
	}
	
}

void SocketTimeoutWatcher::ackTimeoutCheck()
{
	const int sDesc = mSocket->socketDescriptor();
	if ( sDesc != -1 )
	{
#ifdef Q_OS_LINUX
		struct tcp_info info;
		int info_length = sizeof(info);
		if ( getsockopt( sDesc, SOL_TCP, TCP_INFO, (void*)&info, (socklen_t*)&info_length ) == 0 )
		{
			if ( info.tcpi_last_ack_recv < info.tcpi_last_data_sent || info.tcpi_last_data_sent <= 0 )
			{
				mAckCheckTimer->stop();
			}
 			else if ( info.tcpi_last_ack_recv > info.tcpi_last_data_sent && info.tcpi_last_data_sent > mTimeoutThreshold )
			{
				kWarning() << "Connection timeout for " << mSocket->peerAddress();
				mAckCheckTimer->stop();
				emit error( QAbstractSocket::RemoteHostClosedError );
				emit errorInt( QAbstractSocket::RemoteHostClosedError );
				mSocket->abort();
			}
			return;
		}
#endif

		if ( mActive )
		{
			mAckCheckTimer->stop();
			mActive = false;
			kWarning() << "Timeout watcher not active for " << mSocket->peerAddress();
		}
	}
	else
	{
		mAckCheckTimer->stop();
	}
}

}

#include "kopetesockettimeoutwatcher.moc"
