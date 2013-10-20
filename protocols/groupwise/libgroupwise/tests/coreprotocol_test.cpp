//
// C++ Implementation: coreprotocol_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "requestfactory.h"
#include "request.h"
#include "usertransfer.h"

#include "coreprotocol.h"
#include <QByteArray>

int main()
{
	CoreProtocol testObject;
	RequestFactory testFactory;
	QByteArray command("login");
	Request * firstRequest = testFactory.request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_USERID, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_UTF8, "blah@fasel.org" ) );
	firstRequest->setFields( lst );
	testObject.outgoingTransfer( firstRequest );
	return 0;
}
