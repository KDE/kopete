/*
    Kopete Yahoo Protocol
    Receive a file

    Copyright (c) 2006 Andre Duffeck <andre.duffeck@kdemail.net>

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
#include <qstring.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

ReceiveFileTask::ReceiveFileTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_file = new QFile( m_localUrl.path() );
	if( !m_file->open( IO_WriteOnly ) )
	{
		emit error( m_transferId, KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Could not open file for writing.") );
		setSuccess( false );
		return;
	}
	m_transferJob = KIO::get( m_remoteUrl, false, false );
	QObject::connect( m_transferJob, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
	QObject::connect( m_transferJob, SIGNAL( data( KIO::Job*, const QByteArray & ) ), this, SLOT( slotData( KIO::Job*, const QByteArray & ) ) );
}

void ReceiveFileTask::slotData( KIO::Job *job, const QByteArray& data )
{
	Q_UNUSED( job );
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	m_transmitted += data.size();
	emit bytesProcessed( m_transferId, m_transmitted );
	m_file->writeBlock( data.data() , data.size() );
	
}

void ReceiveFileTask::slotComplete( KIO::Job *job )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	m_file->close();
	if ( job->error () || transfer->isErrorPage () )
	{
		emit error( m_transferId, KIO::ERR_ABORTED, i18n("An error occured while uploading the file.") );
	}
	else
	{
		emit complete( m_transferId );
	}
}

void ReceiveFileTask::setRemoteUrl( KURL url )
{
	m_remoteUrl = url;
}

void ReceiveFileTask::setLocalUrl( KURL url )
{
	m_localUrl = url;
}

void ReceiveFileTask::setTransferId( unsigned int transferId )
{
	m_transferId = transferId;
}

#include "receivefiletask.moc"

