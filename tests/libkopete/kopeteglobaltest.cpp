/*
    Tests for Kopete::Global::Properties

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


#include "kopeteglobal.h"
#include "kopeteproperty.h"

#include <QObject>
#include <QtTest>
Q_DECLARE_METATYPE(Kopete::PropertyTmpl::PropertyOption);
class GlobalPropertiesTest : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultProperties();
    void testCustomProperties();
    void testDefaultProperties_data();
};

void GlobalPropertiesTest::testDefaultProperties_data()
{
    QTest::addColumn<QString>("PropertyName");
    QTest::addColumn<QString>("Key");
    QTest::addColumn<QString>("Label");
    QTest::addColumn<QString>("Icon");
    QTest::addColumn<Kopete::PropertyTmpl::PropertyOption>("Option");
    QTest::newRow("Test Fromatted Name") << QStringLiteral("FormattedName") << QStringLiteral("FormattedName") << QStringLiteral("Full Name") << QString() << Kopete::PropertyTmpl::NoProperty;
    QTest::newRow("Test Idle Time") << QStringLiteral("idleTime") << QStringLiteral("idleTime") << QStringLiteral("Idle Time") << QString() << Kopete::PropertyTmpl::NoProperty;
    QTest::newRow("Test Online Since") << QStringLiteral("onlineSince") << QStringLiteral("onlineSince") << QStringLiteral("Online Since") << QString() << Kopete::PropertyTmpl::NoProperty;
    QTest::newRow("Test Last Seen") << QStringLiteral("lastSeen") << QStringLiteral("lastSeen") << QStringLiteral("Last Seen") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Status Message") << QStringLiteral("statusMessage") << QStringLiteral("statusMessage") << QStringLiteral("Status Message") << QString() << Kopete::PropertyTmpl::NoProperty;
    QTest::newRow("Test First Name") << QStringLiteral("firstName") << QStringLiteral("firstName") << QStringLiteral("First Name") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Last Name") << QStringLiteral("lastName") << QStringLiteral("lastName") << QStringLiteral("Last Name") << QString() << Kopete::PropertyTmpl::PersistentProperty;    
    QTest::newRow("Test Private Phone") << QStringLiteral("privatePhoneNumber") << QStringLiteral("privatePhoneNumber") << QStringLiteral("Private Phone") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Private Mobile Phone") << QStringLiteral("privateMobilePhoneNumber") << QStringLiteral("privateMobilePhoneNumber") << QStringLiteral("Private Mobile Phone") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Work Phone") << QStringLiteral("workPhoneNumber") << QStringLiteral("workPhoneNumber") << QStringLiteral("Work Phone") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Work Mobile Phone") << QStringLiteral("workMobilePhoneNumber") << QStringLiteral("workMobilePhoneNumber") << QStringLiteral("Work Mobile Phone") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Email Address") << QStringLiteral("emailAddress") << QStringLiteral("emailAddress") << QStringLiteral("Email Address") << QStringLiteral("mail") << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Nick Name") << QStringLiteral("nickName") << QStringLiteral("nickName") << QStringLiteral("Nick Name") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Custom Name") << QStringLiteral("customName") << QStringLiteral("customName") << QStringLiteral("Custom Name") << QString() << Kopete::PropertyTmpl::PersistentProperty;
    QTest::newRow("Test Photo") << QStringLiteral("photo") << QStringLiteral("photo") << QStringLiteral("Photo") << QString() << Kopete::PropertyTmpl::PersistentProperty;
}

void GlobalPropertiesTest::testDefaultProperties()
{
    Kopete::Global::Properties *dummy = Kopete::Global::Properties::self();
    
    QFETCH(QString, PropertyName);
    QFETCH(QString, Key);
    QFETCH(QString, Label);
    QFETCH(QString, Icon);
    QFETCH(Kopete::PropertyTmpl::PropertyOption, Option);
    QVERIFY(dummy->isRegistered(PropertyName));
    Kopete::PropertyTmpl temp = dummy->tmpl(PropertyName);
    QCOMPARE(temp.key(), Key);
    QCOMPARE(temp.label(), Label);
    QCOMPARE(temp.icon(), Icon);
    QCOMPARE(temp.options(), Option);    
}

void GlobalPropertiesTest::testCustomProperties()
{
    Kopete::Global::Properties *A = Kopete::Global::Properties::self();
    
    A->isAlwaysVisible();
    const QString isAlwaysVisible = QStringLiteral("isAlwaysVisible");
    QVERIFY(A->isRegistered(isAlwaysVisible));
    Kopete::PropertyTmpl temp = A->tmpl(isAlwaysVisible);
    QCOMPARE(temp.key(), isAlwaysVisible);
    QCOMPARE(temp.label(), QStringLiteral("Shown even if offline"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);    

    A->statusTitle();
    const QString statusTitle = QStringLiteral("statusTitle");
    QVERIFY(A->isRegistered(statusTitle));
    temp = A->tmpl(statusTitle);
    QCOMPARE(temp.key(), statusTitle);
    QCOMPARE(temp.label(), QStringLiteral("Status Title"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::NoProperty);


    Kopete::PropertyTmpl::Map map = A->templateMap();
    Kopete::PropertyTmpl test = map[isAlwaysVisible];
    
    QVERIFY(A->isRegistered(isAlwaysVisible));
    QCOMPARE(test.key(), isAlwaysVisible);
    QCOMPARE(test.label(), QStringLiteral("Shown even if offline"));
    QCOMPARE(test.icon(), QStringLiteral());
    QCOMPARE(test.options(), Kopete::PropertyTmpl::PersistentProperty);
}

QTEST_MAIN(GlobalPropertiesTest)
#include "kopeteglobaltest.moc"
