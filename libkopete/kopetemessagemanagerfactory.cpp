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



Kopete::ChatSessionManager* Kopete::ChatSessionManager::s_factory = 0L;

Kopete::ChatSessionManager* Kopete::ChatSessionManager::self()
{
	if( !s_factory )
		s_factory = new Kopete::ChatSessionManager( kapp );

	return s_factory;
}

Kopete::ChatSessionManager::ChatSessionManager( QObject* parent,
	const char* name )
	: QObject( parent, name ), mId( 0 )
{
	s_factory = this;
}

Kopete::ChatSessionManager::~ChatSessionManager()
{
	s_factory = 0L;
	QIntDictIterator<Kopete::ChatSession> it( mSessionDict );
	for ( ; it.current() ; ++it )
	{
		kdDebug( 14010 ) << k_funcinfo << "Unloading KMM: Why this KMM isn't yet unloaded?" << endl;
		it.current()->deleteLater();
	}
}

Kopete::ChatSession* Kopete::ChatSessionManager::findChatSession(const Kopete::Contact *user,
		Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol)
{
	Kopete::ChatSession *result = 0L;
	QIntDictIterator<Kopete::ChatSession> it( mSessionDict );

	for ( Kopete::ChatSession* kmm = it.current(); kmm && !result ; ++it , kmm = it.current()  )
	{
		if ( it.current()->protocol() == protocol && user == kmm->user() )
		{
			QPtrList<Kopete::Contact> contactlist = kmm->members();

			// set this to false if chatContacts doesn't contain current kmm's contactlist
			bool halfMatch = true;

			Kopete::Contact *tmpContact;
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

Kopete::ChatSession *Kopete::ChatSessionManager::create(
	const Kopete::Contact *user, Kopete::ContactPtrList chatContacts, Kopete::Protocol *protocol)
{
	Kopete::ChatSession *result=findChatSession( user,  chatContacts, protocol);
	if (!result)
	{
		result = new Kopete::ChatSession(user,  chatContacts, protocol, ++mId );
		addChatSession(result);
	}
	return (result);
}

void Kopete::ChatSessionManager::slotReadMessage()
{
	emit readMessage();
}

void Kopete::ChatSessionManager::addChatSession(Kopete::ChatSession * result)
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
	connect( result, SIGNAL( messageAppended( Kopete::Message &, Kopete::ChatSession * ) ),
		SIGNAL( aboutToDisplay( Kopete::Message & ) ) );
	connect( result, SIGNAL( messageSent( Kopete::Message &, Kopete::ChatSession * ) ),
		SIGNAL( aboutToSend(Kopete::Message & ) ) );
	connect( result, SIGNAL( messageReceived( Kopete::Message &, Kopete::ChatSession * ) ),
		SIGNAL( aboutToReceive(Kopete::Message & ) ) );

	connect( result, SIGNAL(messageAppended( Kopete::Message &, Kopete::ChatSession *) ),
		SIGNAL( display( Kopete::Message &, Kopete::ChatSession *) ) );

	emit chatSessionCreated(result);
}

Kopete::ChatSession* Kopete::ChatSessionManager::findChatSession( int id )
{
	return mSessionDict.find ( id );
}

void Kopete::ChatSessionManager::removeSession( Kopete::ChatSession *session)
{
	kdDebug(14010) << k_funcinfo << endl;
	mSessionDict.setAutoDelete( false );
	mSessionDict.remove( session->mmId() );
}

const Kopete::ChatSessionDict& Kopete::ChatSessionManager::sessions( )
{
	return mSessionDict;
}

KopeteView * Kopete::ChatSessionManager::createView( Kopete::ChatSession *kmm , Kopete::Message::ViewType type )
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

void Kopete::ChatSessionManager::postNewEvent(Kopete::MessageEvent *e)
{
	emit newEvent(e);
}

KopeteView *Kopete::ChatSessionManager::activeView()
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

