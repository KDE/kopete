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

RequestFactory::RequestFactory()
: m_nextTransaction( 0 )
{
}

RequestFactory::~RequestFactory()
{
}

Request* RequestFactory::request( QCString &command )
{
	return new Request( m_nextTransaction++, command );
}


