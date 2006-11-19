/*
    Unit test for Kopete::StatusMessage class.

    Copyright (c) 2006  by Michaël Larouche          <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "statusmessage_test.h"

#include <qtest_kde.h>
#include <QLatin1String>
#include <QHash>

#include "kopetestatusmessage.h"

QTEST_KDEMAIN( StatusMessage_Test, GUI )


void StatusMessage_Test::testNormalStatus()
{
	Kopete::StatusMessage status1;
	status1.setMessage( QLatin1String("http://kopete.kde.org/") );

	QCOMPARE( status1.message(), QString("http://kopete.kde.org/") );

	Kopete::StatusMessage status2( QLatin1String("QTestLib rocks !") );
	
	QCOMPARE( status2.message(), QString("QTestLib rocks !") );
}

void StatusMessage_Test::testMusicMetaData()
{
	Kopete::StatusMessage status2;
	status2.setMessage( QLatin1String("Jordan Rudess = keyboard god") );

	status2.addMetaData( QLatin1String("musicPlayer"), QString("amaroK") );
	status2.addMetaData( QLatin1String("artist"), QString("Dream Theater") );
	status2.addMetaData( QLatin1String("title"), QString("Beyond This Life") );
	status2.addMetaData( QLatin1String("album"), QString("Live Scenes From New York") );
	
	QCOMPARE( status2.hasMetaData("hjjhadhasdasd"), false );
	QCOMPARE( status2.hasMetaData("artist"), true );
	QCOMPARE( status2.metaData("artist").toString(), QString("Dream Theater") );
	QCOMPARE( status2.message(), QString("Jordan Rudess = keyboard god") );
}

void StatusMessage_Test::testAppendHash()
{
	Kopete::StatusMessage status3;
	status3.setMessage( QLatin1String("Jordan Rudess = keyboard god") );

	status3.addMetaData( QLatin1String("musicPlayer"), QString("amaroK") );
	status3.addMetaData( QLatin1String("artist"), QString("Dream Theater") );
	status3.addMetaData( QLatin1String("title"), QString("Beyond This Life") );
	status3.addMetaData( QLatin1String("album"), QString("Live Scenes From New York") );

	QCOMPARE( status3.metaData("artist").toString(), QString("Dream Theater") );
	QCOMPARE( status3.metaData("title").toString(), QString("Beyond This Life") );

	QHash<QString,QVariant> metadataHash;
	metadataHash.insert( QLatin1String("artist"), QLatin1String("Iron Maiden") );
	metadataHash.insert( QLatin1String("title"), QLatin1String("Blood Brothers") );

	status3.addMetaData( metadataHash);

	QCOMPARE( status3.metaData("artist").toString(), QString("Iron Maiden") );
	QCOMPARE( status3.metaData("title").toString(), QString("Blood Brothers") );
	QCOMPARE( status3.metaData("album").toString(), QString("Live Scenes From New York") );
}

#include "statusmessage_test.moc"
