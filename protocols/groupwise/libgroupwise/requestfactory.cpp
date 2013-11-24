/*
    requestfactory.cpp - Kopete Groupwise Protocol
  
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

#include "requestfactory.h"

#include "request.h"

#define GW_REQUESTFACTORY_FIRST_TID 1
RequestFactory::RequestFactory()
: m_nextTransaction( GW_REQUESTFACTORY_FIRST_TID )
{
}

RequestFactory::~RequestFactory()
{
}

Request* RequestFactory::request( const QString &command )
{
	return new Request( m_nextTransaction++, command );
}


