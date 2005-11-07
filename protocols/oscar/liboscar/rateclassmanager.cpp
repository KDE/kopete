/*
    Kopete Oscar Protocol
    rateclassmanager.cpp - Manages the rates we get from the OSCAR server

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

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

#include <qvaluelist.h>
#include <kdebug.h>


#include "rateclassmanager.h"
#include "transfer.h"
#include "connection.h"
#include "rateclass.h"


class RateClassManagerPrivate
{
public:
	//! The list of rate classes owned by this manager
	QValueList<RateClass*> classList;
	Connection* client;
};

RateClassManager::RateClassManager( Connection* parent, const char* name )
: QObject( parent, name )
{
	d = new RateClassManagerPrivate();
	d->client = parent;
}

RateClassManager::~RateClassManager()
{
	reset();
	delete d;
}

void RateClassManager::reset()
{
	QValueList<RateClass*>::iterator it = d->classList.begin();
	while ( it != d->classList.end() && d->classList.count() > 0)
	{
		RateClass* rc = ( *it );
		it = d->classList.remove( it );
		delete rc;
	}
}

void RateClassManager::registerClass( RateClass* rc )
{
	QObject::connect( rc, SIGNAL( dataReady( Transfer* ) ), this, SLOT( transferReady( Transfer* ) ) );
	d->classList.append( rc );
}

bool RateClassManager::canSend( Transfer* t ) const
{
	SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );

	if ( !st ) //no snac transfer, no rate limiting
	{ kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Not sending a snac" << endl;
		return true;
	}

	RateClass* rc = findRateClass( st );
	if ( rc )
	{ 
		if ( rc->timeToNextSend() == 0 )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "rate class " << rc->id() << " said it's okay to send" << endl;
			return true;
		}
		else
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "rate class " << rc->id() << " said it's not okay to send yet" << endl;
			return false;
		}
	}
	else // no rate class
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "no rate class. doing no rate limiting" << endl;
		return true;
	}
}

void RateClassManager::queue( Transfer* t )
{
	SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );
	if ( !st )
	{ //we're not sending a snac
		transferReady( t );
		return;
	}

	RateClass* rc = findRateClass( st );
	if ( rc )
		rc->enqueue( st );
	else
		transferReady( t );
}

QValueList<RateClass*> RateClassManager::classList() const
{
	return d->classList;
}

void RateClassManager::transferReady( Transfer* t )
{
	//tell the client to send it again. We should be
	//able to send it now
	FlapTransfer* ft = dynamic_cast<FlapTransfer*>( t );
	
	if ( ft )
		ft->setFlapSequence( d->client->flapSequence() );
	
	d->client->forcedSend( t );
}


RateClass* RateClassManager::findRateClass( SnacTransfer* st ) const
{
	SNAC s = st->snac();
	//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Looking for SNAC " << s.family << ", " << s.subtype << endl;
	RateClass* rc = 0L;
	QValueList<RateClass*>::const_iterator it;
	QValueList<RateClass*>::const_iterator rcEnd = d->classList.constEnd();

	for ( it = d->classList.constBegin(); it != rcEnd; ++it )
	{
		if ( ( *it )->isMember( s.family, s.subtype ) )
		{
			//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Found SNAC(" << s.family << ", " << s.subtype << ") in class" << endl;
			rc = ( *it );
			break;
		}
	}

	return rc;
}

void RateClassManager::recalcRateLevels()
{
	QValueList<RateClass*>::iterator it;
	QValueList<RateClass*>::iterator rcEnd = d->classList.end();
	for ( it = d->classList.begin(); it != rcEnd; ++it )
		( *it )->updateRateInfo();
}

int RateClassManager::timeToInitialLevel( SNAC s )
{
	QValueList<RateClass*>::const_iterator it;
	QValueList<RateClass*>::const_iterator rcEnd = d->classList.constEnd();
	
	for ( it = d->classList.constBegin(); it != rcEnd; ++it )
	{
		if ( ( *it )->isMember( s.family, s.subtype ) )
		{
			return ( *it )->timeToInitialLevel();
		}
	}
	return 0;
}

#include "rateclassmanager.moc"

//kate: tab-width 4; indent-mode csands;
