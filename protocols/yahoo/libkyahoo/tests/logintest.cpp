//Licensed under the GNU General Public License

#include "logintest.h"
#include <kdebug.h>
#include "../ymsgtransfer.h"
#include "../yahootypes.h"

LoginTest::LoginTest(int argc, char ** argv) : QApplication( argc, argv )
{
	// set up client stream
	myConnector = new KNetworkConnector( 0 );
	//myConnector->setOptHostPort( "localhost", 8300 );
	myConnector->setOptHostPort( "scs.msg.yahoo.com", 5050 );
	myClientStream = new ClientStream( myConnector, myConnector);
	// notify when the transport layer is connected
	myClient = new Client();
	// do test once the event loop is running
	QTimer::singleShot( 0, this, SLOT( slotDoTest() ) );
	connected = false;
}

LoginTest::~LoginTest()
{
	delete myClientStream;
	delete myConnector;
	delete myClient;
}

void LoginTest::slotDoTest()
{
	QString server = QString::fromLatin1("scs.msg.yahoo.com");
	// connect to server
	kdDebug(14180) << k_funcinfo << " connecting to server" << endl;
	
	connect( myClient, SIGNAL( connected() ), SLOT( slotConnected() ) );
	myClient->start( server, 5050, "kopetetest", "snapscan" );
	myClient->connectToServer( myClientStream, server, true );
}

void LoginTest::slotConnected()
{	
	kdDebug(14180) << k_funcinfo << " connection is up" << endl;
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
