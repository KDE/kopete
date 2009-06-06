/*
    Kopete Groupwise Protocol
    conferencetask.h - Event Handling task responsible for all conference related events

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#ifndef CONFERENCETASK_H
#define CONFERENCETASK_H

#include "gwerror.h"
#include "eventtask.h"
#include <QtCore/QList>

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
	void invitationDeclined( const ConferenceEvent & );
	void closed( const ConferenceEvent & );
	void message( const ConferenceEvent &);
	void autoReply( const ConferenceEvent & );
	// GW7
	void broadcast( const ConferenceEvent &);
	void systemBroadcast( const ConferenceEvent &);
protected slots:
	void slotReceiveUserDetails( const GroupWise::ContactDetails & );
protected:
	quint32 readFlags( QDataStream & din );
	QString readMessage( QDataStream & din );
	/**
	 * Checks to see if we need more data from the client before we can propagate this event
	 * and queues the event if so
	 * @return whether the event was queued pending more data
	 */
	bool queueWhileAwaitingData( const ConferenceEvent & event );
	void dumpConferenceEvent( ConferenceEvent & evt );
private:
	// A list of events which are waiting for more data from the server before they can be exposed to the client
	QList< ConferenceEvent > m_pendingEvents;
};

#endif
