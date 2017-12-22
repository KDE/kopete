/*
    usertransfer.cpp - Ancestor of In- or outgoing Transfers (Requests and Response)
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

#include "usertransfer.h"

UserTransfer::UserTransfer( int transactionId )
{
	m_transactionId = transactionId;
}

UserTransfer::~UserTransfer()
{
	m_fields.purge();
}

void UserTransfer::setFields( Field::FieldList fields )
{
	m_fields = fields;
}

int UserTransfer::transactionId() const
{
	return m_transactionId;
}

Field::FieldList UserTransfer::fields()
{
	return m_fields;
}

