/*
  rateclass.cpp  -  Rate Limiting Implementation

    Copyright (c) 2004 by Tom Linsky <thomas.linsky@cwru.edu>
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
#include <qtimer.h>
#include <kdebug.h>

RateClass::RateClass()
{
	mPacketTimer.start();
}

void RateClass::setRateInfo( WORD pclassid, DWORD pwindowsize, DWORD pclear,
	DWORD palert, DWORD plimit, DWORD pdisconnect, DWORD pcurrent,
	DWORD pmax, DWORD plastTime, BYTE pcurrentState )
{
	classid = pclassid;
	windowsize = pwindowsize;
	clear = pclear;
	alert = palert;
	limit = plimit;
	disconnect = pdisconnect;
	current = pcurrent;
	max = pmax;
	lastTime = plastTime;
	currentState = pcurrentState;
}
	
void RateClass::addMember( SnacPair *pmember )
{
	members.append(pmember);
}

void RateClass::dequeue(void)
{
	//clear the buffer
	mPacketQueue.first().clear();
	
	//then remove it
	mPacketQueue.pop_front();
}

void RateClass::enqueue(Buffer &outbuf)
{
	mPacketQueue.push_back(outbuf);
	timedSend();
}

void RateClass::timedSend(void)
{
	if ( mPacketQueue.empty() )
	{
		kdDebug() << k_funcinfo << "Calling writeData on empty queue!" << endl;
	}
	Buffer &outbuf = mPacketQueue.first();
	
	DWORD newLevel = 0;
	
	//get time elapsed since the last packet was sent
	int timeDiff = mPacketTimer.elapsed();
	
	//Calculate new rate level
	//NewLevel = ((Window - 1)*OldLevel + TimeDiff)/Window
	//add 1 because we never want to round down or there will be problems
	newLevel = ((windowsize - 1)*current + timeDiff)/windowsize + 1;
	
	//The maximum level at which we can safely send a packet
	DWORD maxPacket = limit*windowsize/(windowsize-1) + 25;
	
	//kdDebug(14150) << k_funcinfo << "Rate Information:"
	//	<< "\nWindow Size: " << windowsize
	//	<< "\nWindow Size - 1: " << windowsize - 1
	//	<< "\nOld Level: " << current 
	//	<< "\nAlert Level: " << alert 
	//	<< "\nLimit Level: " << limit
	//	<< "\nDisconnect Level: " << disconnect
	//	<< "\nNew Level: " << newLevel
	//	<< "\nTime elapsed: " << timeDiff 
	//	<< "\nMax packet: " << maxPacket
	//	<< "\nSend queue length: " << mPacketQueue.count() << endl;
		
	//If we are one packet or less away from being blocked, 
	// wait to send
	if ( newLevel < maxPacket || newLevel < disconnect )
	{
		//We are sending TOO FAST, let's wait and send later
		kdDebug(14150) << k_funcinfo << "Sending this packet would violate rate limit... waiting " 
			<< maxPacket*windowsize - (windowsize - 1)*current - timeDiff + 25
			<< " ms..." << endl;
			
		//the 25 is included here to allow for differences in clock
		//granularity between client and server
		QTimer::singleShot( 
			maxPacket*windowsize - (windowsize - 1)*current - timeDiff + 25,
			this,
			SLOT(timedSend()) );
			
		return;
	}
	emit dataReady(outbuf);
	
	//Update rate info
	if ( newLevel > max )
		current = max;
	else
		current = newLevel; 
		
	mPacketTimer.restart();
}

bool RateClass::isMember( const SNAC &s )
{
	for ( SnacPair *sp=members.first(); sp; sp = members.next() )
	{
		if ( sp->group == s.family && sp->type == s.subtype )
		{
			return true;
		}
	}
	return false;
}

#include "rateclass.moc"
