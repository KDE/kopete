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
#include <QByteArray>
#include "oscarmessage.h"
#include "oscartypeclasses.h"

class QTextCodec;

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
	void chatroomMessage( const Oscar::Message&, const QByteArray & );
	void fileMessage( int, const QString, const QByteArray, Buffer );
	
private:
	
	//!Handles messages from channel 1 (type 1 messages)
	void handleType1Message();
	
	//!Handles messages from channel 2 (type 2 messages)
	void handleType2Message();
	
	//!Handles messages from channel 4 (type 4 messages)
	void handleType4Message();

	//!Handles client auto responses (SNAC 0x04/0x0B)
	void handleAutoResponse();

	//!Parses Rendezvous data in Buffer and puts the information into Message
	void parseRendezvousData( Buffer* b, Oscar::Message* msg );
	
	QTextCodec* guessCodec( const QByteArray& string );

private:
	
	QByteArray m_icbmCookie;
	int m_channel;
	QString m_fromUser;
	int m_currentSnacSubtype;
	int m_charSet;
	int m_subCharSet;
	
};

#endif

//kate: indent-mode csands; tab-width 4;
