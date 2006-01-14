//Licensed under the GNU General Public License

#include "clientstream_test.h"

ClientStreamTest::ClientStreamTest(int argc, char ** argv) : QApplication( argc, argv )
{
	// set up client stream
	myConnector = new KNetworkConnector( 0 );
	//myConnector->setOptHostPort( "localhost", 8300 );
	myConnector->setOptHostPort( "login.oscar.aol.com", 5190 );
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
	QString server = QString::fromLatin1("login.oscar.aol.com");
	// connect to server
	qDebug( "connecting to server ");
	myTestObject->connectToServer( server, true ); // fine up to here...
}

void ClientStreamTest::slotConnected()
{
	qDebug( "connection is up");
	connected = true;
}

int main(int argc, char ** argv)
{
	ClientStreamTest a( argc, argv );
	a.exec();
	return 0;
}

#include "clientstream_test.moc"
