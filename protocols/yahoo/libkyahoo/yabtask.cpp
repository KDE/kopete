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
#include <qdom.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>

YABTask::YABTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

YABTask::~YABTask()
{
}

bool YABTask::take( Transfer* transfer )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
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
// 	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
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
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "cookies: " << client()->yCookie() << endl;
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
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	m_data += info;
}

void YABTask::slotResult( KIO::Job* job )
{
	if ( job->error () || m_transferJob->isErrorPage () )
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Could not retrieve server side addressbook for user info." << endl;
	else 
	{
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Server side addressbook retrieved." << endl;
		QDomDocument doc;
		QDomNodeList list;
		QDomElement e;
		QString msg;
		uint it = 0;

		kDebug(YAHOO_RAW_DEBUG) << m_data << endl;
		doc.setContent( m_data );
		
		list = doc.elementsByTagName( "ct" );			// Get records
		for( it = 0; it < list.count(); it++ )	{
			kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Parsing entry..." << endl;
			if( !list.item( it ).isElement() )
				continue;
			e = list.item( it ).toElement();
			
			YABEntry *entry = new YABEntry;
			entry->yahooId = e.attribute("yi");
			entry->firstName = e.attribute("fn");
			entry->secondName = e.attribute("mn");
			entry->lastName = e.attribute("ln");
			entry->nickName = e.attribute("nn");
			entry->email = e.attribute("e0");
			entry->privatePhone = e.attribute("hp");
			entry->workPhone = e.attribute("wp");
			entry->pager = e.attribute("pa");
			entry->fax = e.attribute("fa");
			entry->phoneMobile = e.attribute("mo");
			entry->additionalNumber = e.attribute("ot");
			entry->altEmail1 = e.attribute("e1");
			entry->altEmail2 = e.attribute("e2");
			entry->privateURL = e.attribute("pu");
			entry->title = e.attribute("ti");
			entry->corporation = e.attribute("co");
			entry->workAdress = e.attribute("wa");
			entry->workCity = e.attribute("wc");
			entry->workState = e.attribute("ws");
			entry->workZIP = e.attribute("wz");
			entry->workCountry = e.attribute("wn");
			entry->workURL = e.attribute("wu");
			entry->privateAdress = e.attribute("ha");
			entry->privateCity = e.attribute("hc");
			entry->privateState = e.attribute("hs");
			entry->privateZIP = e.attribute("hz");
			entry->privateCountry = e.attribute("hn");
			QString birtday = e.attribute("bi");
			entry->birthday = QDate( birtday.section("/",2,2).toInt(), birtday.section("/",1,1).toInt(), birtday.section("/",0,0).toInt() );
			QString an = e.attribute("an");
			entry->anniversary = QDate( an.section("/",2,2).toInt(), an.section("/",1,1).toInt(), an.section("/",0,0).toInt() );
			entry->additional1 = e.attribute("c1");
			entry->additional2 = e.attribute("c2");
			entry->additional3 = e.attribute("c3");
			entry->additional4 = e.attribute("c4");
			entry->notes = e.attribute("cm");
			entry->imAIM = e.attribute("ima");
			entry->imGoogleTalk = e.attribute("img");
			entry->imICQ = e.attribute("imq");
			entry->imIRC = e.attribute("imc");
			entry->imMSN = e.attribute("imm");
			entry->imQQ = e.attribute("imqq");
			entry->imSkype = e.attribute("imk");
			
			emit gotEntry( entry );
		}
	}
}

#include "yabtask.moc"
