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

#include "rateclassmanager.h"

#include <QList>
#include <kdebug.h>


#include "transfer.h"
#include "connection.h"
#include "rateclass.h"


class RateClassManagerPrivate
{
public:
	//! The list of rate classes owned by this manager
	QList<RateClass*> classList;
	Connection* client;
};

RateClassManager::RateClassManager( Connection* parent )
: QObject( parent )
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
	QList<RateClass*>::iterator it = d->classList.begin();
	while ( it != d->classList.end() && d->classList.count() > 0)
	{
		RateClass* rc = ( *it );
		it = d->classList.erase( it );
		delete rc;
	}
}

void RateClassManager::registerClass( RateClass* rc )
{
	QObject::connect( rc, SIGNAL(dataReady(Transfer*)), this, SLOT(transferReady(Transfer*)) );
	d->classList.append( rc );
}

bool RateClassManager::canSend( Transfer* t ) const
{
	SnacTransfer* st = dynamic_cast<SnacTransfer*>( t );

	if ( !st ) //no snac transfer, no rate limiting
	{ kDebug(OSCAR_RAW_DEBUG) << "Not sending a snac";
		return true;
	}

	RateClass* rc = findRateClass( st );
	if ( rc )
	{
		if ( rc->timeToNextSend() == 0 )
		{
			kDebug(OSCAR_RAW_DEBUG) << "rate class " << rc->id() << " said it's okay to send";
			return true;
		}
		else
		{
			kDebug(OSCAR_RAW_DEBUG) << "rate class " << rc->id() << " said it's not okay to send yet";
			return false;
		}
	}
	else // no rate class
	{
		kDebug(OSCAR_RAW_DEBUG) << "no rate class. doing no rate limiting";
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

QList<RateClass*> RateClassManager::classList() const
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
	//kDebug(OSCAR_RAW_DEBUG) << "Looking for SNAC " << s.family << ", " << s.subtype;
	RateClass* rc = 0L;
	QList<RateClass*>::const_iterator it;
	QList<RateClass*>::const_iterator rcEnd = d->classList.constEnd();

	for ( it = d->classList.constBegin(); it != rcEnd; ++it )
	{
		if ( ( *it )->isMember( s.family, s.subtype ) )
		{
			//kDebug(OSCAR_RAW_DEBUG) << "Found SNAC(" << s.family << ", " << s.subtype << ") in class";
			rc = ( *it );
			break;
		}
	}

	return rc;
}

void RateClassManager::recalcRateLevels()
{
	QList<RateClass*>::iterator it;
	QList<RateClass*>::iterator rcEnd = d->classList.end();
	for ( it = d->classList.begin(); it != rcEnd; ++it )
		( *it )->updateRateInfo();
}

int RateClassManager::timeToInitialLevel( SNAC s )
{
	QList<RateClass*>::const_iterator it;
	QList<RateClass*>::const_iterator rcEnd = d->classList.constEnd();

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
