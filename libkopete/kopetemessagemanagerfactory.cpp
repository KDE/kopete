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
#include "kopetemessagemanager.h"

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
	const KopeteContact *user, KopeteContactList &contacts,
	QString logFile )
{
	bool createNewSession = false;
	KopeteMessageManager *tmp;
	for ( tmp = mSessionList.first(); tmp ; tmp = mSessionList.next() )
	{
    	if ( user == tmp->user() )
		{
			kdDebug() << "[KopeteMessageManagerFactory] User match, looking session members" << endl;	
			KopeteContact *tmp_contact;
			KopeteContactList contactlist = tmp->contacts();
            for (  tmp_contact = contactlist.first(); tmp ; tmp_contact = contactlist.next() )
			{
				if ( !contacts.containsRef( tmp_contact ) )
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
	
	KopeteMessageManager *session = new KopeteMessageManager ( user, contacts , logFile);
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

