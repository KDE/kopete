#include "response.h"

Response::Response( int transactionId, int resultCode, Field::FieldList fields )
: UserTransfer( transactionId ), m_resultCode( resultCode )
{
	setFields( fields );
}

int Response::resultCode() const
{
	return m_resultCode;
}