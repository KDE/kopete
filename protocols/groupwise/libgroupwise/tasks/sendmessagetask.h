/*
    Kopete Groupwise Protocol
    sendmessagetask.h - sends a message to a conference

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
