//
// C++ Interface: sendmessagetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SENDMESSAGETASK_H
#define SENDMESSAGETASK_H

#include "client.h"
#include "requesttask.h"

/**
Sends messages to a particular conference on the server

@author SUSE AG
*/
class SendMessageTask : public RequestTask
{
public:
	SendMessageTask(Task* parent);
	~SendMessageTask();
	
	void message( const QStringList & recipientDNList, const OutgoingMessage & msg );
};

#endif
