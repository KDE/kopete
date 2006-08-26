/*
   Kopete Oscar Protocol
   offlinemessagestask.h - Offline messages handling

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef OFFLINEMESSAGESTASK_H
#define OFFLINEMESSAGESTASK_H

#include "icqtask.h"
#include "oscarmessage.h"

/**
ICQ Offline messages handling

@author Gustavo Pichorim Boiko
*/
class OfflineMessagesTask : public ICQTask
{
Q_OBJECT
public:
	OfflineMessagesTask( Task* parent );

	~OfflineMessagesTask();
	
	virtual void onGo();
	virtual bool forMe( const Transfer* t ) const;
	virtual bool take( Transfer* t );
	
signals:
	void receivedOfflineMessage( const Oscar::Message& msg );
	
private:
	void handleOfflineMessage();
	void endOfMessages();
	void deleteOfflineMessages();
	
private:
	int m_sequence;
};

#endif
