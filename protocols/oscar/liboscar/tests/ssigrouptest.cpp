//Licensed under the GNU General Public License

#include "ssigrouptest.h"

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
	QTimer::singleShot( 10000, this, SLOT(runAddGroupTest() ) );
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

void LoginTest::runAddGroupTest()
{
	qDebug( "running ssi group add test" );
	QString group = QString::fromLatin1( "dummygroup" );
	myClient->addGroup( group );
	QTimer::singleShot( 5000, this, SLOT( runDelGroupTest() ) );
}

void LoginTest::runDelGroupTest()
{
	qDebug( "running ssi group del test" );
	QString group = QString::fromLatin1( "dummygroup" );
	myClient->removeGroup( group );
}


#include "ssigrouptest.moc"
