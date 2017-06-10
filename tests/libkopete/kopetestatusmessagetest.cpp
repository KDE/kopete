/*
    Tests for Kopete::StatusMessage

    Copyright (c) 2017      by Vijay Krishnavanshi    <vijaykrishnavashi>

    Kopete    (c) 2002-2017 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "kopetestatusmessage.h"
#include <QObject>
#include <QtTest>
class StatusMessageTest : public QObject
{
    Q_OBJECT
private slots:
    void testStatus();
    void testConstructors();
    void testStatusMetaData();
    void testStatusHash();
    void testEmptyStatus();
};

void StatusMessageTest::testStatus()
{
    const QString str = QStringLiteral("given we are here this should work");
    Kopete::StatusMessage status(str);
    status.setMessage(str);
    QCOMPARE(status.message(), str);
}

void StatusMessageTest::testConstructors()
{
    const QString str = QStringLiteral("Hello");
    Kopete::StatusMessage statusD(str);
    Kopete::StatusMessage statusP;
    statusP.setMessage(str);
    QCOMPARE(statusD.message(), statusP.message());
}

void StatusMessageTest::testStatusMetaData()
{
    const QString sname = QStringLiteral("Coldplay - Paradise");
    const QString smplayer = QStringLiteral("VLC");
    const QString sartist = QStringLiteral("ColdPlay");
    const QString stitle = QStringLiteral("Paradise");
    Kopete::StatusMessage status(sname);
    status.addMetaData(QStringLiteral("MusicPlayer"), smplayer);
    status.addMetaData(QStringLiteral("artist"), sartist);
    status.addMetaData(QStringLiteral("title"), stitle);
    QVERIFY(status.hasMetaData("sadhgsadf") == false);
    QCOMPARE(status.hasMetaData("artist") , true);
    QCOMPARE(status.metaData("MusicPlayer").toString(), smplayer);
    QCOMPARE(status.metaData("artist").toString(), sartist);
    QCOMPARE(status.metaData("title").toString(), stitle);
    QCOMPARE(status.message(), sname);
}

void StatusMessageTest::testStatusHash()
{
    const QString sname = QStringLiteral("Coldplay - Paradise");
    const QString smplayer = QStringLiteral("VLC");
    const QString sartist = QStringLiteral("ColdPlay");
    const QString stitle = QStringLiteral("Paradise");
    const QString salbum = QStringLiteral("Unknown");
    Kopete::StatusMessage status(sname);
    status.addMetaData(QStringLiteral("MusicPlayer"), smplayer);
    status.addMetaData(QStringLiteral("artist"), sartist);
    status.addMetaData(QStringLiteral("title"), stitle);
    status.addMetaData(QStringLiteral("album"), salbum);
    QCOMPARE(status.metaData("artist").toString(), sartist);
    QCOMPARE(status.metaData("title").toString(), stitle);
    QHash<QString, QVariant> metaDataHash;
    metaDataHash.insert(QStringLiteral("artist"), QStringLiteral("Chris Martin"));
    metaDataHash.insert(QStringLiteral("title"), QStringLiteral("Undefined"));
    status.addMetaData(metaDataHash);
    QCOMPARE(status.metaData("artist").toString(), QStringLiteral("Chris Martin"));
    QCOMPARE(status.metaData("title").toString(), QStringLiteral("Undefined"));
    QCOMPARE(status.metaData("album").toString(), salbum);
}

void StatusMessageTest::testEmptyStatus()
{
    Kopete::StatusMessage status;
    QCOMPARE(status.isEmpty(), true);
}

QTEST_MAIN(StatusMessageTest)
#include "kopetestatusmessagetest.moc"
