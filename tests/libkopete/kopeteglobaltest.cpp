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

class GlobalPropertiesTest : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultProperties();
    void testCustomProperties();
};

void GlobalPropertiesTest::testDefaultProperties()
{
    Kopete::Global::Properties *A = Kopete::Global::Properties::self();
    
    const QString fromattedName = QStringLiteral("FormattedName");
    QVERIFY(A->isRegistered(fromattedName));
    Kopete::PropertyTmpl temp = A->tmpl(fromattedName);
    QCOMPARE(temp.key(), fromattedName);
    QCOMPARE(temp.label(), QStringLiteral("Full Name"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::NoProperty);
    
    const QString idleTime = QStringLiteral("idleTime");
    QVERIFY(A->isRegistered(idleTime));
    temp = A->tmpl(idleTime);
    QCOMPARE(temp.key(), idleTime);
    QCOMPARE(temp.label(), QStringLiteral("Idle Time"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::NoProperty);

    const QString onlineSince = QStringLiteral("onlineSince");
    QVERIFY(A->isRegistered(onlineSince));
    temp = A->tmpl(onlineSince);
    QCOMPARE(temp.key(), onlineSince);
    QCOMPARE(temp.label(), QStringLiteral("Online Since"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::NoProperty);

    const QString lastSeen = QStringLiteral("lastSeen");
    QVERIFY(A->isRegistered(lastSeen));
    temp = A->tmpl(lastSeen);
    QCOMPARE(temp.key(), lastSeen);
    QCOMPARE(temp.label(), QStringLiteral("Last Seen"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);

    const QString statusMessage = QStringLiteral("statusMessage");
    QVERIFY(A->isRegistered(statusMessage));
    temp = A->tmpl(statusMessage);
    QCOMPARE(temp.key(), statusMessage);
    QCOMPARE(temp.label(), QStringLiteral("Status Message"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::NoProperty);

    const QString firstName = QStringLiteral("firstName");
    QVERIFY(A->isRegistered(firstName));
    temp = A->tmpl(firstName);
    QCOMPARE(temp.key(), firstName);
    QCOMPARE(temp.label(), QStringLiteral("First Name"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);

    const QString lastName = QStringLiteral("lastName");
    QVERIFY(A->isRegistered(lastName));
    temp = A->tmpl(lastName);
    QCOMPARE(temp.key(), lastName);
    QCOMPARE(temp.label(), QStringLiteral("Last Name"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);    

    const QString privatePhoneNumber = QStringLiteral("privatePhoneNumber");
    QVERIFY(A->isRegistered(privatePhoneNumber));
    temp = A->tmpl(privatePhoneNumber);
    QCOMPARE(temp.key(), privatePhoneNumber);
    QCOMPARE(temp.label(), QStringLiteral("Private Phone"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);        
    
    const QString privateMobilePhoneNumber = QStringLiteral("privateMobilePhoneNumber");
    QVERIFY(A->isRegistered(privateMobilePhoneNumber));
    temp = A->tmpl(privateMobilePhoneNumber);
    QCOMPARE(temp.key(), privateMobilePhoneNumber);
    QCOMPARE(temp.label(), QStringLiteral("Private Mobile Phone"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);       

    const QString workPhoneNumber = QStringLiteral("workPhoneNumber");
    QVERIFY(A->isRegistered(workPhoneNumber));
    temp = A->tmpl(workPhoneNumber);
    QCOMPARE(temp.key(), workPhoneNumber);
    QCOMPARE(temp.label(), QStringLiteral("Work Phone"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);     

    const QString workMobilePhoneNumber = QStringLiteral("workMobilePhoneNumber");
    QVERIFY(A->isRegistered(workMobilePhoneNumber));
    temp = A->tmpl(workMobilePhoneNumber);
    QCOMPARE(temp.key(), workMobilePhoneNumber);
    QCOMPARE(temp.label(), QStringLiteral("Work Mobile Phone"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);     

    const QString emailAddress = QStringLiteral("emailAddress");
    QVERIFY(A->isRegistered(emailAddress));
    temp = A->tmpl(emailAddress);
    QCOMPARE(temp.key(), emailAddress);
    QCOMPARE(temp.label(), QStringLiteral("Email Address"));
    QCOMPARE(temp.icon(), QStringLiteral("mail"));
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);        

    const QString nickName = QStringLiteral("nickName");
    QVERIFY(A->isRegistered(nickName));
    temp = A->tmpl(nickName);
    QCOMPARE(temp.key(), nickName);
    QCOMPARE(temp.label(), QStringLiteral("Nick Name"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);        

    const QString customName = QStringLiteral("customName");
    QVERIFY(A->isRegistered(customName));
    temp = A->tmpl(customName);
    QCOMPARE(temp.key(), customName);
    QCOMPARE(temp.label(), QStringLiteral("Custom Name"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);            

    const QString photo QStringLiteral("photo");
    QVERIFY(A->isRegistered(photo));
    temp = A->tmpl(photo);
    QCOMPARE(temp.key(), photo);
    QCOMPARE(temp.label(), QStringLiteral("Photo"));
    QCOMPARE(temp.icon(), QStringLiteral());
    QCOMPARE(temp.options(), Kopete::PropertyTmpl::PersistentProperty);
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
