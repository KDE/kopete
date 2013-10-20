#include "clientstream_test.h"
#include <QByteArray>

ClientStreamTest::ClientStreamTest(int argc, char ** argv) : QApplication( argc, argv )
{
	// set up client stream
	myConnector = new KNetworkConnector( 0 );
	//myConnector->setOptHostPort( "localhost", 8300 );
	myConnector->setOptHostPort( "reiser.suse.de", 8300 );
	myConnector->setOptSSL( true );
	Q_ASSERT( QCA::isSupported(QCA::CAP_TLS) );
	myTLS = new QCA::TLS;
	myTLSHandler = new QCATLSHandler( myTLS );
	myTestObject = new ClientStream( myConnector, myTLSHandler, 0);
	// notify when the transport layer is connected
	connect( myTestObject, SIGNAL(connected()), SLOT(slotConnected()) );
	// it's necessary to catch this signal and tell the TLS handler to proceed, even if we don't check cert validity
	connect( myTLSHandler, SIGNAL(tlsHandshaken()), SLOT(slotTLSHandshaken()) );
	// notify and start sending
	connect( myTestObject, SIGNAL(securityLayerActivated(int)), SLOT(slotSend(int)) );
	connect( myTestObject, SIGNAL(warning(int)), SLOT(slotWarning(int)) );

	// do test once the event loop is running
	QTimer::singleShot( 0, this, SLOT(slotDoTest()) );
}

ClientStreamTest::~ClientStreamTest()
{
	delete myTestObject;
	delete myTLSHandler;
	delete myTLS;
	delete myConnector;
}

void ClientStreamTest::slotDoTest()
{
	NovellDN dn;
	dn.dn = "maeuschen";
	dn.server = "reiser.suse.de";
	// connect to server
	qDebug( "connecting to server ");
	myTestObject->connectToServer( dn, true ); // fine up to here...
}

void ClientStreamTest::slotConnected()
{
	qDebug( "connection is up");
}

void ClientStreamTest::slotWarning(int warning)
{
	qDebug( "warning: %i", warning);
}

void ClientStreamTest::slotsend(int layer)
{
	qDebug( "security layer is up: %i", layer);
	RequestFactory testFactory;
	// we're not connecting...
	qDebug( "sending request" );
	// send a request
	QByteArray command("login");
	Request * firstRequest = testFactory.request( command );
	Field::FieldList lst;
	lst.append( new Field::SingleField( Field::NM_A_SZ_USERID, 0, NMFIELD_TYPE_UTF8, "maeuschen" ) );
	lst.append( new Field::SingleField( NM_A_SZ_CREDENTIALS, 0, NMFIELD_TYPE_UTF8, "maeuschen" ) );
	lst.append( new Field::SingleField( NM_A_SZ_USER_AGENT, 0, NMFIELD_TYPE_UTF8, "libgroupwise/0.1 (linux, 2.6.5-7.97-smp)" ) );
	lst.append( new Field::SingleField( NM_A_UD_BUILD, 0, NMFIELD_TYPE_UDWORD, 2 ) );
	lst.append( new Field::SingleField( GroupWise::NM_A_IP_ADDRESS, 0, NMFIELD_TYPE_UTF8, "10.10.11.103" ) );
	firstRequest->setFields( lst );
	myTestObject->write( firstRequest );
	qDebug( "done");
}
	
void ClientStreamTest::slotTLSHandshaken()
{
	qDebug( "TLS handshake complete" );
	int validityResult = myTLS->certificateValidityResult ();

	if( validityResult == QCA::TLS::Valid )
	{
		qDebug( "Certificate is valid, continuing.");
		// valid certificate, continue
		myTLSHandler->continueAfterHandshake ();
	}
	else
	{
		qDebug( "Certificate is not valid, continuing" );
		// certificate is not valid, query the user
		/*			if(handleTLSWarning (validityResult, server (), myself()->contactId ()) == KMessageBox::Continue)
					{*/
		myTLSHandler->continueAfterHandshake ();
		/*			}
					else
					{
					disconnect ( Kopete::Account::Manual );
					}*/
	}

}
int main(int argc, char ** argv)
{
	ClientStreamTest a( argc, argv );
	a.exec();
	return 0;
}

#include "clientstream_test.moc"
