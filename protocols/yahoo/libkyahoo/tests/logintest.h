/*
    Kopete Yahoo Protocol
    
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

#ifndef logintest_h
#define logintest_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "client.h"
#include "coreprotocol.h"
#include "yahooclientstream.h"
#include "yahooconnector.h"


#define QT_FATAL_ASSERT 1

class LoginTest : public QApplication
{
Q_OBJECT
public:
	LoginTest(int argc, char ** argv);
	
	~LoginTest();
	
	bool isConnected() { return connected; }

public slots:
	void slotDoTest();
	
	void slotConnected();
	
	//void slotWarning(int warning);

	//void slotsend(int layer);

private:
	KNetworkConnector *myConnector;
	ClientStream *myClientStream;
	Client* myClient;

	bool connected;
};

#endif
