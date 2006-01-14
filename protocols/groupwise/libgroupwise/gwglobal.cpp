/*
    gwglobal.cpp - Kopete Groupwise Protocol
  
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwerror.h"

namespace GroupWise
{
	ConferenceGuid::ConferenceGuid() {}
	ConferenceGuid::ConferenceGuid( const QString & string ) : QString( string ) {}
	
	ConferenceGuid::~ConferenceGuid() {}
	
	bool operator==( const ConferenceGuid & g1, const ConferenceGuid & g2 )
	{
		return g1.left( CONF_GUID_END ) == g2.left( CONF_GUID_END );
	}
	bool operator==( const QString & s, const ConferenceGuid & g )
	{
		return s.left( CONF_GUID_END ) == g.left( CONF_GUID_END );
	}
	bool operator==( const ConferenceGuid & g, const QString & s )
	{
		return s.left( CONF_GUID_END ) == g.left( CONF_GUID_END );
	}
}
