/*
    Tests for Kopete::Identity

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

#include "kopeteidentity.h"
#include "kopeteaccount.h"
#include "kopeteonlinestatus.h"
#include "kopetestatusmessage.h"
#include "kopeteprotocol.h"
#include <QObject>
#include <QtTest>

class DummyProtocol : public Kopete::Protocol
{
public:
    DummyProtocol() : Kopete::Protocol(nullptr, false)
    {

    }
    Kopete::Account *createNewAccount(const QString &accountId)
    {
        Q_UNUSED(accountId)
        return nullptr;
    }
    AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account)
    {
        Q_UNUSED(parent)
        Q_UNUSED(account)
        return nullptr;
    }
    KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent)
    {
        Q_UNUSED(account)
        Q_UNUSED(parent)
        return nullptr;
    }
    void setCapability(Protocol::Capabilities capabilities)
    {
        this->setCapabilities(capabilities);
    }
};  

class TestAccount : public Kopete::Account
{
public:
    TestAccount(const QString &param) : Kopete::Account(new DummyProtocol(), param)
    {

    }
    bool createContact(const QString &contactId, Kopete::MetaContact *parentContact)
    {
        Q_UNUSED(contactId);
        Q_UNUSED(parentContact);
        return false;
    }
    void connect(const Kopete::OnlineStatus &initialStatus = Kopete::OnlineStatus())
    {
        Q_UNUSED(initialStatus);
    }
    void disconnect()
    {

    }
    void setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(), const OnlineStatusOptions &options = None)
    {
        Q_UNUSED(status);
        Q_UNUSED(reason);
        Q_UNUSED(options);
    }
    void setStatusMessage(const Kopete::StatusMessage &statusMessage)
    {
        Q_UNUSED(statusMessage);
    }
};

class IdentityTest : public QObject
{
    Q_OBJECT
private slots:
    void testIdentity(); 
    void testIdentityAccounts();   
};

void IdentityTest::testIdentity()
{
    const QString Id = QStringLiteral("Id");
    const QString Label = QStringLiteral("Label");
    Kopete::Identity *A = new Kopete::Identity(Id, Label);
    // check if unique connection exists
    QCOMPARE(A->id(), Id);
    QCOMPARE(A->label(), Label);
    QSignalSpy spy(A, &Kopete::Identity::identityChanged);
    QVERIFY(spy.isValid());
    const QString newLabel = QStringLiteral("newLabel");
    A->setLabel(newLabel);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(A->label(), newLabel);
    A->setLabel(newLabel);      // signal is emmitted even if value is same
    QCOMPARE(spy.count(), 2);
    QVERIFY(!A->excludeConnect()); // check if it still return false
    Kopete::OnlineStatus::StatusType test = A->onlineStatus();
    Kopete::OnlineStatus::StatusType expected = Kopete::OnlineStatus::Unknown;
    QCOMPARE(test, expected);
    QVERIFY(A->statusMessage().isEmpty());
    QString tt = A->toolTip();
    QVERIFY(tt.startsWith("<qt>"));
    QVERIFY(tt.endsWith("</qt>"));
    QCOMPARE(A->customIcon(), QStringLiteral("user-identity"));
    QVERIFY(A->accounts().empty());
    QSignalSpy spyToolTip(A, &Kopete::Identity::toolTipChanged);
    A->updateOnlineStatus();
    QCOMPARE(spyToolTip.count(), 1);
    A->updateOnlineStatus();
    QCOMPARE(spyToolTip.count(), 2);
}

void IdentityTest::testIdentityAccounts()
{
    TestAccount *dummyAccount = new TestAccount(QStringLiteral("dummyAccount"));
    const QString Id = QStringLiteral("Id");
    const QString Label = QStringLiteral("Label");
    Kopete::Identity *A = new Kopete::Identity(Id, Label);
    QSignalSpy spyIdentityChanged(A, &Kopete::Identity::identityChanged);
    QSignalSpy spyToolTip(A, &Kopete::Identity::toolTipChanged);
    A->addAccount(dummyAccount); 
    QCOMPARE(spyIdentityChanged.count(), 1);
    QCOMPARE(spyToolTip.count(), 2);
    QList<Kopete::Account *> list = A->accounts();
    QVERIFY(list.indexOf(dummyAccount) != 1);
    A->removeAccount(dummyAccount);
    QCOMPARE(spyIdentityChanged.count(), 2);
    QCOMPARE(spyToolTip.count(), 4);
}

QTEST_MAIN(IdentityTest)
#include "kopeteidentitytest.moc"
