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
#ifndef GW_RESPONSE_H
#define GW_RESPONSE_H

#include "usertransfer.h"

/**
 * Represents the server's reply to a client generated request
 * @author Kopete Developers
*/
class Response: public UserTransfer
{
public:
	Response( int transactionId, Field::FieldList fields );
	~Response( ) {}
	TransferType type() { return Transfer::ResponseTransfer; }
};

#endif
