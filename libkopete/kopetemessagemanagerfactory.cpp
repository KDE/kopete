/*
    kopetemessagemanagerfactory.cpp - Creates chat sessions

    Copyright (c) 2002-2003 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemessagemanagerfactory.h"

#include <kapplication.h>
#include <kdebug.h>

#include "ui/kopeteview.h"
#include "kopetecontact.h"



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
	QIntDictIterator<KopeteMessageManager> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		kdDebug( 14010 ) << k_funcinfo << "Unloading KMM: Why this KMM isn't yet unloaded?" << endl;
		it.current()->deleteLater();
	}
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
		result = new KopeteMessageManager(user,  chatContacts, protocol, ++mId );
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

	mSessionDict.insert( result->mmId(), result );

	/*
	 * There's no need for a slot here... just add a public remove()
	 * method and call from KMM's destructor
	 */
	connect( result, SIGNAL( messageAppended( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToDisplay( KopeteMessage & ) ) );
	connect( result, SIGNAL( messageSent( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToSend(KopeteMessage & ) ) );
	connect( result, SIGNAL( messageReceived( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToReceive(KopeteMessage & ) ) );

	connect( result, SIGNAL(messageAppended( KopeteMessage &, KopeteMessageManager *) ),
		SIGNAL( display( KopeteMessage &, KopeteMessageManager *) ) );

	emit messageManagerCreated(result);
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

const KopeteMessageManagerDict& KopeteMessageManagerFactory::sessions( )
{
	return mSessionDict;
}

KopeteView * KopeteMessageManagerFactory::createView( KopeteMessageManager *kmm , KopeteMessage::MessageType type )
{
	KopeteView *newView=0L;
	emit requestView( newView , kmm , type  );
	if(!newView)
	{
		kdDebug(14010) << k_funcinfo << "View not successfuly created" << endl;
		return 0L;
	}

	QObject *viewObject = dynamic_cast<QObject *>(newView);
	if(viewObject)
	{
		connect(viewObject, SIGNAL(activated(KopeteView *)),
			this, SIGNAL(viewActivated(KopeteView *)));
		connect(viewObject, SIGNAL(closing(KopeteView *)),
			this, SIGNAL(viewClosing(KopeteView *)));
	}
	else
	{
		kdWarning(14010) << "Failed to cast view to QObject *" << endl;
	}

	emit viewCreated( newView ) ;
	return newView;
}

void KopeteMessageManagerFactory::postNewEvent(KopeteEvent *e)
{
	emit newEvent(e);
}

KopeteView *KopeteMessageManagerFactory::activeView()
{
	/**
	 * FIXME: This is an awful retarded way to do this. Why can't
	 we just make the KMM factory a friend of the view manager and
	 return activeView directly?
	 */

	KopeteView *v = 0L;
	emit getActiveView(v);
	return v;
}

#include "kopetemessagemanagerfactory.moc"

// vim: set noet ts=4 sts=4 sw=4:

