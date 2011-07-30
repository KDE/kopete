/*
    Kopete Yahoo Protocol
    Receive a file

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

#include "receivefiletask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"

#include <QTimer>
#include <QFile>
#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

ReceiveFileTask::ReceiveFileTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
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
	kDebug(YAHOO_RAW_DEBUG) ;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7);
	switch( m_type )
	{
	case FileTransferAccept:
		m_file = new QFile( m_localUrl.toLocalFile() );
		if( !m_file->open( QIODevice::WriteOnly ) )
		{
			emit error( m_transferId, KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Could not open file for writing.") );
			setError();
			delete t;
			return;
		}
		m_transferJob = KIO::get( m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo );
		QObject::connect( m_transferJob, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)) );
		QObject::connect( m_transferJob, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );
		delete t;
		break;
	case FileTransfer7Accept:
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, m_userId.toLocal8Bit() );
		t->setParam( 265, m_remoteUrl.url().toLocal8Bit() );
		t->setParam( 222, 3 );
	
		send( t );
		break;
	case FileTransfer7Reject:
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, m_userId.toLocal8Bit() );
		t->setParam( 265, m_remoteUrl.url().toLocal8Bit() );
		t->setParam( 222, 4 );
	
		send( t );
		break;
	default:
		delete t;
	}
}

bool ReceiveFileTask::take( Transfer* transfer )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	parseFileTransfer7Info( t );
	
	return true;
}

bool ReceiveFileTask::forMe( const Transfer *transfer ) const
{
	kDebug(YAHOO_RAW_DEBUG) ;
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if( t->service() == Yahoo::ServiceFileTransfer7Info )
	{
		// Only take this transfer if we are the corresponding task (in case of simultaneous file transfers)
		if( t->firstParam( 265 ) == m_remoteUrl.url().toLocal8Bit() )
			return true;
		else
			return false;
	}	
	else
		return false;
}

void ReceiveFileTask::slotData( KIO::Job *job, const QByteArray& data )
{
	Q_UNUSED( job );
	kDebug(YAHOO_RAW_DEBUG) ;

	m_transmitted += data.size();
	emit bytesProcessed( m_transferId, m_transmitted );
	m_file->write( data );
	
}


void ReceiveFileTask::slotHeadComplete( KJob *job )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	bool got_error = job->error () || transfer->isErrorPage ();

	/* SEEME:
	   Following check disabled for now because Yahoo is sending a non http conforming answer ('\r\n' and *then* the HTTP response headers.
	   That makes kio slave http not want to read more and parse the headers. 
	   So.. leaving this comment here until yahoo is sending a correct http response so that we can correctly check if the HEAD cmd
	   succeeded.
        */
#if 0
	if (!got_error)
	{
		got_error = transfer->queryMetaData(QLatin1String("HTTP-Headers")).length() <= 5; // I was getting 2 bytes in such error cases.
	}
#endif
	if (got_error)
	{
		emit error( m_transferId, KIO::ERR_ABORTED, i18n("An error occurred while downloading the file.") );
		setError();
	}
	else
	{
		// same URL from the HEAD cmd
		m_transferJob = KIO::get( transfer->url(), KIO::Reload, KIO::HideProgressInfo );

		QObject::connect( m_transferJob, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)) );
		QObject::connect( m_transferJob, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );

        	setCommonTransferMetaData(m_transferJob);
	}
	m_mimetypeJob = 0;
}


void ReceiveFileTask::slotComplete( KJob *job )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if( m_file )
		m_file->close();
	if ( job->error () || transfer->isErrorPage () )
	{
		emit error( m_transferId, KIO::ERR_ABORTED, i18n("An error occurred while downloading the file.") );
		setError();
	}
	else
	{
		emit complete( m_transferId );
		setSuccess();
	}
	m_transferJob = 0;
}

void ReceiveFileTask::parseFileTransfer7Info( YMSGTransfer *transfer )
{	
	kDebug(YAHOO_RAW_DEBUG) ;

	if( transfer->firstParam( 249 ).toInt() == 1 )
	{
		// Reject P2P Transfer offer
		YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Accept);
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, transfer->firstParam( 4 ) );
		t->setParam( 265, transfer->firstParam( 265 ) );
		t->setParam( 66, -3 );
	
		send( t );
	}
	else if( transfer->firstParam( 249 ).toInt() == 3 )
	{
		m_file = new QFile( m_localUrl.toLocalFile() );
		if( !m_file->open( QIODevice::WriteOnly ) )
		{
			emit error( m_transferId, KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Could not open file for writing.") );
			setError();
			return;
		}

		YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceFileTransfer7Accept);
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, transfer->firstParam( 4 ) );
		t->setParam( 265, transfer->firstParam( 265 ) );
		t->setParam( 27, transfer->firstParam( 27 ) );
		t->setParam( 249, 3 );			// Use Reflection server
		t->setParam( 251, transfer->firstParam( 251 ) );
	
		send( t );
		// The server expects a HTTP HEAD command prior to the GET
		m_mimetypeJob = KIO::mimetype(QString(
				QString::fromLatin1("http://%1/relay?token=")
					.arg( QString(transfer->firstParam( 250 )) ) 
				+
				QString(QUrl::toPercentEncoding(QString(transfer->firstParam( 251 )))) +
				QString::fromLatin1("&sender=%1&recver=%2")
					.arg(m_userId)
					.arg(client()->userId()))
				, 
				KIO::HideProgressInfo);
		m_mimetypeJob->addMetaData("cookies", "manual");
		setCommonTransferMetaData(m_mimetypeJob);
		QObject::connect( m_mimetypeJob, SIGNAL(result(KJob*)), this, SLOT(slotHeadComplete(KJob*)) );

	}
}


void ReceiveFileTask::setCommonTransferMetaData(KIO::TransferJob* job)
{
	job->addMetaData(QString::fromLatin1("accept"), "*/*"); // Accept header
	job->addMetaData(QString::fromLatin1("UserAgent"), "Mozilla/5.0");
	job->addMetaData(QString::fromLatin1("cache"), "reload");

	job->addMetaData("cookies", "manual");
 	job->addMetaData("setcookies", QString::fromLatin1("Cookie: T=%1; Y=%2;")
 				.arg(client()->tCookie()).arg(client()->yCookie()) );
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

void ReceiveFileTask::canceled( unsigned int id )
{
	if( m_transferId != id )
		return;

	if( m_mimetypeJob )
	{
		m_mimetypeJob->kill();
		m_mimetypeJob = 0L;
	}
	if( m_transferJob )
	{
		m_transferJob->kill();
		m_transferJob = 0L;
	}
	
	setError();
}

#include "receivefiletask.moc"

