//
// C++ Interface: conferencetask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONFERENCETASK_H
#define CONFERENCETASK_H

#include "gwerror.h"
#include "eventtask.h"

/**
 * This Task is responsible for handling all conference related events, and signalling them up to @ref GroupWiseAccount
 * Implementation note: It would be fit the model more cleanly to have each of these in their own Task, but the amount
 * of code they share is quite large, and the differences in the way each event uses it are small
 * @author SUSE AG
 */
 
using namespace GroupWise;

class ConferenceTask : public EventTask
{
Q_OBJECT
public:
	ConferenceTask( Task* parent );
	~ConferenceTask();
	bool take( Transfer * transfer );
signals:
	void typing( const ConferenceEvent & );
	void notTyping( const ConferenceEvent & );
	void joined( const ConferenceEvent & );
	void left( const ConferenceEvent &);
	void invited( const ConferenceEvent & );
	void otherInvited( const ConferenceEvent & );
	void invitationRejected( const ConferenceEvent & );
	void closed( const ConferenceEvent & );
	void message( const ConferenceEvent &);
	void autoReply( const ConferenceEvent & );
protected slots:
	void slotReceiveUserDetails( const GroupWise::ContactDetails & );
protected:
	Q_UINT32 readFlags( QDataStream & din );
	QString readMessage( QDataStream & din );
	/**
	 * Handles an event, queuing it if necessary
	 * emits the appropriate signal based on the event type
	 */
	void handleEvent( ConferenceEvent & event );
	/**
	 * Checks to see if we need more data from the client before we can propagate this event
	 * and queues the event if so
	 * @return whether the event was queued pending more data
	 */
	bool queueWhileAwaitingData( const ConferenceEvent & event );
private:
	// A list of events which are waiting for more data from the server before they can be exposed to the client
	QValueList< ConferenceEvent > m_pendingEvents; 
};

#endif
