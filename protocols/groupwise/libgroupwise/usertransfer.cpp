//
// C++ Implementation: usertransfer
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "usertransfer.h"

UserTransfer::UserTransfer( int transactionId )
{
	m_transactionId = transactionId;
}

UserTransfer::~UserTransfer()
{
}

void UserTransfer::setFields( Field::FieldList fields )
{
	m_fields = fields;
}

int UserTransfer::transactionId()
{
	return m_transactionId;
}

Field::FieldList UserTransfer::fields()
{
	return m_fields;
}


