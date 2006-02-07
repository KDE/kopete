/*
    Kopete Yahoo Protocol
    yabtask.h - Handles the Yahoo Address Book

    Copyright (c) 2006 André Duffeck <andre.duffeck@kdemail.net>
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

YABTask::YABTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

YABTask::~YABTask()
{
}

bool YABTask::take( Transfer* transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
	return true;
}

bool YABTask::forMe( Transfer* transfer ) const
{
// 	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( 0 )	
		return true;
	else
		return false;
}

void YABTask::getAllEntries()
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "cookies: " << client()->yCookie() << endl;
	m_data = QString::null;
	QString url = QString::fromLatin1("http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&diffs=1&t=1139039075&tags=short&rt=1139039099&prog-ver=%1")
		.arg( YMSG_PROGRAM_VERSION_STRING );

	m_transferJob = KIO::get( url , false, false );
	m_transferJob->addMetaData("cookies", "manual");
	m_transferJob->addMetaData("setcookies", QString::fromLatin1("Cookie: Y=%1; T=%2; C=%3;")
				.arg(client()->yCookie()).arg(client()->tCookie()).arg(client()->cCookie()) );
	connect( m_transferJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ), this, SLOT( slotData( KIO::Job*, const QByteArray & ) ) );
	connect( m_transferJob, SIGNAL( result( KIO::Job *) ), this, SLOT( slotResult( KIO::Job* ) ) );
}

void YABTask::slotData( KIO::Job* /*job*/, const QByteArray &info  )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_data += info;
}

void YABTask::slotResult( KIO::Job* job )
{
	if( job->error () || m_transferJob->isErrorPage () )
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Could not retrieve server side addressbook for user info." << endl;
	else 
	{
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Server side addressbook retrieved." << endl;
		QDomDocument doc;
		QDomNodeList list;
		QDomElement e;
		uint it = 0;

		kdDebug(YAHOO_RAW_DEBUG) << m_data << endl;
		doc.setContent( m_data );
		
		list = doc.elementsByTagName( "ct" );			// Get records
		for( it = 0; it < list.count(); it++ )	{
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Parsing entry..." << endl;
			if( !list.item( it ).isElement() )
				continue;
			e = list.item( it ).toElement();
			
			YABEntry *entry = new YABEntry;
			entry->fromQDomElement( e );
			emit gotEntry( entry );
		}
	}
}

#include "yabtask.moc"
