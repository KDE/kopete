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

Request::Request( const int transactionId, const QString &command )
: UserTransfer(transactionId), m_command( command )
{
}

Request::~Request()
{
}

QString Request::command()
{
	return m_command;
}


