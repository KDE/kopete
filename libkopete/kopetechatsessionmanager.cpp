/*
    kopetechatsessionmanager.cpp - Creates chat sessions

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

#include "kopetechatsessionmanager.h"
#include "kopeteviewmanager.h"

#include <kapplication.h>
#include <kdebug.h>

#include "ui/kopeteview.h"
#include "kopetecontact.h"

namespace Kopete {

class ChatSessionManager::Private
{
  public:
	QValueList <ChatSession*> sessions;
//	UI::ChatView *activeView;
};

ChatSessionManager* ChatSessionManager::s_self = 0L;

ChatSessionManager* ChatSessionManager::self()
{
	if( !s_self )
		s_self = new ChatSessionManager( kapp );

	return s_self;
}

ChatSessionManager::ChatSessionManager( QObject* parent,
	const char* name )
	: QObject( parent, name )
{
	d=new Private;
	s_self = this;
}

ChatSessionManager::~ChatSessionManager()
{
	s_self = 0L;
	QValueListIterator<ChatSession*> it;
	for ( it=d->sessions.begin() ; it!=d->sessions.end() ; ++it )
	{
		kdDebug( 14010 ) << k_funcinfo << "Unloading KMM: Why this KMM isn't yet unloaded?" << endl;
		(*it)->deleteLater();
	}
	delete d;
}

ChatSession* ChatSessionManager::findChatSession(const Contact *user,
		ContactPtrList chatContacts, Protocol *protocol)
{
	ChatSession *result = 0L;
	QValueList<ChatSession*>::Iterator it;
	for ( it= d->sessions.begin(); it!=d->sessions.end() && !result ; ++it  )
	{
	  ChatSession* cs=(*it);
	  if ( cs->protocol() == protocol && user == cs->myself() )
		{
			QPtrList<Contact> contactlist = cs->members();

			// set this to false if chatContacts doesn't contain current cs's contactlist
			bool halfMatch = true;

			Contact *tmpContact;
			for (tmpContact = contactlist.first(); tmpContact && halfMatch; tmpContact = contactlist.next())
			{
				if ( !chatContacts.containsRef( tmpContact ) )
					halfMatch = false;
			}

			// If chatContacts contains current cs's contactlist, try the other way around
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
					result = cs;
			}
		}
	}
	return result;
}

ChatSession *ChatSessionManager::create(
	const Contact *user, ContactPtrList chatContacts, Protocol *protocol)
{
	ChatSession *result=findChatSession( user,  chatContacts, protocol);
	if (!result)
	{
		result = new ChatSession(user,  chatContacts, protocol );
		registerChatSession(result);
	}
	return (result);
}

void ChatSessionManager::slotReadMessage()
{
	emit readMessage();
}

void ChatSessionManager::registerChatSession(ChatSession * result)
{
	d->sessions.append( result );

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


void ChatSessionManager::removeSession( ChatSession *session)
{
	kdDebug(14010) << k_funcinfo << endl;
	d->sessions.remove( session );
}

QValueList<ChatSession*> ChatSessionManager::sessions( )
{
	return d->sessions;
}

KopeteView * ChatSessionManager::createView( ChatSession *kmm , const QString &requestedPlugin )
{
	KopeteView *newView = KopeteViewManager::viewManager()->view(kmm,requestedPlugin);
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

void ChatSessionManager::postNewEvent(MessageEvent *e)
{
	emit newEvent(e);
}

KopeteView *ChatSessionManager::activeView()
{
    return KopeteViewManager::viewManager()->activeView();
}

} //END namespace Kopete

#include "kopetechatsessionmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

