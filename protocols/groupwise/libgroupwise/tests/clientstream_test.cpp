//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "gwclientstream.h"
#include "requestfactory.h"
#include "request.h"
#include "usertransfer.h"

#include "coreprotocol.h"

int main()
{
	// set up client stream
	
	ClientStream testObject;
	NovellDN dn;
	dn.dn = "maeuschen";
	dn.server = "reiser.suse.de";
	RequestFactory testFactory;
	// connect to server
	testObject.connectToServer( dn, true );
	
	// send a request
	QCString command("login");
	Request * firstRequest = testFactory.request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_USERID, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_UTF8, "blah@fasel.org" ) );
	firstRequest->setFields( lst );
	testObject.write( firstRequest );
	return 0;
}
