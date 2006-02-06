/*
	clienttest.h
 
	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>
 
	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#ifndef CLIENTTEST_H
#define CLIENTTEST_H

#include <kunittest/tester.h>

/**
	@author Heiko Sch&auml;fer <heiko@rangun.de>
*/
class ClientTest : public KUnitTest::SlotTester {
    Q_OBJECT

    ClientTest(const ClientTest&);
    ClientTest& operator=(const ClientTest&);

public:
    ClientTest(const char * name = 0);
    virtual ~ClientTest();

private slots:
    void testInitIsReady();
    void testAfterConnectIsReady();
    void testConnect();
    void testCommunicationBeforeConnect();
    void testServerIDBeforeConnect();
    void testServerVersionBeforeConnect();
    void testCommunicationAfterConnect();
    void testServerIDAfterConnect();
    void testServerVersionAfterConnect();
    void testCommunicationAfterDisconnect();
    void testServerIDAfterDisconnect();
    void testServerVersionAfterDisconnect();
};

#endif
