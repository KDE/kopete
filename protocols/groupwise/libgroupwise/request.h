//
// C++ Interface: request
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LIBGW_REQUEST_H
#define LIBGW_REQUEST_H

#include "usertransfer.h"

/**
 * Represents a client generated request to the server
 * Create with @ref RequestFactory::request().
 * @author Kopete Developers
*/
class Request : public UserTransfer
{
friend class RequestFactory;

public:
	~Request( );
	QString command();
	TransferType type() { return Transfer::RequestTransfer; }
private:
	Request( const int transactionId, const QString &command );
	QString m_command;
};

#endif
