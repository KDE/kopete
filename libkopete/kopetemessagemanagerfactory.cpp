/*
	kopetemessagemanagerfactory.cpp - Creates chat sessions

	Copyright   : (c) 2002 by Duncan Mac-Vicar Prett
	Email       : duncan@kde.org

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/

#include "kopetemessagemanagerfactory.h"
#include "kopetemessagemanagerfactory.moc"
#include "kopetemessagemanager.h"
#include "kopeteprotocol.h"

#include <kdebug.h>

KopeteMessageManagerFactory::KopeteMessageManagerFactory( QObject* parent,
	const char* name )
	: QObject( parent, name )
{
}

KopeteMessageManagerFactory::~KopeteMessageManagerFactory()
{
}

KopeteMessageManager *KopeteMessageManagerFactory::create(
	const KopeteContact *user, KopeteContactList _contacts, /* Touch that underscore and you die, along with ICQ not compiling. Fuck the underscore, that BLOWS CHUNKS. */
	KopeteProtocol *protocol, QString logFile , int widget, int capabilities )
{
	bool createNewSession = false;
	KopeteMessageManager *tmp;
	KopeteMessageManagerList this_protocol_sessions;

	/* We build the sessions list for this protocol */
	for ( tmp = mSessionList.first(); tmp ; tmp = mSessionList.next() )
	{
		if ( tmp->protocol() == protocol )
		{
			this_protocol_sessions.append(tmp);
		}
	}

	for ( tmp = this_protocol_sessions.first(); tmp ; tmp = this_protocol_sessions.next() )
	{
    	/* This way we support profiles for each protocol */
		if ( user == tmp->user() )
		{
			kdDebug() << "[KopeteMessageManagerFactory] User match, looking session members" << endl;	
			KopeteContactList contactlist = tmp->members();
			KopeteContact *tmp_contact = contactlist.first();
			for( ; tmp_contact; tmp_contact = contactlist.next() )
			{
				if ( !_contacts.containsRef( tmp_contact ) )
				{
					kdDebug() << "[KopeteMessageManagerFactory] create() Oops, contact not found! new session needed!" << endl;	
					createNewSession = true;
					break;
				}
			}
			if ( createNewSession == false )
			{
				/* current session (tmp) is the same session the user is requesting */
				return tmp;
			}		
		}
		else
		{
			kdDebug() << "[KopeteMessageManagerFactory] User doesnt match, trying next session" << endl;	
		}
	}
	
	KopeteMessageManager *session = new KopeteMessageManager(user, _contacts, protocol, logFile, widget, capabilities);
	connect( session, SIGNAL(dying(KopeteMessageManager*)), this, SLOT(slotRemoveSession(KopeteMessageManager*)));
	(mSessionList).append(session);
	return (session);

}

void KopeteMessageManagerFactory::slotRemoveSession( KopeteMessageManager *session)
{
	mSessionList.setAutoDelete(false);
	(mSessionList).remove(session);
}

// vim: set noet ts=4 sts=4 sw=4:

