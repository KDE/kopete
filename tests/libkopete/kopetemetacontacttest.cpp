/*
    Tests for Kopete::MetaContact

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


#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetechatsession.h"

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

class DummyAccount : public Kopete::Account
{
    Q_OBJECT
public:
    DummyAccount(DummyProtocol *dummyProtocol):Kopete::Account(dummyProtocol, QStringLiteral("Dummy Account"))
    {

    }    
protected:
    bool createContact(const QString &contactId, Kopete::MetaContact *parentContact)
    {
        Q_UNUSED(contactId);
        Q_UNUSED(parentContact);
        return true;
    }
public Q_SLOTS:
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
    void setSelf(Kopete::Contact *contact)
    {
        setMyself(contact);
    }
    void settingAccountLabel(const QString &label)
    {
        setAccountLabel(label);
    }
};

class DummyContact : public Kopete::Contact
{
public:
    DummyContact(Kopete::Account *account, const QString &contactId, Kopete::MetaContact *parent, const QString &icon) : Kopete::Contact(account, contactId, parent, icon)
    {

    };
    Kopete::ChatSession *manager(CanCreateFlags canCreate = CannotCreate)
    {
        Q_UNUSED(canCreate);
        return nullptr;
    }
};

class MetaContactTest : public QObject
{
    Q_OBJECT
private slots:
    void testUUID();
    void testUUID_data();
    void testSignalConnection();
    void testContactList_data();
    void testContactList();
};

void MetaContactTest::testUUID_data()
{
    QTest::addColumn<QString>("test");
    QTest::addColumn<QString>("expected");

    QTest::newRow("Ideal Case") << QStringLiteral("UUid") << QUuid(QStringLiteral("UUid")).toString();
    QTest::newRow("Empty Case") << QString() << QUuid(QString()).toString();
}

void MetaContactTest::testUUID()
{
    Kopete::MetaContact *dummy = new Kopete::MetaContact();
    QFETCH(QString, test);
    QFETCH(QString, expected);
    dummy->setMetaContactId(QUuid(test));
    QCOMPARE(dummy->metaContactId().toString(), expected);
    delete dummy;
}

void MetaContactTest::testSignalConnection()
{
    Kopete::MetaContact *dummy = new Kopete::MetaContact();
    QSignalSpy spy(dummy, &Kopete::MetaContact::persistentDataChanged);
    emit dummy->pluginDataChanged();
    QCOMPARE(spy.count(), 1);
    emit dummy->useCustomIconChanged(true);
    QCOMPARE(spy.count(), 2);
    emit dummy->displayNameChanged(QString(), QString());
    QCOMPARE(spy.count(), 3);
    emit dummy->contactAdded(nullptr);
    QCOMPARE(spy.count(), 4);
    emit dummy->contactRemoved(nullptr);
    QCOMPARE(spy.count(), 5);
    emit dummy->iconChanged(Kopete::ContactListElement::Open, QString());
    QCOMPARE(spy.count(), 6);
    emit dummy->movedToGroup(nullptr, nullptr, nullptr);
    QCOMPARE(spy.count(), 7);
    emit dummy->removedFromGroup(nullptr, nullptr);
    QCOMPARE(spy.count(), 8);
    emit dummy->addedToGroup(nullptr, nullptr);
    QCOMPARE(spy.count(), 9);
    delete dummy;
}

void MetaContactTest::testContactList_data()
{
    QTest::addColumn<QString>("contactId");
    QTest::addColumn<QString>("icon");

    QTest::newRow("Ideal Case") << QStringLiteral("ContactId") << QStringLiteral("Icon");
    QTest::newRow("Empty Case") << QString() << QString();
}


void MetaContactTest::testContactList()
{
    QFETCH(QString, contactId);
    QFETCH(QString, icon);
    
    Kopete::MetaContact *parentMetaContact = new Kopete::MetaContact();
    DummyProtocol *dummyProtocol = new DummyProtocol();
    DummyAccount *dummyAccount = new DummyAccount(dummyProtocol);
    DummyContact *dummyContact = new DummyContact(dummyAccount, contactId, parentMetaContact, icon);
    Kopete::MetaContact *dummy = new Kopete::MetaContact();
    QSignalSpy spy(dummy, &Kopete::MetaContact::contactAdded);
    dummy->addContact(dummyContact);
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), 1);
    QSignalSpy spy1(dummy, &Kopete::MetaContact::contactRemoved);
    dummy->removeContact(dummyContact);
    QVERIFY(spy1.isValid());
    QCOMPARE(spy1.count(), 1);
    delete parentMetaContact;
    delete dummyProtocol;
    delete dummy;
}


QTEST_MAIN(MetaContactTest)
#include "kopetemetacontacttest.moc"
