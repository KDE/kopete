#include "response.h"

Response::Response( int transactionId, Field::FieldList fields )
: UserTransfer( transactionId )
{
	setFields( fields );
}
