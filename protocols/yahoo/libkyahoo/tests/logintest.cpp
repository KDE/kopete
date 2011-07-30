/*
    Kopete Yahoo Protocol Tests
    
    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>
    
    Based on code 
    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

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
	QTimer::singleShot( 0, this, SLOT(slotDoTest()) );
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
	QLatin1String server("scs.msg.yahoo.com");
	// connect to server
	kDebug(14180) << " connecting to server";
	
	connect( myClient, SIGNAL(connected()), SLOT(slotConnected()) );
	myClient->start( server, 5050, "duncanmacvicar", "**********" );
	myClient->connectToServer( myClientStream, server, true );
}

void LoginTest::slotConnected()
{	
	kDebug(14180) << " connection is up";
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
