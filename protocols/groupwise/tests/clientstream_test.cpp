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

#include <qglobal.h>
#include <qapplication.h>
#include "gwclientstream.h"
#include "gwconnector.h"
#include <qca.h>
#include "qcatlshandler.h"
#include "requestfactory.h"
#include "request.h"
#include "usertransfer.h"

#include "coreprotocol.h"

#define QT_FATAL_ASSERT 1

int main()
{
	// set up client stream
	KNetworkConnector myConnector( 0 );
	myConnector.setOptHostPort( "reiser.suse.de", 8300 );
	Q_ASSERT( QCA::isSupported(QCA::CAP_TLS) );
	QCA::TLS * myTLS = new QCA::TLS;
	QCATLSHandler * myTLSHandler = new QCATLSHandler( myTLS );

	ClientStream testObject( &myConnector, myTLSHandler, 0);
	NovellDN dn;
	dn.dn = "maeuschen";
	dn.server = "reiser.suse.de";
	RequestFactory testFactory;
	// connect to server
	qDebug( "connecting to server ");
	testObject.connectToServer( dn, true );
	// we're not connecting...
	
	// or maybe we are but we're not starting the qt event loop in this program!
	
	// send a request
	QCString command("login");
	Request * firstRequest = testFactory.request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( NM_A_SZ_USERID, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_UTF8, "blah@fasel.org" ) );
	firstRequest->setFields( lst );
	testObject.write( firstRequest );
	qDebug( "done");
	return 0;
	
}
