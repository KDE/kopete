/*
   sendmessagetask.h  - Outgoing OSCAR Messaging Handler

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

#ifndef SENDMESSAGETASK_H
#define SENDMESSAGETASK_H

#include "task.h"
#include <qstring.h>
#include "oscarmessage.h"
/**
@author Kopete Developers
*/
class SendMessageTask : public Task
{
public:
	SendMessageTask( Task* parent );
	~SendMessageTask();

	//! Set the message to be sent
	void setMessage( const Oscar::Message& msg );

	//! Are we sending an auto response
	void setAutoResponse( bool autoResponse );

	//! our ip address
	void setIp( Oscar::DWORD ip );

	virtual void onGo();

private:
	void addBasicTLVs( Buffer* b );
	void addChannel1Data( Buffer* b );
	void addChannel2Data( Buffer* b );
	void addChannel4Data( Buffer* b );
	void addRendezvousMessageData( Buffer* b );
	void addPluginData( Buffer* b );

	QByteArray encodeFileName( const QString &fileName, QString &encodingType ) const;

private:
	Oscar::Message m_message;
	bool m_autoResponse;
	uint m_cookieCount;
	Oscar::DWORD m_ip;
};

#endif

//kate: indent-mode csands;
