//
// C++ Interface: usertransfer
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef USERTRANSFER_H
#define USERTRANSFER_H

#include "gwfield.h"

#include "transfer.h"

/**
 * Represents transfers of data in response to a user action, either outgoing Requests, or incoming Responses
 * @author Kopete Developers
 */
class UserTransfer : public Transfer
{
public:
    UserTransfer( int transactionId );
    ~UserTransfer();
	int transactionId();
	Field::FieldList fields();
	void setFields( Field::FieldList fields );
	
private:
	int m_transactionId;
	Field::FieldList m_fields;
	
};

#endif
