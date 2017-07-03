/*
    Tests for Kopete Contact List Element

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


#include "kopetecontactlistelement.h"
#include "kopeteplugin.h"
#include <QObject>
#include <QtTest>

class TestContact : public Kopete::ContactListElement
{
    Q_OBJECT

public:
    TestContact() : Kopete::ContactListElement(nullptr)
    {

    }   
};

class ContactListElementTest : public QObject
{
    Q_OBJECT
private slots:
    void testIconAndMetaPart();
    void testPluginDataPart();
    void testPluginContactDataPart();
};

void ContactListElementTest::testIconAndMetaPart()
{
    TestContact *A = new TestContact();

    QVERIFY(!A->loading());
    A->setLoading(true);
    QVERIFY(A->loading());
    
    QVERIFY(!A->useCustomIcon());
    
    QSignalSpy spy(A, &TestContact::useCustomIconChanged);
    A->setUseCustomIcon(true);
    QVERIFY(A->useCustomIcon());
    QVERIFY(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool() == true);
    
    // Spy above is reset after this so we create a new spy for signal
    QSignalSpy spy1(A, &TestContact::useCustomIconChanged);
    A->setUseCustomIcon(true);
    QVERIFY(A->useCustomIcon());
    QVERIFY(spy1.count() == 0);
    A->setUseCustomIcon(false);
    QVERIFY(spy1.count() == 1);

    Kopete::ContactListElement::IconMap iMap = A->icons();
    QVERIFY(iMap.isEmpty());
    
    const QString icon = QStringLiteral("None Icon");
    QSignalSpy spyIconChanged(A, &Kopete::ContactListElement::iconChanged);
    QSignalSpy spyIconAppearanceChanged(A, &Kopete::ContactListElement::iconAppearanceChanged);
    A->setIcon(icon, Kopete::ContactListElement::IconState::None);
    QVERIFY(spyIconChanged.count() == 1);
    QVERIFY(spyIconAppearanceChanged.count() == 1);
    
    QString expected = A->icon(Kopete::ContactListElement::IconState::None);
    QCOMPARE(icon, expected);
    
    Kopete::ContactListElement::IconMap updatedIMap = A->icons();
    expected = updatedIMap[Kopete::ContactListElement::IconState::None];
    QCOMPARE(icon, expected);
}


void ContactListElementTest::testPluginDataPart()
{
    TestContact *dummyContact = new TestContact();
    Kopete::Plugin *testPlugin = new Kopete::Plugin(nullptr);
    QMap<QString, QString> testPluginData;
    const QString Key1 = QStringLiteral("Key 1"), Value1 = QStringLiteral("Value 1");
    const QString Key2 = QStringLiteral("Key 2"), Value2 = QStringLiteral("Value 2");
    QSignalSpy spy(dummyContact, &TestContact::pluginDataChanged);
    testPluginData[Key1] = Value1;
    dummyContact->setPluginData(testPlugin, testPluginData);
    QVERIFY(spy.count() == 1);
    dummyContact->setPluginData(testPlugin, Key2, Value2);
    QVERIFY(spy.count() == 2);
    testPluginData[Key2] = Value2;
    QMap<QString, QString> expectedPluginData;
    expectedPluginData = dummyContact->pluginData(testPlugin);
    QCOMPARE(testPluginData[Key1], expectedPluginData[Key1]);
    QCOMPARE(testPluginData[Key2], expectedPluginData[Key2]);
    QCOMPARE(testPluginData[Key1], dummyContact->pluginData(testPlugin, Key1));
    QCOMPARE(testPluginData[Key2], dummyContact->pluginData(testPlugin, Key2));
}

void ContactListElementTest::testPluginContactDataPart()
{
    TestContact *dummyContact = new TestContact();
    Kopete::Plugin *testPlugin = new Kopete::Plugin(nullptr);
    const QString Key1 = QStringLiteral("Name 1"), Value1 = QStringLiteral("Phone 1");
    QMap<QString, QString> testContactData1;
    testContactData1[Key1] = Value1;
    const QString Key2 = QStringLiteral("Name 2"), Value2 = QStringLiteral("Phone 2");
    QMap<QString, QString> testContactData2;
    testContactData2[Key2] = Value2;
    QList< QMap<QString, QString> > testContactDataList;
    testContactDataList.append(testContactData1);
    testContactDataList.append(testContactData2);
    QList< QMap<QString, QString> > expectedContactDataList;
    QSignalSpy spy(dummyContact, &TestContact::pluginDataChanged);
    dummyContact->setPluginContactData(testPlugin, testContactDataList);
    QVERIFY(spy.count() == 1);
    const QString Key3 = QStringLiteral("Name 3"), Value3 = QStringLiteral("Phone 3");
    QMap<QString, QString> testContactData3;
    testContactData3[Key3] = Value3;
    dummyContact->appendPluginContactData(testPlugin->pluginId(), testContactData3);
    QVERIFY(spy.count() == 2);
    testContactDataList.append(testContactData3);
    expectedContactDataList = dummyContact->pluginContactData(testPlugin);
    QCOMPARE(expectedContactDataList[0][Key1], testContactDataList[0][Key1]);
    QCOMPARE(expectedContactDataList[1][Key2], testContactDataList[1][Key2]);
    QCOMPARE(expectedContactDataList[2][Key3], testContactDataList[2][Key3]);
}

QTEST_MAIN(ContactListElementTest)
#include "kopetecontactlistelementtest.moc"
