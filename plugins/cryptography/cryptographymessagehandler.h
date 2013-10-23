/*
    cryptographymessagehandler.h - Kopete Message Filtering

    This file is borrowed from privacymessagefilter.h in the Kopete Cryptography plugin

    Copyright (c) 2006 by Andre Duffeck <andre@duffeck.de>
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CRYPTOGRAPHY_MESSAGEHANDLER_H
#define CRYPTOGRAPHY_MESSAGEHANDLER_H

#include "kopetemessagehandler.h"
#include "cryptography_export.h"
using namespace Kopete;

class KOPETECRYPTOGRAPHY_EXPORT CryptographyMessageHandlerFactory : public MessageHandlerFactory
{
public:

	CryptographyMessageHandlerFactory( Message::MessageDirection direction, int position,
	                             QObject *target, const char *slot );
	~CryptographyMessageHandlerFactory();
	
	MessageHandler *create( ChatSession *manager, Message::MessageDirection direction );
	int filterPosition( ChatSession *manager, Message::MessageDirection direction );
	
private:
	class Private;
	Private *d;
};

class CryptographyMessageHandler : public MessageHandler
{
	Q_OBJECT
public:
	CryptographyMessageHandler();
	~CryptographyMessageHandler();
	
	void handleMessage( MessageEvent *event );
	
Q_SIGNALS:
	void handle( Kopete::MessageEvent *event );

private:
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
