/*
    AIM Login Task Test

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

#include "aimlogintasktest.h"
#include "buffer.h"
#include "aimlogintask.h"

OSCAR_TEST_MAIN( AimLoginTaskTest )

void AimLoginTaskTest::testAuthString()
{
    if ( ! loadFile("snac1707.buffer") )
    {
        QFAIL("couldn't load test data file");
    }

    QByteArray challenge = AimLoginTask::parseAuthString( m_data );
    QVERIFY( challenge == "2000900472" );

}

#include "aimlogintasktest.moc"
