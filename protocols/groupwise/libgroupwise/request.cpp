//
// C++ Implementation: request
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

Request::Request(int transactionId, QCString &command )
: UserTransfer(transactionId), m_command( command )
{
}

Request::~Request()
{
}

QCString Request::command()
{
	return m_command;
}


