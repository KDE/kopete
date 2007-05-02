/*
    Kopete Yahoo Protocol
    Handles several lists such as buddylist, ignorelist and so on

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>

#include "listtask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>
#include <kdebug.h>

ListTask::ListTask(Task* parent) : Task(parent)
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

ListTask::~ListTask()
{

}

bool ListTask::take( Transfer* transfer )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer *>(transfer);

	parseBuddyList( t );
	parseStealthList( t );

	return true;
}

bool ListTask::forMe( Transfer* transfer ) const
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServiceList )
		return true;
	else
		return false;
}

void ListTask::parseBuddyList( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString raw;
	m_list.append( t->firstParam( 87 ) );

	if( t->firstParam( 59 ).isEmpty() )
		return;

	QStringList groups;
	groups = QStringList::split( "\n", m_list );

	for ( QStringList::Iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) 
	{
		QString group = (*groupIt).section(":", 0, 0);
		QStringList buddies;
		buddies = QStringList::split( ",", (*groupIt).section(":", 1,1) );
		for ( QStringList::Iterator buddyIt = buddies.begin(); buddyIt != buddies.end(); ++buddyIt ) 
		{
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Parsed buddy: " << *buddyIt << " in group " << group << endl;
			emit gotBuddy( *buddyIt, QString::null, group );
		}
	}
	m_list.truncate( 0 );
}

void ListTask::parseStealthList( YMSGTransfer *t )
{
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString raw;
	raw = t->firstParam( 185 );

	QStringList buddies = QStringList::split( ",", raw );
	for ( QStringList::Iterator it = buddies.begin(); it != buddies.end(); ++it ) 
	{
		emit stealthStatusChanged( *it, Yahoo::StealthActive );
	}
}

#include "listtask.moc"
