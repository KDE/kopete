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

#include <kapplication.h>
#include <kdebug.h>


#include "kopetecontact.h"
#include "kopetechatsession.h"

namespace Kopete
{

class ChatSessionManager::Private
{
	public:
		QValueList <ChatSession*> sessions;
		UI::ChatView *activeView;
};

ChatSessionManager* ChatSessionManager::s_self = 0L;

ChatSessionManager* ChatSessionManager::self()
{
	if( !s_self )
		s_self = new ChatSessionManager( kapp , "ChatSessionManager" );

	return s_self;
}

ChatSessionManager::ChatSessionManager( QObject* parent, const char* name )
	: QObject( parent, name )
{
	d=new Private();
	d->activeView=0L;
}

ChatSessionManager::~ChatSessionManager()
{
	s_self = 0L;
	QValueList<ChatSession*>::Iterator it;
	for ( it=d->sessions.begin(); it!=d->sessions.end() ; ++it )
	{
		kdDebug( 14010 ) << k_funcinfo << "Unloading KMM: Why this KMM isn't yet unloaded?" << endl;
		(*it)->deleteLater();
	}
	delete d;
}

ChatSession* ChatSessionManager::findChatSession(const Contact *myself, QValueList<Contact*> chatContacts) const
{
	QValueList<ChatSession*>::Iterator it;
	for ( it=d->sessions.begin(); it!=d->sessions.end() ; ++it )
	{
		ChatSession *kmm=*it;
		if ( kmm->myself() == myself )
		{
			QValueList<Contact*> contactlist = kmm->members();

			// set this to false if chatContacts doesn't contain current kmm's contactlist
			Contact *tmpContact;
			bool match=true;
			QValueList<Contact*>::Iterator it2;
			for ( it2=contactlist.begin(); it2!=contactlist.end() ; ++it2 )
			{
				if ( !chatContacts.contains( *it2 ) )
				{
					match=false;
					break;
				}
			}
			if(match)
			{
				match=true;
				// If chatContacts contains current kmm's contactlist, try the other way around
				for ( it2=chatContacts.begin(); it2!=chatContacts.end() ; ++it2 )
				{
					if ( !contactlist.contains( *it2 ) )
					{
						match=false;
						break;
					}
				}
				if(match)
					return kmm;
			}
		}
	}
	return 0L;
}

ChatSession *ChatSessionManager::create(Contact *myself,  QValueList<Contact*> chatContacts)
{
	ChatSession *result=findChatSession( myself,  chatContacts);
	if (!result)
	{
		result = new ChatSession(myself,  chatContacts );
		registerChatSession(result);
	}
	return (result);
}

void ChatSessionManager::registerChatSession(ChatSession * result)
{
	d->sessions.append( result );

	connect( result, SIGNAL( chatSessionDestroyed( ChatSession* ) ) , this , SLOT(removeSession(ChatSession* )));

#if 0 //TODO
	connect( result, SIGNAL( messageAppended( KopeteMessage &, MessageManager * ) ),
		SIGNAL( aboutToDisplay( KopeteMessage & ) ) );
	connect( result, SIGNAL( messageSent( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToSend(KopeteMessage & ) ) );
	connect( result, SIGNAL( messageReceived( KopeteMessage &, KopeteMessageManager * ) ),
		SIGNAL( aboutToReceive(KopeteMessage & ) ) );

	connect( result, SIGNAL(messageAppended( KopeteMessage &, KopeteMessageManager *) ),
		SIGNAL( display( KopeteMessage &, KopeteMessageManager *) ) );
#endif

	emit chatSessionCreated(result);
}


void ChatSessionManager::removeSession( ChatSession *session)
{
	kdDebug(14010) << k_funcinfo << endl;
	d->sessions.remove( session) ;
}

QValueList<ChatSession*> ChatSessionManager::sessions( ) const
{
	return d->sessions;
}


#if 0 //TODO
UI::ChatView* ChatSessionManager::createView( ChatSession *kmm , Message::MessageType type )
{

	View *newView=0L;
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


void ChatSessionManager::postNewEvent(KopeteEvent *e)
{
	emit newEvent(e);
}
#endif

UI::ChatView *ChatSessionManager::activeView()
{
	return d->activeView;
}

void ChatSessionManager::setActiveView(UI::ChatView *v)
{
	d->activeView=v;
}

} //END namespace Kopete

#include "kopetechatsessionmanager.moc"



