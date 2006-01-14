/*
    kopetenotifyevent.h - Kopete Notifications for a given event

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
#include "kopetenotifyevent.h"
#include "kopeteeventpresentation.h"

Kopete::NotifyEvent::NotifyEvent( const bool suppressCommon )
{
	m_suppressCommon = suppressCommon;
	m_message = 0;
	m_chat = 0;
	m_sound = 0;
}

Kopete::NotifyEvent::~NotifyEvent()
{
	delete m_sound;
	delete m_message;
	delete m_chat;
}

bool Kopete::NotifyEvent::suppressCommon() const
{
	return m_suppressCommon;
}

Kopete::EventPresentation *Kopete::NotifyEvent::presentation( const Kopete::EventPresentation::PresentationType type ) const
{
	switch ( type )
	{
	case Kopete::EventPresentation::Sound:
		return m_sound;
	case Kopete::EventPresentation::Message:
		return m_message;
	case Kopete::EventPresentation::Chat:
		return m_chat;
	default:
		return 0;
	}
}

void Kopete::NotifyEvent::removePresentation( const Kopete::EventPresentation::PresentationType type )
{
	Kopete::EventPresentation **presToChange;
	switch ( type )
	{
	case Kopete::EventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case Kopete::EventPresentation::Message:
		presToChange = &m_message;
		break;
	case Kopete::EventPresentation::Chat:
		presToChange = &m_chat;
		break;
	default:
		kdDebug( 14010 ) << k_funcinfo << " Someone tried to set an unrecognised type of presentation!" << endl;
		return;
	}
	if ( *presToChange )
	{
		delete *presToChange;
		*presToChange = 0;
	}
}

void Kopete::NotifyEvent::setPresentation( const Kopete::EventPresentation::PresentationType type, Kopete::EventPresentation * notification )
{
	Kopete::EventPresentation **presToChange;
	switch ( type )
	{
	case Kopete::EventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case Kopete::EventPresentation::Message:
		presToChange = &m_message;
		break;
	case Kopete::EventPresentation::Chat:
		presToChange = &m_chat;
		break;
	default:
		kdDebug( 14010 ) << k_funcinfo << " Someone tried to set an unrecognised type of presentation!" << endl;
		return;
	}
	if ( *presToChange )
		delete *presToChange;
	*presToChange = notification;
}

bool Kopete::NotifyEvent::firePresentation( const Kopete::EventPresentation::PresentationType type )
{
	kdDebug( 14010 ) << k_funcinfo << endl;
	Kopete::EventPresentation **presToChange;
	switch ( type )
	{
	case Kopete::EventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case Kopete::EventPresentation::Message:
		presToChange = &m_message;
		break;
	case Kopete::EventPresentation::Chat:
		presToChange = &m_chat;
		break;
	default:
		return false;
	}
	kdDebug( 14010 ) << toString() << endl;
	if ( *presToChange && (*presToChange)->singleShot() )
	{
		kdDebug( 14010 ) << " removing singleshot!" << endl;
		delete *presToChange;
		*presToChange = 0;
		kdDebug( 14010 ) << toString() << endl;
		return true;
	}
	return false;
}

void Kopete::NotifyEvent::setSuppressCommon( const bool suppress )
{
	m_suppressCommon = suppress;
}

const QValueList<QDomElement> Kopete::NotifyEvent::toXML() const
{
	QDomDocument eventData;
	QValueList<QDomElement> eventNodes;
	if ( m_sound && !m_sound->content().isEmpty() )
	{
		QDomElement soundElmt = eventData.createElement( QString::fromLatin1( "sound-presentation" ) );
		soundElmt.setAttribute( QString::fromLatin1( "enabled" ), QString::fromLatin1( m_sound->enabled() ? "true" : "false" ) );
		soundElmt.setAttribute( QString::fromLatin1( "single-shot" ), QString::fromLatin1( m_sound->singleShot() ? "true" : "false" ) );
		soundElmt.setAttribute( QString::fromLatin1( "src" ), m_sound->content() );
		eventNodes.append( soundElmt );
	}
	if ( m_message && !m_message->content().isEmpty() )
	{
		QDomElement msgElmt = eventData.createElement( QString::fromLatin1( "message-presentation" ) );
		msgElmt.setAttribute( QString::fromLatin1( "enabled" ), QString::fromLatin1( m_message->enabled() ? "true" : "false" ) );
		msgElmt.setAttribute( QString::fromLatin1( "single-shot" ), QString::fromLatin1( m_message->singleShot() ? "true" : "false" ) );
		msgElmt.setAttribute( QString::fromLatin1( "src" ), m_message->content() );
		eventNodes.append( msgElmt );
	}
	if ( m_chat && m_chat->enabled() )
	{
		QDomElement chatElmt = eventData.createElement( QString::fromLatin1( "chat-presentation" ) );
		chatElmt.setAttribute( QString::fromLatin1( "enabled" ), QString::fromLatin1( "true" ) );
		chatElmt.setAttribute( QString::fromLatin1( "single-shot" ), QString::fromLatin1( m_chat->singleShot() ? "true" : "false" ) );
		eventNodes.append( chatElmt );
	}
	return eventNodes;
}

QString Kopete::NotifyEvent::toString()
{
	QString stringRep = QString::fromLatin1("Event; Suppress common=%1").arg( QString::fromLatin1( suppressCommon() ? "true" : "false" ) );
	if ( m_sound)
		stringRep += m_sound->toString();
	if ( m_message)
		stringRep += m_message->toString();
	if ( m_chat)
		stringRep += m_chat->toString();
	return stringRep;
}
