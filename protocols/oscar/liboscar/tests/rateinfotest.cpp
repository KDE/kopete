/*
    Server Versions Task Test

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

#include "rateinfotest.h"
#include "rateinfotask.h"
#include "buffer.h"
#include "rateclass.h"

OSCAR_TEST_MAIN( RateInfoTest )

void RateInfoTest::testRateClasses()
{
	if ( ! loadFile("snac0107.buffer") )
	{
		QFAIL("couldn't load test data file");
	}
	//m_data should now be a buffer with our data

	QList<RateClass*> rates = RateInfoTask::parseRateClasses( m_data );

	QVERIFY( rates.isEmpty() ==  false );
	QVERIFY( rates.count() == 5 );

	//verify each rate class
	QList<RateClass*>::iterator it = rates.begin();
	QList<RateClass*>::iterator rcEnd = rates.end();
	for ( int i=1; it != rcEnd; ++it, ++i )
	{
		RateClass *r = *it;
		QVERIFY( r->id() == i );
		Oscar::RateInfo ri = r->getRateInfo();

		//now we have to verify everything in the struct
		if ( i < 3 )
		{
			QVERIFY( ri.windowSize == 0x50 );
			QVERIFY( ri.alertLevel == 0x7d0 );
			QVERIFY( ri.limitLevel == 0x5dc );
			QVERIFY( ri.maxLevel == 0x1770 );
			if ( i==1 )
			{
				QVERIFY( ri.clearLevel == 0x9c4 );
				QVERIFY( ri.disconnectLevel == 0x320 );
				QVERIFY( ri.currentLevel == 0x16dd );
			}
			else
			{
				QVERIFY( ri.clearLevel == 0xbb8 );
				QVERIFY( ri.disconnectLevel == 0x3e8 );
				QVERIFY( ri.currentLevel == 0x1770 );
			}
		}
		else if ( i==3 )
		{
			QVERIFY( ri.windowSize == 0x14 );
			QVERIFY( ri.clearLevel == 0xc1c );
			QVERIFY( ri.alertLevel == 0x9c4 );
			QVERIFY( ri.limitLevel == 0x7d0 );
			QVERIFY( ri.disconnectLevel == 0x5dc );
			QVERIFY( ri.currentLevel == 0xdac );
			QVERIFY( ri.maxLevel == 0x1194 );
		}
		else // i>3
		{
			if ( i==4 )
				QVERIFY( ri.windowSize == 0x14 );
			else
				QVERIFY( ri.windowSize == 0xa );
			QVERIFY( ri.clearLevel == 0x157c );
			QVERIFY( ri.alertLevel == 0x14b4 );
			QVERIFY( ri.limitLevel == 0x1068 );
			QVERIFY( ri.disconnectLevel == 0xbb8 );
			QVERIFY( ri.currentLevel == 0x1770 );
			QVERIFY( ri.maxLevel == 0x1f40 );
		}
		if ( i==1 )
			QVERIFY( ri.lastTime == 0 );
		else
			QVERIFY( ri.lastTime == 0xcd );
		QVERIFY( ri.currentState == 0 );

		//we won't bother verifying the rate group stuff, it's not important.
	}
}


#include "rateinfotest.moc"
