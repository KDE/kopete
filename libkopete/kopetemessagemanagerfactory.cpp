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

#include "kopetecontact.h"

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
	const KopeteContact *user, KopeteContactPtrList _contacts, /* Touch that underscore and you die, along with ICQ not compiling. Fuck the underscore, that BLOWS CHUNKS. */
	KopeteProtocol *protocol, QString logFile, enum KopeteMessageManager::WidgetType widget)
{
	/* We build the sessions list for this protocol */
	KopeteMessageManager *tmpKmm;
	KopeteMessageManagerList this_protocol_sessions;
	for ( tmpKmm = mSessionList.first(); tmpKmm ; tmpKmm = mSessionList.next() )
	{
		if ( tmpKmm->protocol() == protocol )
		{
			this_protocol_sessions.append(tmpKmm);
		}
	}

	// Point this to the right KMM, if found
	KopeteMessageManager* result = 0;

	for ( KopeteMessageManager* kmm = this_protocol_sessions.first(); kmm && !result ; kmm = this_protocol_sessions.next() ) {
		if ( user == kmm->user() && widget == kmm->widget()) {

			kdDebug() << "[KopeteMessageManagerFactory] User match, looking session members" << endl;
			QPtrList<KopeteContact> contactlist = kmm->members();

			// set this to false if _contacts doesn't contain current kmm's contactlist
			bool halfMatch = true;

			KopeteContact *tmp_contact;
			for (tmp_contact = contactlist.first(); tmp_contact && halfMatch; tmp_contact = contactlist.next()) {
				if ( !_contacts.containsRef( tmp_contact ) )
				{
					kdDebug() << "[KopeteMessageManagerFactory] create() Oops, contact \"" << /* THIS CAUSES CRASHES, DONT ENABLE tmp_contact->name() << */ "\" not found! in _contacts" << endl;
					halfMatch = false;
				}
			}

			// If _contacts contains current kmm's contactlist, try the other way around
			if (halfMatch) {

				bool fullMatch = true;
				for (tmp_contact = _contacts.first(); tmp_contact && fullMatch; tmp_contact = _contacts.next()) {
					if ( !contactlist.containsRef( tmp_contact ) )
					{
						kdDebug() << "[KopeteMessageManagerFactory] create() Oops, contact \"" << tmp_contact->name() << "\" not found! in contactlist" << endl;
						fullMatch = false;
					}
				}

				// We have a winner
				if (fullMatch) {
					result = kmm;
				}

			}

		} else {
			kdDebug() << "[KopeteMessageManagerFactory] User doesn't match, trying next session" << endl;
		}

	}

	if (0 == result) {
		result = new KopeteMessageManager(user,  _contacts, protocol, logFile, widget);
		mSessionList.append(result);

		/*
		 * There's no need for a slot here... just add a public remove()
		 * method and call from KMM's destructor
		 */
		connect( result, SIGNAL(dying(KopeteMessageManager*)), this, SLOT(slotRemoveSession(KopeteMessageManager*)));
	}
	return (result);
}

void KopeteMessageManagerFactory::slotRemoveSession( KopeteMessageManager *session)
{
	mSessionList.setAutoDelete(false);
	(mSessionList).remove(session);
}

KopeteMessageManagerList KopeteMessageManagerFactory::protocolSessions( KopeteProtocol *protocol)
{
	KopeteMessageManager *tmpKmm;
	KopeteMessageManagerList this_protocol_sessions;
	for ( tmpKmm = mSessionList.first(); tmpKmm ; tmpKmm = mSessionList.next() )
	{
		if ( tmpKmm->protocol() == protocol )
		{
			this_protocol_sessions.append(tmpKmm);
		}
	}
	return this_protocol_sessions;
}

void KopeteMessageManagerFactory::cleanSessions( KopeteProtocol *protocol)
{
	KopeteMessageManager *tmpKmm;
	KopeteMessageManagerList protocol_sessions = protocolSessions( protocol );
	for ( tmpKmm = protocol_sessions.first(); tmpKmm ; tmpKmm = protocol_sessions.next() )
	{
			slotRemoveSession(tmpKmm);
	}
}


// vim: set noet ts=4 sts=4 sw=4:

