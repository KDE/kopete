/*
    Kopete Yahoo Protocol
    yabtask.h - Handles the Yahoo Address Book

    Copyright (c) 2006 André Duffeck <duffeck@kde.org>
    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "yabtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qdatastream.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <klocale.h>

using namespace KYahoo;

YABTask::YABTask(Task* parent) : Task(parent)
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
}

YABTask::~YABTask()
{
}

bool YABTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);
	
	if( t->service() == Yahoo::ServiceContactDetails )
		parseContactDetails( t );
	
	return true;
}

bool YABTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceContactDetails )	
		return true;
	else
		return false;
}

void YABTask::parseContactDetails( YMSGTransfer* t )
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

	QString from;		/* key = 7  */
	int count;

	from = t->firstParam( 4 );
	count = t->paramCount( 5 );
	
	for( int i = 0; i < count; i++ )
	{
		QString who = t->nthParam( 5, i );
		QString s = t->nthParamSeparated( 280, i, 5 );
		if( s.isEmpty() )
			continue;

		QDomDocument doc;
		doc.setContent( s );
		YABEntry *entry = new YABEntry;
		entry->fromQDomDocument( doc );
		entry->source = YABEntry::SourceContact;
		entry->dump();
		emit gotEntry( entry );
	}
}

void YABTask::getAllEntries( long lastMerge, long lastRemoteRevision )
{
	qCDebug(YAHOO_PROTOCOL_LOG) << "LastMerge: " << lastMerge << " LastRemoteRevision: " << lastRemoteRevision;
	m_data.clear();
	QString url = QStringLiteral("http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&diffs=1&t=%1&tags=short&rt=%2&prog-ver=%3")
		.arg( lastMerge ).arg( lastRemoteRevision ).arg( YMSG_PROGRAM_VERSION_STRING );

	m_transferJob = KIO::get( url , KIO::NoReload, KIO::HideProgressInfo );
	m_transferJob->addMetaData(QStringLiteral("cookies"), QStringLiteral("manual"));
	m_transferJob->addMetaData(QStringLiteral("setcookies"), QStringLiteral("Cookie: Y=%1; T=%2; C=%3;")
				.arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()) );
	connect( m_transferJob, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );
	connect( m_transferJob, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)) );
}

void YABTask::slotData( KIO::Job* /*job*/, const QByteArray &info  )
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	m_data += info;
}

void YABTask::slotResult( KJob* job )
{
	if( job->error () || ( m_transferJob && m_transferJob->isErrorPage() ) )
	{
		qCDebug(YAHOO_PROTOCOL_LOG) << "Could not retrieve server side addressbook for user info.";
		client()->notifyError( i18n( "Could not retrieve server side address book for user info." ), job->errorString(), Client::Info );
	}
	else 
	{
		qCDebug(YAHOO_PROTOCOL_LOG) << "Server side addressbook retrieved.";
		QDomDocument doc;
		QDomNodeList list;
		QDomElement e;
		int it = 0;

		qCDebug(YAHOO_PROTOCOL_LOG) << m_data;
		doc.setContent( m_data );
		
		list = doc.elementsByTagName( QStringLiteral("ab") );			// Get the Addressbook
		for( it = 0; it < list.count(); it++ )	{
			if( !list.item( it ).isElement() )
				continue;
			e = list.item( it ).toElement();
			
			if( !e.attribute( QStringLiteral("lm") ).isEmpty() )
				emit gotRevision( e.attribute( QStringLiteral("lm") ).toLong(), true );

			if( !e.attribute( QStringLiteral("rt") ).isEmpty() )
				emit gotRevision( e.attribute( QStringLiteral("rt") ).toLong(), false );
		}
		
		list = doc.elementsByTagName( QStringLiteral("ct") );			// Get records
		for( it = 0; it < list.count(); it++ )	{
			if( !list.item( it ).isElement() )
				continue;
			e = list.item( it ).toElement();
			
			YABEntry *entry = new YABEntry;
			entry->fromQDomElement( e );
			entry->source = YABEntry::SourceYAB;
			emit gotEntry( entry );
		}
	}
}

