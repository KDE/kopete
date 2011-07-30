/*
  rateclass.cpp  -  Rate Limiting Implementation

    Copyright (c) 2004 by Tom Linsky <thomas.linsky@cwru.edu>
    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "rateclass.h"
#include <QTimer>
#include <QList>
#include <kdebug.h>
#include "transfer.h"

using namespace Oscar;

RateClass::RateClass( QObject* parent )
: QObject( parent )
{
	m_waitingToSend = false;
	m_packetTimer.start();
}

RateClass::~ RateClass()
{
	dumpQueue();
	m_members.clear();
}

Oscar::WORD RateClass::id() const
{
	return m_rateInfo.classId;
}

void RateClass::setRateInfo( RateInfo newRateInfo )
{
	m_rateInfo = newRateInfo;
}

Oscar::RateInfo RateClass::getRateInfo()
{
	return m_rateInfo;
}

void RateClass::addMember( const SNAC& s )
{
	addMember( s.family, s.subtype );
}

void RateClass::addMember( Oscar::WORD family, Oscar::WORD subtype )
{
	SnacPair snacPair;
	snacPair.family = family;
	snacPair.subtype = subtype;
	m_members.append( snacPair );
}

bool RateClass::isMember(const SNAC &s) const
{
	QList<SnacPair>::const_iterator it;
	QList<SnacPair>::const_iterator spEnd = m_members.constEnd();
	for ( it = m_members.constBegin(); it != spEnd; ++it )
	{
		if ( ( *it ).family == s.family && ( *it ).subtype == s.subtype )
			return true;
	}
	return false;
}

bool RateClass::isMember( Oscar::WORD family, Oscar::WORD subtype ) const
{

	QList<SnacPair>::const_iterator it;
	QList<SnacPair>::const_iterator spEnd = m_members.constEnd();
	for ( it = m_members.constBegin(); it != spEnd; ++it )
	{
		if ( ( *it ).family == family && ( *it ).subtype == subtype )
		{
			return true;
		}
	}
	return false;
}

void RateClass::enqueue( Transfer* t )
{
	m_packetQueue.push_back( t );
	/*kDebug(OSCAR_RAW_DEBUG) << "Send queue length is now: "
		<< m_packetQueue.count() << endl;*/
	setupTimer();
}

void RateClass::dequeue()
{
	m_packetQueue.pop_front();
}

bool RateClass::queueIsEmpty() const
{
	return m_packetQueue.isEmpty();
}

int RateClass::timeToInitialLevel()
{
	Oscar::DWORD newLevel = 0;

	//get time elapsed since the last packet was sent
	int timeDiff = m_packetTimer.elapsed();

	newLevel = calcNewLevel( timeDiff );

	if ( newLevel < m_rateInfo.initialLevel )
	{
		int waitTime = ( m_rateInfo.initialLevel * m_rateInfo.windowSize ) - ( ( m_rateInfo.windowSize - 1 ) * m_rateInfo.currentLevel );
		return waitTime;
	}

	return 0;
}

int RateClass::timeToNextSend()
{
	
	Oscar::DWORD newLevel = 0;
	
	//get time elapsed since the last packet was sent
	int timeDiff = m_packetTimer.elapsed();
	
	Oscar::DWORD windowSize = m_rateInfo.windowSize;
	newLevel = calcNewLevel( timeDiff );
		
	//The maximum level at which we can safely send a packet
	Oscar::DWORD maxPacket = m_rateInfo.alertLevel + RATE_SAFETY_TIME;
		
/*kDebug(OSCAR_RAW_DEBUG) << "Rate Information:"
		<< "\nWindow Size: " << windowSize
		<< "\nWindow Size - 1: " << windowSize - 1
		<< "\nOld Level: " << m_rateInfo.currentLevel
		<< "\nAlert Level: " << m_rateInfo.alertLevel
		<< "\nLimit Level: " << m_rateInfo.limitLevel
		<< "\nDisconnect Level: " << m_rateInfo.disconnectLevel
		<< "\nNew Level: " << newLevel
		<< "\nTime elapsed: " << timeDiff
		<< "\nMax level to send: " << maxPacket << endl; */
	
	//If we are one packet or less away from being blocked, wait to send
	if ( newLevel < maxPacket || newLevel < m_rateInfo.disconnectLevel )
	{
		int waitTime = ( windowSize * maxPacket ) - ( ( windowSize - 1 ) * m_rateInfo.currentLevel );
		kDebug(OSCAR_RAW_DEBUG) << "We're sending too fast. Will wait " << waitTime << "ms before sending";
		return waitTime;
	}
	
	return 0;
}

Oscar::DWORD RateClass::calcNewLevel( int timeDifference ) const
{
	//kDebug(OSCAR_RAW_DEBUG) << "Time since last packet: "
	//		<< timeDifference << endl;
	//Calculate new rate level
	//NewLevel = ( ( Window - 1 ) * OldLevel + TimeDiff )/Window
	//add 1 because we never want to round down or there will be problems
	uint newLevel = ( ( ( m_rateInfo.windowSize - 1 ) * m_rateInfo.currentLevel  ) + timeDifference  ) / m_rateInfo.windowSize;
	if ( newLevel > m_rateInfo.initialLevel )
		newLevel = m_rateInfo.initialLevel;
	
	return newLevel;
}

void RateClass::setupTimer()
{
	if ( !m_waitingToSend )
	{
		m_waitingToSend = true;
		
		int ttns = timeToNextSend();
		if ( ttns <= 0 )
		{
			slotSend(); //send now
		}
		else
		{
			QTimer::singleShot( ttns, this, SLOT(slotSend()) ); //or send later
		}
	}
}

void RateClass::slotSend()
{
	//kDebug(OSCAR_RAW_DEBUG) ;

	if ( m_packetQueue.isEmpty() )
		return;
	
	//send then pop off the list
	emit dataReady( m_packetQueue.first() );
	dequeue();
	
	updateRateInfo();
	
	m_waitingToSend = false;
	
	// check if we still have packets to send
	if ( !m_packetQueue.isEmpty() )
		setupTimer();
}

void RateClass::updateRateInfo()
{
	//Update rate info
	Oscar::DWORD newLevel = calcNewLevel( m_packetTimer.elapsed() );
	m_rateInfo.currentLevel = newLevel;
	//kDebug(OSCAR_RAW_DEBUG) << "Current Level = " <<  newLevel;
	
	//restart the timer
	m_packetTimer.restart();
}

void RateClass::dumpQueue()
{
	QList<Transfer*>::iterator it = m_packetQueue.begin();
	while ( it != m_packetQueue.end() && m_packetQueue.count() > 0 )
	{
		Transfer* t = ( *it );
		it = m_packetQueue.erase( it );
		delete t;
	}
}

#include "rateclass.moc"

//kate: tab-width 4; indent-mode csands;
