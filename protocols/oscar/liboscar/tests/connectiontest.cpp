/*
    Connection Test

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

#include "connectiontest.h"
#include "connection.h"
#include "QList"

OSCAR_TEST_MAIN( ConnectionTest )

void ConnectionTest::checkSupportedFamiliesSingle()
{
    Connection* c = new Connection( 0, 0 );
    QVERIFY( c->supportedFamilies().isEmpty() );
    c->addToSupportedFamilies( 1 );
    QVERIFY( c->supportedFamilies().count() == 1  );
    c->addToSupportedFamilies( 2 );
    c->addToSupportedFamilies( 0x15 );
    QVERIFY( c->supportedFamilies().count() == 3  );
    QVERIFY( c->isSupported( 1 ) == true );
    QVERIFY( c->isSupported( 0 ) == false );
    QVERIFY( c->isSupported( 2 ) == true );
    QVERIFY( c->isSupported( 0x15 ) == true  );
    delete c;
}

void ConnectionTest::checkSupportedFamiliesList()
{
    Connection* c = new Connection( 0, 0 );
    QVERIFY( c->supportedFamilies().isEmpty() );
    QList<int> families;
    families.append( 1 );
    families.append( 0x15 );
    families.append( 2 );
    families.append( 0x0F );
    c->addToSupportedFamilies( families );
    QVERIFY( c->supportedFamilies().count() == 4 );
    QVERIFY( c->isSupported( 0x15 ) );
    QVERIFY( c->isSupported( 1 ) );
    QVERIFY( c->isSupported( 2 ) );
    QVERIFY( c->isSupported( 0x0F ) );
    delete c;
}

#include "connectiontest.moc"
