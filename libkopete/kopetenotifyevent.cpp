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

KopeteNotifyEvent::KopeteNotifyEvent( const bool suppressCommon )
{
	m_suppressCommon = suppressCommon;
	m_message = 0;
	m_chat = 0;
	m_sound = 0;
}

KopeteNotifyEvent::~KopeteNotifyEvent()
{
	delete m_sound;
	delete m_message;
	delete m_chat;
}

bool KopeteNotifyEvent::suppressCommon() const
{
	return m_suppressCommon;
}

KopeteEventPresentation *KopeteNotifyEvent::presentation( const KopeteEventPresentation::PresentationType type ) const
{
	switch ( type )
	{
	case KopeteEventPresentation::Sound:
		return m_sound;
	case KopeteEventPresentation::Message:
		return m_message;
	case KopeteEventPresentation::Chat:
		return m_chat;
	default:
		return 0;
	}
}

void KopeteNotifyEvent::removePresentation( const KopeteEventPresentation::PresentationType type )
{
	KopeteEventPresentation **presToChange;
	switch ( type )
	{
	case KopeteEventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case KopeteEventPresentation::Message:
		presToChange = &m_message;
		break;
	case KopeteEventPresentation::Chat:
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

void KopeteNotifyEvent::setPresentation( const KopeteEventPresentation::PresentationType type, KopeteEventPresentation * notification )
{
	KopeteEventPresentation **presToChange;
	switch ( type )
	{
	case KopeteEventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case KopeteEventPresentation::Message:
		presToChange = &m_message;
		break;
	case KopeteEventPresentation::Chat:
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

bool KopeteNotifyEvent::firePresentation( const KopeteEventPresentation::PresentationType type )
{
	kdDebug( 14010 ) << k_funcinfo << endl;
	KopeteEventPresentation **presToChange;
	switch ( type )
	{
	case KopeteEventPresentation::Sound:
		presToChange = &m_sound;
		break;
	case KopeteEventPresentation::Message:
		presToChange = &m_message;
		break;
	case KopeteEventPresentation::Chat:
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

void KopeteNotifyEvent::setSuppressCommon( const bool suppress )
{
	m_suppressCommon = suppress;
}

const QValueList<QDomElement> KopeteNotifyEvent::toXML() const
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

QString KopeteNotifyEvent::toString()
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
