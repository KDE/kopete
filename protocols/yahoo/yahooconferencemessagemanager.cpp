 /*
    yahooconferencemessagemanager.cpp

    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
 
#include "kopetecontact.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "yahooprotocol.h"
#include "yahoocontact.h"
#include "yahooconferencemessagemanager.h"
#include "yahooconferencemessagemanager.moc"

YahooConferenceMessageManager::YahooConferenceMessageManager(const KopeteContact *user,
										   KopeteContactPtrList others,
										   const KopeteMessageManager::WidgetType type,
										   const char *name)
					: KopeteMessageManager(user, others, YahooProtocol::protocol(), 0,
										   type, YahooProtocol::protocol(), name )
{

	// let Kopete's factory know about us
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager(this);

	// this slot will forward messages to the protocol
	connect( this, SIGNAL( messageSent( const KopeteMessage&,
		KopeteMessageManager* ) ),
		this, SLOT( slotMessageSent( const KopeteMessage&,
		KopeteMessageManager* ) ) );

	// in order to use auto-deletion, we have to create a chat window
	// first or we will self-destruct ourselves right away
	newChatView();
	setCanBeDeleted(true);

}

void YahooConferenceMessageManager::slotMessageSent(const KopeteMessage &message, KopeteMessageManager *)
{
	kdDebug(14130) << "[YahooImMessageManager] slotMessageSent called: " << message.body() << endl;

}

YahooConferenceMessageManager::~YahooConferenceMessageManager()
{
}
