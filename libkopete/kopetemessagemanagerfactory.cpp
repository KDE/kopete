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

#include <kapplication.h>

#include "kopetemessagemanager.h"
#include "kopeteprotocol.h"
#include "kopetecontact.h"

#include <kdebug.h>

KopeteMessageManagerFactory* KopeteMessageManagerFactory::s_factory = 0L;

KopeteMessageManagerFactory* KopeteMessageManagerFactory::factory()
{
	if( !s_factory )
		s_factory = new KopeteMessageManagerFactory( kapp );

	return s_factory;
}

KopeteMessageManagerFactory::KopeteMessageManagerFactory( QObject* parent,
	const char* name )
	: QObject( parent, name ), mId( 0 )
{
	s_factory = this;
}

KopeteMessageManagerFactory::~KopeteMessageManagerFactory()
{
	s_factory = 0L;
}

KopeteMessageManager* KopeteMessageManagerFactory::findKopeteMessageManager(const KopeteContact *user,
		KopeteContactPtrList chatContacts, KopeteProtocol *protocol)
{
	KopeteMessageManager *result = 0L;
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );

	for ( KopeteMessageManager* kmm = it.current(); kmm && !result ; ++it , kmm = it.current()  )
	{
		if ( it.current()->protocol() == protocol && user == kmm->user() )
		{
			QPtrList<KopeteContact> contactlist = kmm->members();

			// set this to false if chatContacts doesn't contain current kmm's contactlist
			bool halfMatch = true;

			KopeteContact *tmpContact;
			for (tmpContact = contactlist.first(); tmpContact && halfMatch; tmpContact = contactlist.next())
			{
				if ( !chatContacts.containsRef( tmpContact ) )
					halfMatch = false;
			}

			// If chatContacts contains current kmm's contactlist, try the other way around
			if (halfMatch)
			{
				bool fullMatch = true;
				for (tmpContact = chatContacts.first(); tmpContact && fullMatch; tmpContact = chatContacts.next())
				{
					if ( !contactlist.containsRef( tmpContact ) )
						fullMatch = false;
				}
				// We have a winner
				if (fullMatch)
					result = kmm;
			}
		} 
	}
	return result;
}

KopeteMessageManager *KopeteMessageManagerFactory::create(
	const KopeteContact *user, KopeteContactPtrList chatContacts, KopeteProtocol *protocol)
{
	KopeteMessageManager *result=findKopeteMessageManager( user,  chatContacts, protocol);
	if (!result)
	{
		result = new KopeteMessageManager(user,  chatContacts, protocol, ++mId);
		addKopeteMessageManager(result);
	}
	return (result);
}

void KopeteMessageManagerFactory::addKopeteMessageManager(KopeteMessageManager * result)
{
	if(result->mmId() == 0)
	{
		result->setMMId(++mId);
	}

	mSessionDict.insert( mId, result );

	/*
	 * There's no need for a slot here... just add a public remove()
	 * method and call from KMM's destructor
	 */
	connect( result, SIGNAL( messageReceived( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToDisplay( KopeteMessage & ) ) );
	connect( result, SIGNAL( messageSent( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToSend(KopeteMessage & ) ) );
}

KopeteMessageManager* KopeteMessageManagerFactory::findKopeteMessageManager( int id )
{
	return mSessionDict.find ( id );
}

void KopeteMessageManagerFactory::removeSession( KopeteMessageManager *session)
{
	kdDebug(14010) << k_funcinfo << endl;
	mSessionDict.setAutoDelete( false );
	mSessionDict.remove( session->mmId() );
}

KopeteMessageManagerDict KopeteMessageManagerFactory::protocolSessions( KopeteProtocol *protocol)
{
	KopeteMessageManagerDict protocolSessions;
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		if ( it.current()->protocol() == protocol )
		{
			protocolSessions.insert( it.current()->mmId(), it.current() );
		}
	}
	return protocolSessions;
}

const KopeteMessageManagerDict& KopeteMessageManagerFactory::sessions( )
{
	return mSessionDict;
}

void KopeteMessageManagerFactory::cleanSessions( KopeteProtocol *protocol )
{
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		if ( it.current()->protocol() == protocol )
		{
			kdDebug( 14010 ) << k_funcinfo << "Unloading KMM " << it.current()->user()->displayName() << endl;
			delete it.current();
		}
	}
}

#include "kopetemessagemanagerfactory.moc"

// vim: set noet ts=4 sts=4 sw=4:

