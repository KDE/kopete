/*
   Kopete Oscar Protocol
   rateinfotask.cpp - Fetch the rate class information

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

#include "rateinfotask.h"

#include <QList>
#include <kdebug.h>
#include "rateclass.h"
#include "rateclassmanager.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "transfer.h"
#include "connection.h"

using namespace Oscar;

RateInfoTask::RateInfoTask( Task* parent )
		: Task( parent )
{
	connect( this, SIGNAL(gotRateLimits()), this, SLOT(sendRateInfoAck()) );
}


RateInfoTask::~RateInfoTask()
{
	
}


bool RateInfoTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( st && st->snacService() == 1 && st->snacSubtype() == 7 )
		return true;
	else
		return false;
}

bool RateInfoTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		handleRateInfoResponse();
		setTransfer( 0 );
		return true;
	}
	return false;
}

void RateInfoTask::onGo()
{
	sendRateInfoRequest();
}

void RateInfoTask::sendRateInfoRequest()
{
	kDebug(OSCAR_RAW_DEBUG) << "sending rate info request (SNAC 0x01, 0x06)";
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0006, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer* st = createTransfer( f, s, buffer );
	send( st );
}

QList<RateClass*> RateInfoTask::parseRateClasses(Buffer *buffer)
{
	QList<RateClass*> rates;
	Oscar::RateInfo ri;
	
	kDebug(OSCAR_RAW_DEBUG) << "handling rate info response (SNAC 0x01, 0x07)";
	
	int numClasses = buffer->getWord();
	kDebug(OSCAR_RAW_DEBUG) << "Got " << numClasses << " rate classes";
	for ( int i = 0; i < numClasses; i++ )
	{
		RateClass* newClass = new RateClass();
		//parse rate classes and put them somewhere
		ri.classId = buffer->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "Rate class: " << ri.classId;
		//discard the rest (for right now)
		ri.windowSize = buffer->getDWord(); //window size
		ri.clearLevel = buffer->getDWord(); //clear level
		ri.alertLevel = buffer->getDWord(); //alert level
		ri.limitLevel = buffer->getDWord(); //limit level
		ri.disconnectLevel = buffer->getDWord(); //disconnect level
		ri.currentLevel = buffer->getDWord(); //current level
		ri.initialLevel = ri.currentLevel;
		ri.maxLevel = buffer->getDWord(); //max level
		ri.lastTime = buffer->getDWord(); //last time
		ri.currentState = buffer->getByte(); //current state
		
		newClass->setRateInfo( ri );
		rates.append( newClass );
	}

	int groupNum = 0;
	int numGroupPairs = 0;
	
	for ( int i = 0; i < numClasses; i++ )
	{
		groupNum = buffer->getWord();
		kDebug(OSCAR_RAW_DEBUG) << "Adding snac members to group " << groupNum;
		
		RateClass* rc = 0L;
		QList<RateClass*>::ConstIterator it = rates.constBegin();
		for ( ; it != rates.constEnd(); ++it )
		{
			if ( ( *it )->id() == groupNum )
			{
				rc = ( *it );
				break;
			}
		}
			
//		m_rateGroups.append( groupNum );
		numGroupPairs = buffer->getWord();
		for ( int j = 0; j < numGroupPairs; j++ )
		{
			Oscar::WORD family = buffer->getWord();
			Oscar::WORD subtype = buffer->getWord();
			rc->addMember( family, subtype );
		}
	}

	return rates;
}

void RateInfoTask::handleRateInfoResponse()
{
	Buffer* buffer = transfer()->buffer();
	QList<RateClass*> rates = parseRateClasses(buffer);

	QList<RateClass*>::ConstIterator it = rates.constBegin();
	QList<RateClass*>::ConstIterator rcEnd = rates.constEnd();
	for ( ; it != rcEnd; ++it )
		client()->rateManager()->registerClass( ( *it ) );
	
	emit gotRateLimits();
}

void RateInfoTask::sendRateInfoAck()
{
	kDebug(OSCAR_RAW_DEBUG) << "sending rate info acknowledgement";
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0008, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();

	QList<int>::const_iterator cit = m_rateGroups.constBegin();
	QList<int>::const_iterator end = m_rateGroups.constEnd();
	for ( cit = m_rateGroups.constBegin(); cit != end; ++cit )
	{
		//kDebug(OSCAR_RAW_DEBUG) << "Adding rate " << (*cit) << " to rate ack";
		buffer->addWord( (*cit) );
	}

	Transfer* st = createTransfer( f, s, buffer );
	send( st );
	setSuccess( 0, QString() );
}

#include "rateinfotask.moc"

//kate: tab-width 4; indent-mode csands;

