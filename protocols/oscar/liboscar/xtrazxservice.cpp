/*
   Kopete Oscar Protocol
   xtrazxservice.cpp - Xtraz XService

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

#include "xtrazxservice.h"

namespace Xtraz
{

XService::XService()
{
}

XService::~XService()
{
}

QDomElement XService::create( QDomDocument& doc, Type type ) const
{
	QDomElement e = doc.createElement( QStringLiteral("srv") );

	QDomElement eId = doc.createElement( QStringLiteral("id") );
	eId.appendChild( doc.createTextNode( serviceId() ) );
	e.appendChild( eId );

	if ( type == Request )
	{
		QDomElement eReq = doc.createElement( QStringLiteral("req") );
		createRequest( doc, eReq );
		e.appendChild( eReq );
	}
	else if ( type == Response )
	{
		QDomElement eVal = doc.createElement( QStringLiteral("val") );
		eVal.setAttribute( QStringLiteral("srv_id"), serviceId() );
		createResponse( doc, eVal );
		e.appendChild( eVal );
	}
	
	return e;
}

void XService::handle( QDomElement& eRoot )
{
	QDomNode childNode;
	for ( childNode = eRoot.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
	{
		QDomElement e = childNode.toElement();
		if( !e.isNull() )
		{
			if ( e.tagName() == QLatin1String("id") )
			{
				Q_ASSERT( e.text() == serviceId() );
			}
			else if ( e.tagName() == QLatin1String("val") )
				handleResponse( e );
			else if ( e.tagName() == QLatin1String("req") )
				handleRequest( e );
			
		}
	}
}

QString XService::serviceId() const
{
	return QString();
}

void XService::createResponse( QDomDocument& doc, QDomElement &e ) const
{
	e.appendChild( doc.createTextNode( QStringLiteral( "undefined" ) ) );
}

}
