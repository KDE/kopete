/*
    Offline Messages Task Test

    Copyright (c) 2006 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "offlinemessagestest.h"
#include "offlinemessagestask.h"
#include "buffer.h"

OSCAR_TEST_MAIN( OfflineMessagesTest )

void OfflineMessagesTest::testOfflineMessages()
{
	if (! loadFile("snac1503.buffer")){
		QFAIL("couldn't load test data file");
	}
	//m_data should now be a buffer with our data

	Oscar::Message msg = OfflineMessagesTask::parseOfflineMessage( m_data );
	QVERIFY( msg.receiver() == "52009835" );
	QVERIFY( msg.sender() == "33146327" );
	QVERIFY( msg.textArray() == "This is a test" );
	QVERIFY( msg.properties() == 0 );
	QVERIFY( msg.channel() == 1 );
	QCOMPARE( msg.timestamp().toString(), 
			QString("Sat Mar 4 11:38:00 2006") );

}


#include "offlinemessagestest.moc"
