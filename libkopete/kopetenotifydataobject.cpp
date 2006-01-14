/*
    kopetenotifydataobject.cpp - Container for notification events

    Copyright (c) 2004 by Will Stephenson     <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qdom.h>
#include <kdebug.h>
#include "kopetenotifydataobject.h"
#include "kopetenotifyevent.h"

class Kopete::NotifyDataObject::Private
{
public:
	QDict<Kopete::NotifyEvent> events;
};

Kopete::NotifyDataObject::NotifyDataObject()
{
	d = new Private();
	d->events.setAutoDelete( true );
}

Kopete::NotifyDataObject::~NotifyDataObject()
{
	delete d;
}

Kopete::NotifyEvent * Kopete::NotifyDataObject::notifyEvent( const QString &event ) const
{
	Kopete::NotifyEvent *evt = d->events.find( event );
	return evt;
}

void Kopete::NotifyDataObject::setNotifyEvent( const QString& event, Kopete::NotifyEvent *notifyEvent )
{
	d->events.replace( event, notifyEvent );
}

bool Kopete::NotifyDataObject::removeNotifyEvent( const QString &event )
{
	return d->events.remove( event );
}

QDomElement Kopete::NotifyDataObject::notifyDataToXML()
{
	QDomDocument notify;
	QDomElement notifications;
	if ( !d->events.isEmpty() )
	{
		//<custom-notifications>
		notifications = notify.createElement( QString::fromLatin1( "custom-notifications" ) );
		QDictIterator<Kopete::NotifyEvent> it( d->events );
		for ( ; it.current(); ++it )
		{
			//<event name="..." suppress-common="true|false">
			QDomElement event = notify.createElement( QString::fromLatin1( "event" ) );
			event.setAttribute( QString::fromLatin1( "name" ), it.currentKey() );
			event.setAttribute( QString::fromLatin1( "suppress-common" ), QString::fromLatin1( it.current()->suppressCommon() ? "true" : "false" ) );
			QValueList<QDomElement> presentations = it.current()->toXML();
			//<sound-notification enabled="true|false" src="..." single-shot="">
			for ( QValueList<QDomElement>::Iterator it = presentations.begin(); it != presentations.end(); ++it )
				event.appendChild( notify.importNode( *it, true ) );
			notifications.appendChild( event );
		}
	}
	return notifications;
}

bool Kopete::NotifyDataObject::notifyDataFromXML( const QDomElement& element )
{
	if ( element.tagName() == QString::fromLatin1( "custom-notifications" ) )
	{
		QDomNode field = element.firstChild();
		while( !field.isNull() )
		{
			//read an event
			QDomElement fieldElement = field.toElement();
			if ( fieldElement.tagName() == QString::fromLatin1( "event" ) )
			{
				// get its attributes
				QString name = fieldElement.attribute( QString::fromLatin1( "name" ), QString::null );
				QString suppress = fieldElement.attribute( QString::fromLatin1( "suppress-common" ), QString::null );
				Kopete::NotifyEvent *evt = new Kopete::NotifyEvent( suppress == QString::fromLatin1( "true" ) );
			
				// get its children
				QDomNode child = fieldElement.firstChild();
				while( !child.isNull() )
				{
					QDomElement childElement = child.toElement();
					if ( childElement.tagName() == QString::fromLatin1( "sound-presentation" ) )
					{
//						kdDebug(14010) << k_funcinfo << "read: sound" << endl;
						QString src = childElement.attribute( QString::fromLatin1( "src" ) );
						QString enabled = childElement.attribute( QString::fromLatin1( "enabled" ) );
						QString singleShot = childElement.attribute( QString::fromLatin1( "single-shot" ) );
						Kopete::EventPresentation *pres = new Kopete::EventPresentation( Kopete::EventPresentation::Sound, src,
								( singleShot == QString::fromLatin1( "true" ) ),
								( enabled == QString::fromLatin1( "true" ) ) );
						evt->setPresentation( Kopete::EventPresentation::Sound, pres );
// 						kdDebug(14010) << k_funcinfo << "after sound: " << evt->toString() << endl;
					}
					if ( childElement.tagName() == QString::fromLatin1( "message-presentation" ) )
					{
// 						kdDebug(14010) << k_funcinfo << "read: msg" << endl;
						QString src = childElement.attribute( QString::fromLatin1( "src" ) );
						QString enabled = childElement.attribute( QString::fromLatin1( "enabled" ) );
						QString singleShot = childElement.attribute( QString::fromLatin1( "single-shot" ) );
						Kopete::EventPresentation *pres = new Kopete::EventPresentation(  Kopete::EventPresentation::Message, src,
								( singleShot == QString::fromLatin1( "true" ) ),
								( enabled == QString::fromLatin1( "true" ) ) );
						evt->setPresentation( Kopete::EventPresentation::Message, pres );
// 						kdDebug(14010) << k_funcinfo << "after message: " << evt->toString() << endl;
					}
					if ( childElement.tagName() == QString::fromLatin1( "chat-presentation" ) )
					{
// 						kdDebug(14010) << k_funcinfo << "read: chat" << endl;
						QString enabled = childElement.attribute( QString::fromLatin1( "enabled" ) );
						QString singleShot = childElement.attribute( QString::fromLatin1( "single-shot" ) );
						Kopete::EventPresentation *pres = new Kopete::EventPresentation( Kopete::EventPresentation::Chat, QString::null,
								( singleShot == QString::fromLatin1( "true" ) ),
								( enabled == QString::fromLatin1( "true" ) ) );
						evt->setPresentation( Kopete::EventPresentation::Chat, pres );
// 						kdDebug(14010) << k_funcinfo << "after chat: " << evt->toString() << endl;
					}
					child = child.nextSibling();
				}
// 				kdDebug(14010) << k_funcinfo << "read: " << evt->toString() << endl;
				setNotifyEvent( name, evt );
			}
			field = field.nextSibling();
		}
		return true;
	}
	else
	{
		kdDebug( 14010 ) << "element wasn't custom-notifications" << endl;
		return false;
	}
}

