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
 * of code they share is quite large
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
	void typing( ConferenceEvent & );
	void notTyping( ConferenceEvent & );
	void joined( ConferenceEvent & );
	void left( ConferenceEvent &);
	void invited( ConferenceEvent & );
	void otherInvited( ConferenceEvent & );
	void invitationRejected( ConferenceEvent & );
	void closed( ConferenceEvent & );
	void message( const ConferenceEvent &);
	void autoReply( ConferenceEvent & );
protected:
	Q_UINT32 readFlags( QDataStream & din );
	QString readMessage( QDataStream & din );
};

#endif
