//
// C++ Interface: response
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REQUEST_H
#define REQUEST_H

#include "usertransfer.h"

/**
 * Represents the server's reply to a client generated request
 * @author Kopete Developers
*/
class Response: public UserTransfer
{
public:
	Response( int transactionId, QCString &command, Field::FieldList fields );
	~Response( );
	TransferType type() { return Transfer::ResponseTransfer; }
};

#endif
