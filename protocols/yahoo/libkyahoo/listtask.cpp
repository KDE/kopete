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
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>
extern "C"
{
#include "libyahoo.h"
}

ListTask::ListTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

ListTask::~ListTask()
{

}

bool ListTask::take( Transfer* transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	parseCookies( transfer );
	parseBuddyList( transfer );	

	return true;
}

bool ListTask::forMe( Transfer* transfer ) const
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServiceList )
		return true;
	else
		return false;
}

void ListTask::parseBuddyList( Transfer *transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return;

	QString raw;
	raw = t->firstParam( "87" );
	
	if( raw.isEmpty() )
		return;

	QStringList groups;
	groups = QStringList::split( "\n", raw );

	for ( QStringList::Iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
		QString group = (*groupIt).section(":", 0, 0);
		QStringList buddies;
		buddies = QStringList::split( ",", (*groupIt).section(":", 1,1) );
		for ( QStringList::Iterator buddyIt = buddies.begin(); buddyIt != buddies.end(); ++buddyIt ) {
			kdDebug(14180) << k_funcinfo << "Parsed buddy: " << *buddyIt << " in group " << group << endl;
			emit gotBuddy( *buddyIt, QString::null, group );
		}
	}
}

void ListTask::parseCookies( Transfer *transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return;

	QStringList params;
	params = t->paramList( "59" );
	for ( QStringList::Iterator it = params.begin(); it != params.end(); ++it ) {
        	if( (*it).startsWith( "Y" ) )
		{
			m_yCookie = getcookie( (*it).latin1() );
			m_loginCookie = getlcookie( (*it).latin1() );
		}
		else if( (*it).startsWith( "T" ) )
		{
			m_tCookie = getcookie( (*it).latin1() );
		}
		else if( (*it).startsWith( "C" ) )
		{
			m_cCookie = getcookie( (*it).latin1() );
		}
    	}
	if( !m_yCookie.isEmpty() && !m_tCookie.isEmpty() &&
		!m_cCookie.isEmpty() )
		emit gotCookies();
}

const QString& ListTask::yCookie() const
{
	return m_yCookie;
}

const QString& ListTask::tCookie() const
{
	return m_tCookie;
}

const QString& ListTask::cCookie() const
{
	return m_cCookie;
}

const QString& ListTask::loginCookie() const
{
	return m_loginCookie;
}

#include "listtask.moc"
