/*
    messagereceivertask.h  - Incoming OSCAR Messaging Handler

    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MESSAGERECEIVERTASK_H
#define MESSAGERECEIVERTASK_H

#include "task.h"
#include <qstring.h>
#include "oscartypeclasses.h"

/**
 * Handles receiving messages. 
 * @author Matt Rogers
*/
class MessageReceiverTask : public Task
{
Q_OBJECT
public:
	MessageReceiverTask( Task* parent );
	
	~MessageReceiverTask();
	
	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	
signals:
	
	void receivedMessage( const Oscar::Message& );
	
private:
	
	//!Handles messages from channel 1 (type 1 messages)
	void handleType1Message();
	
	//!Handles messages from channel 2 (type 2 messages)
	void handleType2Message() {};
	
	//!Handles messages from channel 4 (type 4 messages)
	void handleType4Message() {};
	
private:
	
	QByteArray m_icbmCookie;
	int m_channel;
	QString m_fromUser;
	QString m_messageText;
	int m_charSet;
	int m_subCharSet;
	
};

/**
 * Handles sending and receiving mini typing notifications
 * @author Matt Rogers
 */
class TypingNotifyTask : public Task
{
Q_OBJECT
public:
	enum { Finished = 0x0000, Typed = 0x0001, Begin = 0x0002 };
	
	TypingNotifyTask( Task* parent );
	~TypingNotifyTask();
	
	virtual bool forMe( const Transfer* transfer) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();
	
	void setNotification( int notifyType );
	
signals:
	//! somebody started typing on the other end
	void typingStarted( const QString& contact );
	
	//! somebody finished typing
	void typingFinished( const QString& contact );

private:
	
	//! Parse the incoming SNAC(0x04, 0x14)
	void handleNotification();

private:
	WORD m_notificationType;
};


#endif

//kate: indent-mode csands; tab-width 4;
