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


SnacPair::SnacPair()
{
}

SnacPair::SnacPair(const WORD pGroup, const WORD pType)
{
	mGroup = pGroup;
	mType = pType;
}

const WORD SnacPair::group()
{
	return mGroup;
}

const WORD SnacPair::type()
{
	return mType;
}

// =====================================================================


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

void RateClass::addMember(const WORD snacGroup, const WORD snacType)
{
	SnacPair *snacPair = new SnacPair(snacGroup, snacType);
	mMembers.append(snacPair);
}

void RateClass::dequeue()
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

void RateClass::timedSend()
{
	if ( mPacketQueue.empty() )
	{
		kdDebug(14151) << k_funcinfo << "Calling writeData on empty queue!" << endl;
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
	DWORD maxPacket = limit*windowsize/(windowsize-1) + RATE_SAFETY_TIME;

	/*kdDebug(14151) << k_funcinfo << "Rate Information:"
		<< "\nWindow Size: " << windowsize
		<< "\nWindow Size - 1: " << windowsize - 1
		<< "\nOld Level: " << current
		<< "\nAlert Level: " << alert
		<< "\nLimit Level: " << limit
		<< "\nDisconnect Level: " << disconnect
		<< "\nNew Level: " << newLevel
		<< "\nTime elapsed: " << timeDiff
		<< "\nMax packet: " << maxPacket
		<< "\nSend queue length: " << mPacketQueue.count() << endl;*/

	//If we are one packet or less away from being blocked,
	// wait to send
	if ( newLevel < maxPacket || newLevel < disconnect )
	{
		int waitTime = maxPacket * windowsize - (windowsize - 1) * current - timeDiff + RATE_SAFETY_TIME;
		//We are sending TOO FAST, let's wait and send later
		kdDebug(14151) << k_funcinfo <<
			"Sending this packet would violate rate limit, waiting "
			<< waitTime << " ms..." << endl;
		QTimer::singleShot(waitTime, this, SLOT(timedSend()));
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

bool RateClass::isMember(const SNAC &s)
{
	for (SnacPair *sp=mMembers.first(); sp; sp = mMembers.next())
	{
		if (sp->group() == s.family && sp->type() == s.subtype)
			return true;
	}
	return false;
}

#include "rateclass.moc"
