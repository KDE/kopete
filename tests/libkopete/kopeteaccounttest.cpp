/*
    Tests for Kopete Account

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
class AccountTest : public QObject
{
    Q_OBJECT
private slots:
    void testAccountCreation();
};

void AccountTest::testAccountCreation()
{
    Kopete::MetaContact *parentMetaContact = new Kopete::MetaContact();
    DummyProtocol *dummyProtocol = new DummyProtocol();
    DummyAccount *dummyAccount = new DummyAccount(dummyProtocol);
    Kopete::StatusMessage reason(QStringLiteral("I want to test if it suspends the account."));
    const QString contactId = QStringLiteral("ContactId");
    const QString icon = QStringLiteral("Icon");
    DummyContact *dummyContact = new DummyContact(dummyAccount, contactId, parentMetaContact, icon);
    dummyAccount->setSelf(dummyContact);
    QVERIFY(dummyAccount->suspend(reason));
    QVERIFY(dummyAccount->resume());
    QVERIFY(!dummyAccount->hasCustomStatusMenu());
    const QString contactId_test = QStringLiteral("Contact Id for test add");
    bool success = dummyAccount->addContact(contactId_test, parentMetaContact, Kopete::Account::Temporary);
    QVERIFY(success);
    Kopete::MetaContact *newMetaContact = new Kopete::MetaContact();
    const QString contactIdTest = QStringLiteral("Contact ID for Test new ");
    DummyContact *contactToAdd = new DummyContact(dummyAccount, contactIdTest, newMetaContact, icon);
    // Here it register itself with this constructor call
    bool status = dummyAccount->registerContact(contactToAdd);
    QVERIFY(!status);   // This should be false because this contact already exists because of the constructor
                        // call to create a new contact where it also registers itself

    const QHash<QString, Kopete::Contact *> contactList = dummyAccount->contacts();
    Kopete::Contact *expected = contactList[contactIdTest];
    QCOMPARE(expected->contactId(), contactIdTest);
    QCOMPARE(dummyAccount->accountLabel(), QStringLiteral("Dummy Account"));
    const QString accountLabel = QStringLiteral("Account Label");
    dummyAccount->settingAccountLabel(accountLabel);
    QCOMPARE(dummyAccount->accountLabel(), accountLabel);
    const QString contactBlock = QStringLiteral("Block It");
    QVERIFY(!dummyAccount->isBlocked(contactBlock));
    dummyAccount->block(contactBlock);
    QVERIFY(dummyAccount->isBlocked(contactBlock));
    dummyAccount->unblock(contactBlock);
    QVERIFY(!dummyAccount->isBlocked(contactBlock));
}

QTEST_MAIN(AccountTest)
#include "kopeteaccounttest.moc"
