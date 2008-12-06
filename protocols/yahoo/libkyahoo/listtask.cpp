/*
    Kopete Yahoo Protocol
    Handles several lists such as buddylist, ignorelist and so on

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

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
#include <qstringlist.h>
#include <kdebug.h>

ListTask::ListTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

ListTask::~ListTask()
{

}

bool ListTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer *>(transfer);

	parseBuddyList( t );
	parseStealthList( t );

	return true;
}

bool ListTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServiceBuddyList )
		return true;
	else
		return false;
}

void ListTask::parseBuddyList( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString group;

	// We need some low-level parsing here
	foreach( Param p, t->paramList() )
	{
		switch( p.first )
		{
		case 65:
			group = p.second;
			break;
		case 7:
			emit gotBuddy( p.second, QString(), group );
		}
	}
}

void ListTask::parseStealthList( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString raw;
	raw = t->firstParam( 185 );

	const QStringList buddies = raw.split( ',', QString::SkipEmptyParts );
	for ( QStringList::ConstIterator it = buddies.begin(); it != buddies.end(); ++it )
	{
		emit stealthStatusChanged( *it, Yahoo::StealthActive );
	}
}

#include "listtask.moc"
