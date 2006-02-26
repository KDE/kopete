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

#include "serverversionstest.h"
#include "serverversionstask.h"
#include "buffer.h"

OSCAR_TEST_MAIN( ServerVersionsTest )

void ServerVersionsTest::testSupportedFamilies()
{
    Buffer* b = new Buffer();
    b->addDWord( 0x00010002 );
    b->addDWord( 0x00030004 );

    QList<int> families = ServerVersionsTask::buildFamiliesList( b );
    QVERIFY( families.isEmpty() ==  false );
    QVERIFY( families.count() == 4 );
    QVERIFY( families.takeFirst() == 0x0001 );
    QVERIFY( families.takeFirst() == 0x0002 );
    QVERIFY( families.takeFirst() == 0x0003 );
    QVERIFY( families.takeFirst() == 0x0004 );
    delete b;
}


#include "serverversionstest.moc"
