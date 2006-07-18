/*
    Kopete Yahoo Protocol
    Receive a file

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

#include "receivefiletask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <QString>
#include <QTimer>
#include <QFile>
#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

ReceiveFileTask::ReceiveFileTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_transmitted = 0;
	m_file = 0;
	m_transferJob = 0;
}

ReceiveFileTask::~ReceiveFileTask()
{
	delete m_file;
	m_file = 0;
}

void ReceiveFileTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7);
	switch( m_type )
	{
	case FileTransferAccept:
		m_file = new QFile( m_localUrl.path() );
		if( !m_file->open( QIODevice::WriteOnly ) )
		{
			emit error( m_transferId, KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Could not open file for writing.") );
			setSuccess( false );
			return;
		}
		m_transferJob = KIO::get( m_remoteUrl, false, false );
		QObject::connect( m_transferJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
		QObject::connect( m_transferJob, SIGNAL( data( KIO::Job*, const QByteArray & ) ), this, SLOT( slotData( KIO::Job*, const QByteArray & ) ) );
		delete t;
		break;
	case FileTransfer7Accept:
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().local8Bit() );
		t->setParam( 5, m_userId.local8Bit() );
		t->setParam( 265, m_remoteUrl.url().local8Bit() );
		t->setParam( 222, 3 );
	
		send( t );
		break;
	case FileTransfer7Reject:
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().local8Bit() );
		t->setParam( 5, m_userId.local8Bit() );
		t->setParam( 265, m_remoteUrl.url().local8Bit() );
		t->setParam( 222, 4 );
	
		send( t );
		break;
	default:
		delete t;
	}
}

bool ReceiveFileTask::take( Transfer* transfer )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	parseFileTransfer7Info( t );
	
	return true;
}

bool ReceiveFileTask::forMe( const Transfer *transfer ) const
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if( t->service() == Yahoo::ServiceFileTransfer7Info )
		return true;
	else
		return false;
}

void ReceiveFileTask::slotData( KIO::Job *job, const QByteArray& data )
{
	Q_UNUSED( job );
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	m_transmitted += data.size();
	emit bytesProcessed( m_transferId, m_transmitted );
	m_file->writeBlock( data.data() , data.size() );
	
}

void ReceiveFileTask::slotComplete( KIO::Job *job )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if( m_file )
		m_file->close();
	if ( job->error () || transfer->isErrorPage () )
	{
		emit error( m_transferId, KIO::ERR_ABORTED, i18n("An error occured while downloading the file.") );
		setSuccess( false );
	}
	else
	{
		emit complete( m_transferId );
		setSuccess( true );
	}
}

void ReceiveFileTask::parseFileTransfer7Info( YMSGTransfer *transfer )
{	
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	if( transfer->firstParam( 249 ).toInt() == 1 )
	{
		// Reject P2P Transfer offer
		YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Accept);
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().local8Bit() );
		t->setParam( 5, transfer->firstParam( 4 ) );
		t->setParam( 265, transfer->firstParam( 265 ) );
		t->setParam( 66, -3 );
	
		send( t );
	}
	else if( transfer->firstParam( 249 ).toInt() == 3 )
	{
		m_file = new QFile( m_localUrl.path() );
		if( !m_file->open( QIODevice::WriteOnly ) )
		{
			emit error( m_transferId, KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Could not open file for writing.") );
			setSuccess( false );
			return;
		}

		YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Accept);
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().local8Bit() );
		t->setParam( 5, transfer->firstParam( 4 ) );
		t->setParam( 265, transfer->firstParam( 265 ) );
		t->setParam( 27, transfer->firstParam( 27 ) );
		t->setParam( 249, 3 );			// Use Reflection server
		t->setParam( 251, transfer->firstParam( 251 ) );
	
		send( t );
		// The server expects a HTTP HEAD command prior to the GET
		m_mimetypeJob = KIO::mimetype(QString::fromLatin1("http://%1/relay?token=%2&sender=%3&recver=%4")
				.arg( QString(transfer->firstParam( 250 )) ).arg( QString(transfer->firstParam( 251 )) ).arg(m_userId).arg(client()->userId()), false);
		m_mimetypeJob->addMetaData("cookies", "manual");
		m_mimetypeJob->addMetaData("setcookies", QString::fromLatin1("Cookie: T=%1; path=/; domain=.yahoo.com; Y=%2; C=%3;")
				.arg(client()->tCookie()).arg(client()->yCookie()).arg(client()->cCookie()) );


		m_transferJob = KIO::get( QString::fromLatin1("http://%1/relay?token=%2&sender=%3&recver=%4")
				.arg( QString(transfer->firstParam( 250 )) ).arg( QString(transfer->firstParam( 251 )) ).arg(m_userId).arg(client()->userId()), false, false );
		QObject::connect( m_transferJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
		QObject::connect( m_transferJob, SIGNAL( data( KIO::Job*, const QByteArray & ) ), this, SLOT( slotData( KIO::Job*, const QByteArray & ) ) );
		m_transferJob->addMetaData("cookies", "manual");
		m_transferJob->addMetaData("setcookies", QString::fromLatin1("Cookie: T=%1; path=/; domain=.yahoo.com; Y=%2; C=%3;")
				.arg(client()->tCookie()).arg(client()->yCookie()).arg(client()->cCookie()) );
	}
}

void ReceiveFileTask::setRemoteUrl( KUrl url )
{
	m_remoteUrl = url;
}

void ReceiveFileTask::setLocalUrl( KUrl url )
{
	m_localUrl = url;
}

void ReceiveFileTask::setTransferId( unsigned int transferId )
{
	m_transferId = transferId;
}

void ReceiveFileTask::setType( Type type )
{
	m_type = type;
}

void ReceiveFileTask::setUserId( const QString &userId )
{
	m_userId = userId;
}

#include "receivefiletask.moc"

