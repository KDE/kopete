/*
    rateclass.h  -  Oscar Rate Limiting Implementation

    Copyright (c) 2004 by Tom Linsky <twl6@po.cwru.edu>
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

#ifndef RATECLASS_H
#define RATECLASS_H

#include "buffer.h"
#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qdatetime.h>

struct SnacPair
{
	//just a group+type pair
	 WORD group;
	 WORD type;
};

class RateClass : public QObject
{
	Q_OBJECT
public:
	RateClass();
	
	/* Accessor for classid */
	WORD id(void) { return classid; };
	
	/* Sets rate information */
	void setRateInfo( WORD pclassid, DWORD pwindowsize, DWORD pclear,
		DWORD palert, DWORD plimit, DWORD pdisconnect, DWORD pcurrent,
		DWORD pmax, DWORD plastTime, BYTE pcurrentState );
		
	/* Adds rate class members */
	void addMember( SnacPair *pmember );
	
	/* Tells whether the passed snac is a member of this rate class */
	bool isMember( const SNAC &s );

	/* Add a packet to the queue */
	void enqueue(Buffer &);
	
	/* Takes a packet off the front of the queue */
	void dequeue(void);

private:
	//rate info
	WORD classid;
	DWORD windowsize;
	DWORD clear;
	DWORD alert;
	DWORD limit;
	DWORD disconnect;
	DWORD current;
	DWORD max;
	DWORD lastTime;
	BYTE currentState;
	QPtrList<SnacPair> members;
	QValueList<Buffer> mPacketQueue;
	QTime mPacketTimer;
	
signals:
	//Tells OscarSocket that a packet is ready to be sent
	void dataReady(Buffer &);
	
private slots:
	//Sends the next packet in the queue
	void timedSend(void);
};

#endif
