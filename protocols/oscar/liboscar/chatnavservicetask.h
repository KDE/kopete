/*
    Kopete Oscar Protocol - Chat Navigation service handlers
    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATNAVSERVICETASK_H
#define CHATNAVSERVICETASK_H

#include "task.h"

#include <qvaluelist.h>
#include <oscartypes.h>

class Transfer;

/**
 * @author Matt Rogers
 */
class ChatNavServiceTask : public Task
{
Q_OBJECT
public:
	ChatNavServiceTask( Task* parent );
	~ChatNavServiceTask();

	enum RequestType { Limits = 0x0002, Exchange, Room, ExtRoom, Members,
	                   Search, Create };

	void setRequestType( RequestType );
	RequestType requestType();

	virtual bool forMe( const Transfer* transfer ) const;
	virtual bool take( Transfer* transfer );
	virtual void onGo();
    void createRoom( WORD exchange, const QString& name ); //create a room. sends the packet as well

    QValueList<int> exchangeList() const;

signals:
    void haveChatExchanges( const QValueList<int>& );
    void connectChat( WORD, QByteArray, WORD, const QString& );

private:
	void handleExchangeInfo( const TLV& t );
	void handleBasicRoomInfo( const TLV& t );
    void handleCreateRoomInfo( const TLV& t );

private:
    QValueList<int> m_exchanges;
	RequestType m_type;
};

#endif

//kate: indent-mode csands; tab-width 4;

