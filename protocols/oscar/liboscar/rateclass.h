/*
    rateclass.h  -  Oscar Rate Limiting Implementation

    Copyright (c) 2004 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2004 by Matt Rogers <mattr@k
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

#include "oscartypes.h"
#include <qobject.h>
#include <QList>
#include <qdatetime.h>
#include <qpair.h>
#include "liboscar_export.h"

const int RATE_SAFETY_TIME = 50;

struct SnacPair
{
	int family;
	int subtype;
};

class Transfer;

class LIBOSCAR_EXPORT RateClass : public QObject
{
	Q_OBJECT
public:
	RateClass( QObject* parent = 0 );
	~RateClass();

	/** Accessor for classid */
	Oscar::WORD id() const;

	/** Sets rate information */
	void setRateInfo( Oscar::RateInfo newRateInfo );
	
	/** Gets rate information */
	Oscar::RateInfo getRateInfo();

	/** Add a SNAC to the rate class */
	void addMember( const Oscar::SNAC& s );

	/** Adds rate class members */
	void addMember( Oscar::WORD family, Oscar::WORD subtype );

	/** Tells whether the passed snac is a member of this rate class */
	bool isMember( const Oscar::SNAC& s ) const;

	/**
	 * Tells whether the passed family and subtype combo is a member
	 * of this rate class
	 */
	bool isMember( Oscar::WORD family, Oscar::WORD subtype ) const;

	/** Add a packet to the queue */
	void enqueue( Transfer* );

	/** Takes a packet off the front of the queue */
	void dequeue();

	/** Check if the queue is empty */
	bool queueIsEmpty() const;

	/**
	 * Calulate the time until we can send again
	 * Uses the first packet on the queue to determine the time since that's
	 * the packet that will get sent.
	 * \return the time in milliseconds that we need to wait
	 */
	int timeToNextSend();
	
	/**
	 * Calulate the time until we get to initial level
	 * \return the time in milliseconds that we need to wait
	 */
	int timeToInitialLevel();
	
	/**
	 * Calculates a new rate level and updates the rate class' current level
	 * to match
	 */
	void updateRateInfo();
	
	/**
	 * Dump the current packet queue. These packets will not be sent. Used
	 * on disconnection
	 */
	void dumpQueue();

signals:

	/** Tell the rate class manager we're ready to send */
	void dataReady( Transfer* );

private:
	
	/** Calculate our new rate level */
	Oscar::DWORD calcNewLevel( int timeDifference ) const;

	/** sets up the timer for the transfer just added to the queue */
	void setupTimer();

private slots:
	/**
	 * Send the packet. Basically emits dataReady for the first transfer
	 */
	void slotSend();
	
private:

	Oscar::RateInfo m_rateInfo;
	QList<SnacPair> m_members;
	QList<Transfer*> m_packetQueue;
	QTime m_packetTimer;
	
	// we are waiting for the QTimer::singleShot() to send 
	bool m_waitingToSend;
};

#endif

//kate: tab-width 4; indent-mode csands;
