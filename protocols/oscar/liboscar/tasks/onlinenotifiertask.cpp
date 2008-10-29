/*
	Kopete Oscar Protocol
	onlinenotifiertask.cpp - handles all the status notifications
	
	Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
	
	Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
	Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
	
	Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
	
	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/
#include "onlinenotifiertask.h"
#include "buffer.h"
#include "connection.h"
#include "oscartypes.h"
#include "transfer.h"

#include <kdebug.h>

OnlineNotifierTask::OnlineNotifierTask( Task* parent ) : Task( parent )
{
}


OnlineNotifierTask::~OnlineNotifierTask()
{
}


bool OnlineNotifierTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	
	if ( st->snacService() == 0x0003 )
	{
		switch ( st->snacSubtype() )
		{
		case 0x000B:
		case 0x000C:
			return true;
		};
	}
	return false;
}

bool OnlineNotifierTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
		if ( st )
		{
			setTransfer( transfer );
			
			if ( st->snacSubtype() == 0x000B )
				userOnline();
			else
				userOffline();
			
			setTransfer( 0 );
		}
		return true;
	}
	return false;
}

void OnlineNotifierTask::userOnline()
{
	Buffer* buffer = transfer()->buffer();
	UserDetails ud;
	ud.fill( buffer );
	QString user = ud.userId();
	//kDebug( OSCAR_RAW_DEBUG ) << user << " is now online";
	emit userIsOnline( user, ud );
}

void OnlineNotifierTask::userOffline()
{
	Buffer* buffer = transfer()->buffer();
	UserDetails ud;
	ud.fill( buffer );
	QString user = ud.userId();
	//kDebug( OSCAR_RAW_DEBUG ) << user << " is now offline";
	emit userIsOffline( user, ud );
}

#include "onlinenotifiertask.moc"
//kate: tab-width 4; indent-mode csands;
