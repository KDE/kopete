/*
    usertransfer.h - Ancestor of In- or outgoing Transfers (Requests and Response)
    initated by the user.
   
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
    virtual ~UserTransfer();
	int transactionId() const;
	Field::FieldList fields();
	void setFields( Field::FieldList fields );
	
private:
	int m_transactionId;
	Field::FieldList m_fields;
	
};

#endif
