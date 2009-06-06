/*
   Kopete Oscar Protocol
   xtrazxawayservice.cpp - Xtraz XAwayService

   Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "xtrazxawayservice.h"

namespace Xtraz
{

XAwayService::XAwayService()
	: XService()
{
}


void XAwayService::setSenderId( const QString& uni )
{
	m_senderId = uni;
}

QString XAwayService::senderId() const
{
	return m_senderId;
}

void XAwayService::setIconIndex( int index )
{
	m_iconIndex = index + 1;
}

int XAwayService::iconIndex() const
{
	return m_iconIndex - 1;
}

void XAwayService::setDescription( const QString& description )
{
	m_description = description;
}

QString XAwayService::description() const
{
	return m_description;
}

void XAwayService::setMessage( const QString& message )
{
	m_message = message;
}

QString XAwayService::message() const
{
	return m_message;
}

QString XAwayService::serviceId() const
{
	return QString::fromUtf8( "cAwaySrv" );
}

void XAwayService::createRequest( QDomDocument& doc, QDomElement &e ) const
{
	QDomElement eId = doc.createElement( "id" );
	eId.appendChild( doc.createTextNode( "AwayStat" ) );
	e.appendChild( eId );

	QDomElement eTrans = doc.createElement( "trans" );
	eTrans.appendChild( doc.createTextNode( "1" ) );
	e.appendChild( eTrans );

	QDomElement eSenderId = doc.createElement( "senderId" );
	eSenderId.appendChild( doc.createTextNode( m_senderId ) );
	e.appendChild( eSenderId );
}


void XAwayService::createResponse( QDomDocument& doc, QDomElement &e ) const
{
	QDomElement eRoot = doc.createElement( "Root" );

	eRoot.appendChild( doc.createElement( "CASXtraSetAwayMessage" ) );

	QDomElement eSenderId = doc.createElement( "uin" );
	eSenderId.appendChild( doc.createTextNode( m_senderId ) );
	eRoot.appendChild( eSenderId );

	QDomElement eIndex = doc.createElement( "index" );
	eIndex.appendChild( doc.createTextNode( QString::number( m_iconIndex ) ) );
	eRoot.appendChild( eIndex );

	QDomElement eTitle = doc.createElement( "title" );
	eTitle.appendChild( doc.createTextNode( m_description ) );
	eRoot.appendChild( eTitle );

	QDomElement eDesc = doc.createElement( "desc" );
	eDesc.appendChild( doc.createTextNode( m_message ) );
	eRoot.appendChild( eDesc );

	e.appendChild( eRoot );
}

void XAwayService::handleRequest( QDomElement& eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if( !e.isNull() )
		{
			if ( e.tagName() == "id" )
			{
				Q_ASSERT( e.text() == "AwayStat" );
			}
// 			else if ( e.tagName() == "trans" )
			else if ( e.tagName() == "senderId" )
				m_senderId = e.text();
		}
	}
	
}
void XAwayService::handleResponse( QDomElement& eRoot )
{
	QDomElement rootElement = eRoot.firstChild().toElement();
	if ( !rootElement.isNull() && rootElement.tagName() == "Root" )
	{
		QDomNode childNode;
		for ( childNode = rootElement.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
		{
			QDomElement e = childNode.toElement();
			if( !e.isNull() )
			{
				if ( e.tagName() == "title" )
					m_description = e.text();
				else if ( e.tagName() == "desc" )
					m_message = e.text();
				else if ( e.tagName() == "index" )
					m_iconIndex = e.text().toInt();
				else if ( e.tagName() == "uin" )
					m_senderId = e.text();
			}
		}
	}
}

}
