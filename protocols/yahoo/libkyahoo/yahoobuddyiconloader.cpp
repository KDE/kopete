/*
    yahoobuddyiconloader.cpp - Fetches YahooBuddyIcons

    Copyright (c) 2005 by André Duffeck <andre@duffeck.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoobuddyiconloader.h"

// QT Includes
#include <qfile.h>

// KDE Includes
#include <kdebug.h>
#include <ktempfile.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kurl.h>
#include <kstandarddirs.h>

YahooBuddyIconLoader::YahooBuddyIconLoader()
{
}

YahooBuddyIconLoader::~YahooBuddyIconLoader()
{
}

void YahooBuddyIconLoader::fetchBuddyIcon( const QString &who, KURL url, int checksum )
{
	kdDebug(14180) << k_funcinfo << endl;
	KIO::TransferJob *transfer;
	QString Url = url.url();
	QString ext = Url.left( Url.findRev( "?" ) );
	ext = ext.right( ext.length() - ext.findRev( "." ) );
	//Url.mid( Url.findRev( "."), Url.findRev( "?") - Url.findRev( ".") - 1);
	
	transfer = KIO::get( url, false, false );
	connect( transfer, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
	connect( transfer, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );

	m_jobs[transfer].url = url;
	m_jobs[transfer].who = who;
	m_jobs[transfer].checksum = checksum;
	m_jobs[transfer].file = new KTempFile( locateLocal( "tmp", "yahoobuddyicon-" ), ext );
	m_jobs[transfer].file->setAutoDelete( true );

}

void YahooBuddyIconLoader::slotData( KIO::Job *job, const QByteArray& data )
{

	kdDebug(14180) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if( m_jobs[transfer].file )
		m_jobs[transfer].file->file()->writeBlock( data.data() , data.size() );

}

void YahooBuddyIconLoader::slotComplete( KIO::Job *job )
{
	kdDebug(14180) << k_funcinfo << endl;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		kdDebug(14180) << k_funcinfo << "An error occured while downloading buddy icon!" << endl;
	}
	else
	{
		if ( m_jobs[transfer].file )
		{
			m_jobs[transfer].file->close();
			emit fetchedBuddyIcon( m_jobs[transfer].who, m_jobs[transfer].file, m_jobs[transfer].checksum );
		}
		else
		{
			kdDebug(14180) << k_funcinfo << "Fatal Error! IconLoadJob has an empty KTempFile pointer." << endl;
		}
	}

	m_jobs.remove( transfer );
}



#include "yahoobuddyiconloader.moc"

