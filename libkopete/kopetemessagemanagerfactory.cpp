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
	: QObject( parent, name ), mId( 0 )
{
}

KopeteMessageManagerFactory::~KopeteMessageManagerFactory()
{
}

KopeteMessageManager* KopeteMessageManagerFactory::findKopeteMessageManager(const KopeteContact *user,
		KopeteContactPtrList chatContacts, KopeteProtocol *protocol,
		KopeteMessageManager::WidgetType widget)
{

	/* We build the sessions list for this protocol */
	KopeteMessageManagerDict protocolSessions;
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		if ( it.current()->protocol() == protocol )
		{
			protocolSessions.insert( it.current()->id(), it.current() );
		}
	}
	// Point this to the right KMM, if found
	KopeteMessageManager* result = 0;

	it = QIntDictIterator<KopeteMessageManager>( protocolSessions );
	for ( KopeteMessageManager* kmm = it.current(); kmm && !result ; ++it , kmm = it.current()  ) {
		if ( user == kmm->user() && widget == kmm->widgetType()) {

			kdDebug() << "[KopeteMessageManagerFactory] User match, looking session members" << endl;
			QPtrList<KopeteContact> contactlist = kmm->members();

			// set this to false if chatContacts doesn't contain current kmm's contactlist
			bool halfMatch = true;

			KopeteContact *tmpContact;
			for (tmpContact = contactlist.first(); tmpContact && halfMatch; tmpContact = contactlist.next()) {
				if ( !chatContacts.containsRef( tmpContact ) )
				{
					kdDebug() << "[KopeteMessageManagerFactory] create() Oops, contact \"" << /* THIS CAUSES CRASHES, DONT ENABLE tmpContact->displayName() << */ "\" not found! in chatContacts" << endl;
					halfMatch = false;
				}
			}

			// If chatContacts contains current kmm's contactlist, try the other way around
			if (halfMatch) {

				bool fullMatch = true;
				for (tmpContact = chatContacts.first(); tmpContact && fullMatch; tmpContact = chatContacts.next()) {
					if ( !contactlist.containsRef( tmpContact ) )
					{
						kdDebug() << "[KopeteMessageManagerFactory] create() Oops, contact \"" << tmpContact->displayName() << "\" not found! in contactlist" << endl;
						fullMatch = false;
					}
				}

				// We have a winner
				if (fullMatch) {
					kdDebug()<<"### That's not cool - found one"<<endl;
					result = kmm;
				}

			}

		} else {
			kdDebug() << "[KopeteMessageManagerFactory] User doesn't match, trying next session" << endl;
		}

	}

	return result;
}

KopeteMessageManager *KopeteMessageManagerFactory::create( const KopeteContact *user, KopeteContactPtrList chatContacts,
	KopeteProtocol *protocol, QString logFile, enum KopeteMessageManager::WidgetType widget)
{
	KopeteMessageManager *result=findKopeteMessageManager( user,  chatContacts, protocol,  widget);
	if (!result)
	{
		result = new KopeteMessageManager(user,  chatContacts, protocol, ++mId, logFile, widget);
		addKopeteMessageManager(result);
	}
	return (result);
}

void KopeteMessageManagerFactory::addKopeteMessageManager(KopeteMessageManager * result)
{
	if(result->id() == 0)
	{
		result->setID(++mId);
	}

	mSessionDict.insert( mId, result );

	/*
	 * There's no need for a slot here... just add a public remove()
	 * method and call from KMM's destructor
	 */
	connect( result, SIGNAL(dying(KopeteMessageManager*)), this, SLOT(slotRemoveSession(KopeteMessageManager*)));
	connect( result, SIGNAL(messageReceived(KopeteMessage&)), SIGNAL(messageReceived(KopeteMessage&)) );
	connect( result, SIGNAL(messageQueued(KopeteMessage&)), SIGNAL(messageQueued(KopeteMessage&)) );
}

KopeteMessageManager* KopeteMessageManagerFactory::findKopeteMessageManager( int id )
{
	return mSessionDict.find ( id );
}

void KopeteMessageManagerFactory::slotRemoveSession( KopeteMessageManager *session)
{
	mSessionDict.setAutoDelete( false );
	mSessionDict.remove( session->id() );
}

KopeteMessageManagerDict KopeteMessageManagerFactory::protocolSessions( KopeteProtocol *protocol)
{
	KopeteMessageManagerDict protocolSessions;
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		if ( it.current()->protocol() == protocol )
		{
			protocolSessions.insert( it.current()->id(), it.current() );
//			kdDebug() << "KopeteMessageManagerFactory::protocolSessions : found one" << endl;
		}
	}
	return protocolSessions;
}

void KopeteMessageManagerFactory::cleanSessions( KopeteProtocol *protocol )
{
	KopeteMessageManagerDict sessions=protocolSessions( protocol );
//	kdDebug() << "KopeteMessageManagerFactory::cleanSessions " <<sessions.count() << endl;
	QIntDictIterator<KopeteMessageManager> it( sessions );
	
	for ( ; it.current() ; ++it )
	{
//		kdDebug() << "KopeteMessageManagerFactory::cleanSessions : delete one later" << endl;
		//slotRemoveSession( it.current() );
		it.current()->deleteLater();
	}
}


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

