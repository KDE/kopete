//Licensed under the GNU General Public License

#include "logintest.h"

LoginTest::LoginTest(int argc, char ** argv) : QApplication( argc, argv )
{
	// set up client stream
	myConnector = new KNetworkConnector( 0 );
	myConnector->setOptHostPort( "login.oscar.aol.com", 5190 );
	myTestObject = new ClientStream( myConnector, myConnector);
	
	// notify when the transport layer is connected
	//connect( myTestObject, SIGNAL( connected() ), SLOT( slotConnected() ) );
	myClient = new Client();
	
	myConnection = new Connection( myConnector, myTestObject, "AUTHORIZER" );
	myConnection->setClient( myClient );
	// do test once the event loop is running
	QTimer::singleShot( 0, this, SLOT( slotDoTest() ) );
	connected = false;
}

LoginTest::~LoginTest()
{
	delete myTestObject;
	delete myConnector;
	delete myClient;
}

void LoginTest::slotDoTest()
{
	QString server = QString::fromLatin1("login.oscar.aol.com");
	// connect to server
	qDebug( "connecting to server ");

	myClient->setIsIcq( true );
	myClient->start( server, 5190, "userid", "password" );
	myClient->connectToServer( myConnection, server, true );
	connected = true;
}

void LoginTest::slotConnected()
{
	qDebug( "connection is up");
	connected = true;
}

int main(int argc, char ** argv)
{
	LoginTest a( argc, argv );
	a.exec();
	if ( !a.isConnected() )
		return 0;
}

#include "logintest.moc"
