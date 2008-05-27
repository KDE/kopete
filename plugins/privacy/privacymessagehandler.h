/*
    privacymessagehandler.h - Kopete Message Filtering

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef PRIVACY_MESSAGEHANDLER_H
#define PRIVACY_MESSAGEHANDLER_H

#include "kopetemessagehandler.h"
#include <kopete_export.h>
using namespace Kopete;

class KOPETEPRIVACY_EXPORT PrivacyMessageHandlerFactory : public MessageHandlerFactory
{
public:

	PrivacyMessageHandlerFactory( Message::MessageDirection direction, int position,
	                             QObject *target, const char *slot );
	~PrivacyMessageHandlerFactory();
	
	MessageHandler *create( ChatSession *manager, Message::MessageDirection direction );
	int filterPosition( ChatSession *manager, Message::MessageDirection direction );
	
private:
	class Private;
	Private *d;
};

class PrivacyMessageHandler : public MessageHandler
{
	Q_OBJECT
public:
	PrivacyMessageHandler();
	~PrivacyMessageHandler();
	
	void handleMessage( MessageEvent *event );
	
Q_SIGNALS:
	void handle( Kopete::MessageEvent *event );

private:
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
