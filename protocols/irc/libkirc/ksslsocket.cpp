/*
    ksslsocket.cpp - KDE SSL Socket

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <sys/select.h>

#include <qmutex.h>
#include <qthread.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kssl.h>

#include "ksslsocket.h"

struct KSSLSocketPrivate
{
	mutable KSSL *ssl;
	QMutex mutex;
	SSLPollThread *thread;
	int pendingBytes;
};

//Thread to poll the SSL socket
class SSLPollThread : public QThread
{
	public:
		SSLPollThread( KSSLSocket *s ) : QThread(), parent(s), exiting(false) {};

		void quit()
		{
			//Exit the thread
			exiting = true;
		}

	protected:
		void run()
		{
			while( !exiting )
			{
				/*
				 * Pending only returns the data that is in the internal
				 * buffer, that hasn't been picked up by read() yet. It
				 * doesn't check the socket. So, we need to peek the socket.
				 *
				 * peek blocks, which is why this reader thread exists.
				 */
				char buff[64];

				//Lock the mutex, so we will wait until the parent is done
				//reading any previously registered data
				parent->d->mutex.lock();

				//Register any pending bufer data first, then peek if none
				int bytes = parent->d->ssl->pending();
				if( bytes == 0 )
					bytes = parent->d->ssl->peek( buff, 64 );

				if( bytes )
				{
					//Unlock the mutex and register this data with the parent
					parent->d->mutex.unlock();
					parent->readData( bytes );
				}
			}
		}

	private:
		KSSLSocket *parent;
		bool exiting;

};

KSSLSocket::KSSLSocket() : KExtendedSocket()
{
	d = new KSSLSocketPrivate;
	d->ssl = 0L;
	d->pendingBytes = 0;
	d->thread = new SSLPollThread(this);

	//No blocking
	setBlockingMode(false);

	//Connect internal slots
	QObject::connect( this, SIGNAL(connectionSuccess()), this, SLOT(slotConnected()) );
	QObject::connect( this, SIGNAL(closed(int)), this, SLOT(slotDisconnected()) );
	QObject::connect( this, SIGNAL(connectionFailed(int)), this, SLOT(slotDisconnected()));
}

KSSLSocket::~KSSLSocket()
{
	//Close connection
	closeNow();

	//Get rid of reader thread
	d->thread->wait();
	delete d->thread;

	if( d->ssl )
	{
		d->ssl->close();
		delete d->ssl;
	}

	delete d;
}

Q_LONG KSSLSocket::readBlock( char* data, Q_ULONG maxLen )
{
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	Q_LONG retval = consumeReadBuffer(maxLen, data);

	if( retval == 0 )
	{
		if (sockfd == -1)
			return 0;

		retval = -1;
	}

	return retval;
}

int KSSLSocket::peekBlock( char* data, uint maxLen )
{
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	if( socketStatus() < connected )
		return -2;

	if( sockfd == -1 )
		return -2;

	return consumeReadBuffer(maxLen, data, false);
}

Q_LONG KSSLSocket::writeBlock( const char* data, Q_ULONG len )
{
	return d->ssl->write( data, len );
}

int KSSLSocket::bytesAvailable() const
{
	//Re-implemented because KExtSocket doesn't use this when not in buffered mode
	return KBufferedIO::bytesAvailable();
}

void KSSLSocket::readData( int bytes )
{
	//Lock the mutex, so the reader thread will block until we read this data
	d->mutex.lock();
	d->pendingBytes = bytes;
}

void KSSLSocket::slotPoll()
{
	//Check for registered data
	if( d->pendingBytes )
	{
		//Read in the data sent by the reader thread
		QByteArray buff(d->pendingBytes);
		int bytesRead = d->ssl->read( buff.data(), d->pendingBytes );

		//Fill the read buffer
		feedReadBuffer( bytesRead, buff.data() );
		d->pendingBytes = 0;
		emit readyRead();

		//Unlock the reader thread so it can now check for new data
		d->mutex.unlock();
	}

	if( socketStatus() == connected )
		QTimer::singleShot( 0, this, SLOT( slotPoll() ) );
}

void KSSLSocket::slotConnected()
{
	if( KSSL::doesSSLWork() )
	{
		kdDebug(14120) << k_funcinfo << "Trying SSL connection..." << endl;
		if( !d->ssl )
		{
			d->ssl = new KSSL();
			d->ssl->connect( fd() );
		}
		else
		{
			d->ssl->reInitialize();
		}

		//Start polling for data
		d->thread->start();
		slotPoll();
	}
	else
	{
		kdError(14120) << k_funcinfo << "SSL not functional!" << endl;

		d->ssl = 0L;
		emit sslFailure();
		closeNow();
	}
}

void KSSLSocket::slotDisconnected()
{
	d->thread->quit();
	d->thread->wait();
}


#include "ksslsocket.moc"
