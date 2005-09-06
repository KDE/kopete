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

#include <qvaluelist.h>
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
	connect( this, SIGNAL( gotRateLimits() ), this, SLOT( sendRateInfoAck() ) );
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
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sending rate info request (SNAC 0x01, 0x06)" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0006, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();
	Transfer* st = createTransfer( f, s, buffer );
	send( st );
}

void RateInfoTask::handleRateInfoResponse()
{
	QValueList<RateClass*> rates;
	Oscar::RateInfo ri;
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "handling rate info response (SNAC 0x01, 0x07)" << endl;
	Buffer* buffer = transfer()->buffer();
	
	int numClasses = buffer->getWord();
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Got " << numClasses << " rate classes" << endl;
	for ( int i = 0; i < numClasses; i++ )
	{
		RateClass* newClass = new RateClass( client()->rateManager() );
		//parse rate classes and put them somewhere
		ri.classId = buffer->getWord();
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Rate class: " << ri.classId << endl;
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
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding snac members to group " << groupNum << endl;
		
		RateClass* rc = 0L;
		QValueList<RateClass*>::iterator it = rates.begin();
		for ( ; it != rates.end(); ++it )
		{
			if ( ( *it )->id() == groupNum )
			{
				rc = ( *it );
				break;
			}
		}
			
		m_rateGroups.append( groupNum );
		numGroupPairs = buffer->getWord();
		for ( int j = 0; j < numGroupPairs; j++ )
		{
			WORD family = buffer->getWord();
			WORD subtype = buffer->getWord();
			rc->addMember( family, subtype );
		}
	}

	QValueList<RateClass*>::iterator it = rates.begin();
	QValueList<RateClass*>::iterator rcEnd = rates.end();
	for ( ; it != rcEnd; ++it )
		client()->rateManager()->registerClass( ( *it ) );
	
	emit gotRateLimits();
}

void RateInfoTask::sendRateInfoAck()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sending rate info acknowledgement" << endl;
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0001, 0x0008, 0x0000, client()->snacSequence() };
	Buffer* buffer = new Buffer();

	QValueListConstIterator<int> cit = m_rateGroups.begin();
	QValueListConstIterator<int> end = m_rateGroups.end();
	for ( cit = m_rateGroups.begin(); cit != end; ++cit )
	{
		//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Adding rate " << (*cit) << " to rate ack" << endl;
		buffer->addWord( (*cit) );
	}

	Transfer* st = createTransfer( f, s, buffer );
	send( st );
	setSuccess( 0, QString::null );
}

#include "rateinfotask.moc"

//kate: tab-width 4; indent-mode csands;

