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
#include <QList>

namespace Kopete {

class ChatSessionManager::Private
{
  public:
	QList <ChatSession*> sessions;
//	UI::ChatView *activeView;
};

ChatSessionManager* ChatSessionManager::s_self = 0L;

ChatSessionManager* ChatSessionManager::self()
{
	if( !s_self )
		s_self = new ChatSessionManager( kapp );

	return s_self;
}

ChatSessionManager::ChatSessionManager( QObject* parent )
	: QObject( parent ), d(new Private())
{
	s_self = this;
}

ChatSessionManager::~ChatSessionManager()
{
	s_self = 0L;
	QList<ChatSession*>::ConstIterator it;
	for ( it=d->sessions.constBegin() ; it!=d->sessions.constEnd() ; ++it )
	{
		kDebug( 14010 ) << "Unloading KMM: Why this KMM isn't yet unloaded?";
		(*it)->deleteLater();
	}
	delete d;
}

ChatSession* ChatSessionManager::findChatSession(const Contact *user,
		ContactPtrList chatContacts, Protocol *protocol)
{
	ChatSession *result = 0L;
	QList<ChatSession*>::ConstIterator it;
	int i;

	for ( it= d->sessions.constBegin(); it!=d->sessions.constEnd() && !result ; ++it  )
	{
	  ChatSession* cs=(*it);
	  if ( cs->protocol() == protocol && user == cs->myself() )
		{
			QList<Contact*> contactlist = cs->members();

			// set this to false if chatContacts doesn't contain current cs's contact list
			bool halfMatch = true;

			for ( i = 0; i != contactlist.size() && halfMatch; i++ )
			{
				if ( !chatContacts.contains( contactlist[i] ) )
					halfMatch = false;
			}

			// If chatContacts contains current cs's contactlist, try the other way around
			if (halfMatch)
			{
				bool fullMatch = true;
				for ( i = 0; i != chatContacts.size() && fullMatch; i++ )
				{
					if ( !contactlist.contains( chatContacts[i] ) )
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
	const Contact *user, ContactPtrList chatContacts, Protocol *protocol, Kopete::ChatSession::Form form )
{
	ChatSession *result=findChatSession( user,  chatContacts, protocol);
	if (!result)
	{
		result = new ChatSession(user,  chatContacts, protocol, form );
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
	connect( result, SIGNAL(messageAppended(Kopete::Message&,Kopete::ChatSession*)),
		SIGNAL(aboutToDisplay(Kopete::Message&)) );
	connect( result, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
		SIGNAL(aboutToSend(Kopete::Message&)) );
	connect( result, SIGNAL(messageReceived(Kopete::Message&,Kopete::ChatSession*)),
		SIGNAL(aboutToReceive(Kopete::Message&)) );

	connect( result, SIGNAL(messageAppended(Kopete::Message&,Kopete::ChatSession*)),
		SIGNAL(display(Kopete::Message&,Kopete::ChatSession*)) );

	emit chatSessionCreated(result);
}


void ChatSessionManager::removeSession( ChatSession *session)
{
	kDebug(14010) ;
	d->sessions.removeAll( session );
}

QList<ChatSession*> ChatSessionManager::sessions( )
{
	return d->sessions;
}

KopeteView * ChatSessionManager::createView( ChatSession *kmm , const QString &requestedPlugin )
{
	KopeteView *newView = KopeteViewManager::viewManager()->view(kmm,requestedPlugin);
	if(!newView)
	{
		kDebug(14010) << "View not successfuly created";
		return 0L;
	}

	QObject *viewObject = dynamic_cast<QObject *>(newView);
	if(viewObject)
	{
		connect(viewObject, SIGNAL(activated(KopeteView*)),
			this, SIGNAL(viewActivated(KopeteView*)));
		connect(viewObject, SIGNAL(closing(KopeteView*)),
			this, SIGNAL(viewClosing(KopeteView*)));
	}
	else
	{
		kWarning(14010) << "Failed to cast view to QObject *";
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

