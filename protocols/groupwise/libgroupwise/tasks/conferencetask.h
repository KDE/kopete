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

#include "eventtask.h"

struct ConferenceEvent {
	QString guid;
	QString user;
	QTime timeStamp;
	Q_UINT32 flags;
};

enum ConferenceFlags
{
	Logged = 0x00000001,
	Secure = 0x00000002,
	Closed = 0x10000000
};

typedef QString Message;

/**
 * This Task is responsible for handling all conference related events, and signalling them up to @ref GroupWiseAccount
 * Implementation note: It would be fit the model more cleanly to have each of these in their own Task, but the amount
 * of code they share is quite large
 * @author SUSE AG
 */
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
	void invited( ConferenceEvent &, Message & );
	void otherInvited( ConferenceEvent & );
	void invitationRejected( ConferenceEvent & );
	void closed( ConferenceEvent & );
	void message( ConferenceEvent &, Message & );
	void autoReply( ConferenceEvent &, Message & );
protected:
	Q_UINT32 readFlags( QDataStream & din );
	QString readMessage( QDataStream & din );
};

#endif
