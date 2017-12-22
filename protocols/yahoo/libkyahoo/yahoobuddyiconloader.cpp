/*
    yahoobuddyiconloader.cpp - Fetches YahooBuddyIcons

    Copyright (c) 2005 by André Duffeck <duffeck@kde.org>

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
#include "yahoo_protocol_debug.h"
#include <ktemporaryfile.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "yahootypes.h"
#include "client.h"

using namespace KYahoo;

YahooBuddyIconLoader::YahooBuddyIconLoader( Client *c )
: m_client( c )
{
}

YahooBuddyIconLoader::~YahooBuddyIconLoader()
{
}

void YahooBuddyIconLoader::fetchBuddyIcon( const QString &who, KUrl url, int checksum )
{
	qCDebug(YAHOO_PROTOCOL_LOG) << url;
	KIO::TransferJob *transfer;
	QString Url = url.url();
	QString ext = Url.left( Url.lastIndexOf( QLatin1String("?") ) );
	ext = ext.right( ext.length() - ext.lastIndexOf( QLatin1String(".") ) );
	
	transfer = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
	connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)) );
	connect( transfer, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );

	m_jobs[transfer].url = url;
	m_jobs[transfer].who = who;
	m_jobs[transfer].checksum = checksum;

}

void YahooBuddyIconLoader::slotData( KIO::Job *job, const QByteArray& data )
{

	qCDebug(YAHOO_PROTOCOL_LOG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	m_jobs[transfer].icon.append( data );

}

void YahooBuddyIconLoader::slotComplete( KJob *job )
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

	KIO::TransferJob *transfer = static_cast< KIO::TransferJob * >(job);

	if ( job->error () || transfer->isErrorPage () )
	{
		qCDebug(YAHOO_PROTOCOL_LOG) << "An error occurred while downloading buddy icon.";
		if( m_client )
			m_client->notifyError( i18n( "An error occurred while downloading a buddy icon (%1)", m_jobs[transfer].url.url() ), job->errorString(), Client::Info );
	}
	else
	{
		emit fetchedBuddyIcon( m_jobs[transfer].who, m_jobs[transfer].icon, m_jobs[transfer].checksum );
	}

	m_jobs.remove( transfer );
}

