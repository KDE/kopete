/*
    Tests for Kopete BlackLister

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


#include "kopeteblacklister.h"

#include <QObject>
#include <QtTest>
class BlackListerTest : public QObject
{
    Q_OBJECT
private slots:
    void testAddAndRemoveContact();
    void testPersistanceOfBlackListContacts();
};

void BlackListerTest::testAddAndRemoveContact()
{
    const QString ProtocolID = QStringLiteral("TestBed");
    const QString AccountID = QStringLiteral("Default");
    const QString Contact1 = QStringLiteral("Dummy User 1");
    const QString Contact2 = QStringLiteral("Dummy User 2");
    const QString Contact3 = QStringLiteral("Dummy User 3");
    const QString Contact4 = QStringLiteral("Dummy User 4");
    
    Kopete::BlackLister *A = new Kopete::BlackLister(ProtocolID, AccountID, nullptr);
    // Cleanup is required as it may already be present in memory because of unsuccessful test/ interruption
    if(A->isBlocked(Contact1)) A->removeContact(Contact1);
    if(A->isBlocked(Contact2)) A->removeContact(Contact2);
    if(A->isBlocked(Contact3)) A->removeContact(Contact3);
    if(A->isBlocked(Contact4)) A->removeContact(Contact4);
    
    QSignalSpy spyAdd(A, &Kopete::BlackLister::contactAdded);
    QSignalSpy spyRemove(A, &Kopete::BlackLister::contactRemoved);
    QVERIFY(!A->isBlocked(Contact1));
    A->addContact(Contact1);
    QVERIFY(A->isBlocked(Contact1));
    QVERIFY(spyAdd.count() == 1);
    A->addContact(Contact2);
    QVERIFY(spyAdd.count() == 2);
    A->addContact(Contact3);
    QVERIFY(spyAdd.count() == 3);
    A->addContact(Contact4);
    QVERIFY(spyAdd.count() == 4);
    A->removeContact(Contact1);
    QVERIFY(spyRemove.count() == 1);
    QVERIFY(!A->isBlocked(Contact1));
    A->removeContact(Contact2);
    QVERIFY(spyRemove.count() == 2);
    QVERIFY(!A->isBlocked(Contact2));
    A->removeContact(Contact3);
    QVERIFY(spyRemove.count() == 3);
    QVERIFY(!A->isBlocked(Contact3));
    A->removeContact(Contact4);
    QVERIFY(spyRemove.count() == 4);
    QVERIFY(!A->isBlocked(Contact4));
}


void BlackListerTest::testPersistanceOfBlackListContacts()
{
    const QString ProtocolID = QStringLiteral("TestBed");
    const QString AccountID = QStringLiteral("Default");
    const QString Contact1 = QStringLiteral("Dummy User 1");
    const QString Contact2 = QStringLiteral("Dummy User 2");
    Kopete::BlackLister *A = new Kopete::BlackLister(ProtocolID, AccountID, nullptr);
    // Cleanup is required as it may already be present in memory because of unsuccessful test/ interruption
    if(A->isBlocked(Contact1)) A->removeContact(Contact1);
    if(A->isBlocked(Contact2)) A->removeContact(Contact2);
    A->addContact(Contact1);

    // They use same config file should have same records
    Kopete::BlackLister *B = new Kopete::BlackLister(ProtocolID, AccountID, nullptr);
    QVERIFY(B->isBlocked(Contact1));
    B->addContact(Contact2);
    QVERIFY(B->isBlocked(Contact2));
    A->removeContact(Contact1);
    B->removeContact(Contact2);
    QVERIFY(!A->isBlocked(Contact1));
    QVERIFY(!B->isBlocked(Contact2));
}

QTEST_MAIN(BlackListerTest)
#include "kopeteblacklistertest.moc"
