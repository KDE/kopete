//
// C++ Implementation: requestfactory
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "request.h"

#include "requestfactory.h"

#define GW_REQUESTFACTORY_FIRST_TID 1
RequestFactory::RequestFactory()
: m_nextTransaction( GW_REQUESTFACTORY_FIRST_TID )
{
}

RequestFactory::~RequestFactory()
{
}

Request* RequestFactory::request( QCString &command )
{
	return new Request( m_nextTransaction++, command );
}


