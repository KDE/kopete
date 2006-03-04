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

OSCAR_TEST_MAIN( RateInfoTest )

void RateInfoTest::testRateClasses()
{
    if (! loadFile("snac0107.buffer")){
	    QFAIL("couldn't load test data file");
    }
    //m_data should now be a buffer with our data

    Q3ValueList<RateClass*> rates = RateInfoTask::parseRateClasses( m_data );
    QVERIFY( rates.isEmpty() ==  false );
    QVERIFY( rates.count() == 5 );
}


#include "rateinfotest.moc"
