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

#include "listtask.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "transfer.h"
#include "ymsgtransfer.h"
#include "client.h"
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
	QString buddy;
	// We need some low-level parsing here
	
	foreach( const Param &p, t->paramList() )
	{
		kDebug(YAHOO_RAW_DEBUG) << "1:" << p.first ;
		kDebug(YAHOO_RAW_DEBUG) << "2:" << p.second ;
		switch( p.first )
		{
		case 65:
			group = p.second;
			break;
		case 7:
			buddy = p.second;
			break;
		case 301:
			if( p.second == "319"){
				emit gotBuddy( buddy, QString(), group );
			}
			break;
		case 317:
			if( p.second == "2"){
				kDebug(YAHOO_RAW_DEBUG) << "Stealthed setting on" << buddy ;
				emit stealthStatusChanged( buddy, Yahoo::StealthActive );
			};
			break;
			/**
			* Note: michaelacole
			* Other buddy codes are here for add to list and blacklist
			* I will need to capute more codes for addition here.
			* Blacklist is done on the server at Yahoo whereas
			* Kopete has its own plugin for blacklisting.
			*/
		}
	}
	/**
	* Note: michaelacole
	* Since you can log in from other places and remove or add Perm Offline status
	* We have to reset both conditions at login
	* Yahoo sends this data at this time,
	* so better to compile list of both now then notify kopete client.
	*/
}



#include "listtask.moc"
