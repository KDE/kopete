/*
	clienttest.cpp
 
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

#include "smpppdclient.h"

#include "clienttest.h"

ClientTest::ClientTest(const char * name)
        : KUnitTest::SlotTester(name) {}

ClientTest::~ClientTest() {}

void ClientTest::testInitIsReady() {
    SMPPPD::Client c;
    CHECK(c.isReady(), false);
}

void ClientTest::testAfterConnectIsReady() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        CHECK(c.isReady(), true);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testConnect() {
    SMPPPD::Client c;
    CHECK(c.connect("warwar", 3185), true);
    CHECK(c.connect("localhost", 3185), false);
}

void ClientTest::testCommunicationBeforeConnect() {
    SMPPPD::Client c;
    QStringList l = c.getInterfaceConfigurations();

    CHECK(l.count() == 0, true);
    CHECK(c.statusInterface("ifcfg0"), false);
}

void ClientTest::testServerIDBeforeConnect() {
    SMPPPD::Client c;
    CHECK(c.serverID(), QString::null);
}

void ClientTest::testServerVersionBeforeConnect() {
    SMPPPD::Client c;
    CHECK(c.serverVersion(), QString::null);
}

void ClientTest::testCommunicationAfterConnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        CHECK(c.getInterfaceConfigurations().count() > 0, true);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testServerIDAfterConnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        CHECK(c.serverID().isEmpty(), false);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testServerVersionAfterConnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        CHECK(c.serverVersion().isEmpty(), false);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testCommunicationAfterDisconnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        c.disconnect();
        CHECK(c.getInterfaceConfigurations().count() == 0, true);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testServerIDAfterDisconnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        c.disconnect();
        CHECK(c.serverID(), QString::null);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

void ClientTest::testServerVersionAfterDisconnect() {
    SMPPPD::Client c;
    if(c.connect("warwar", 3185)) {
        c.disconnect();
        CHECK(c.serverVersion(), QString::null);
    } else {
        SKIP("Test skipped because no smpppd at warwar:3185");
    }
}

#include "clienttest.moc"
