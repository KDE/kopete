//Licensed under the GNU General Public License

#include "clientstream_test.h"
#include <kdebug.h>
#include "../ymsgtransfer.h"
#include "../yahootypes.h"

ClientStreamTest::ClientStreamTest(int argc, char ** argv) : QApplication( argc, argv )
{
	// set up client stream
	myConnector = new KNetworkConnector( 0 );
	//myConnector->setOptHostPort( "localhost", 8300 );
	myConnector->setOptHostPort( "scs.msg.yahoo.com", 5050 );
	myTestObject = new ClientStream( myConnector, myConnector);
	// notify when the transport layer is connected
	connect( myTestObject, SIGNAL( connected() ), SLOT( slotConnected() ) );
	// notify and start sending
	//connect( myTestObject, SIGNAL( warning(int) ), SLOT( slotWarning(int) ) );

	// do test once the event loop is running
	QTimer::singleShot( 0, this, SLOT( slotDoTest() ) );
	connected = false;
}

ClientStreamTest::~ClientStreamTest()
{
	delete myTestObject;
	delete myConnector;
}

void ClientStreamTest::slotDoTest()
{
	QString server = QString::fromLatin1("scs.msg.yahoo.com");
	// connect to server
	kdDebug(14180) << k_funcinfo << " connecting to server" << endl;
	myTestObject->connectToServer( server, true ); // fine up to here...
}

void ClientStreamTest::slotConnected()
{
	kdDebug(14180) << k_funcinfo << " connection is up" << endl;
	connected = true;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceLogon);
	t->setParam( 1, "kopetetest");
	
	myTestObject->write(t);
	while(1);
}

int main(int argc, char ** argv)
{
	ClientStreamTest a( argc, argv );
	a.exec();
	return 0;
}

#include "clientstream_test.moc"
