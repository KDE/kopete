/*
    Tests for Kopete Message

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
class MessageTest : public QObject
{
    Q_OBJECT
private slots:
    void testMessageCreation();
    void testMessageCreation_data();
};

void MessageTest::testMessageCreation_data()
{
    QTest::addColumn<QString>("escapedTest");
    QTest::addColumn<QString>("escapedExpected");
    QTest::addColumn<QString>("unescapedTest");
    QTest::addColumn<QString>("unescapedExpected");
    QTest::addColumn<QString>("toContactId");
    QTest::addColumn<QString>("fromContactId");
    QTest::addColumn<QString>("icon");

    QTest::newRow("Ideal Case") << QStringLiteral("Hello\nThere") << QStringLiteral("Hello<br />There") << QStringLiteral("Hello<br />There") << QStringLiteral("Hello\nThere") << QStringLiteral("Roman") << QStringLiteral("Gaurav") << QStringLiteral("Icon");
    QTest::newRow("Negative Case") << QStringLiteral("Hello\tThere") << QStringLiteral("Hello&nbsp;&nbsp;&nbsp;&nbsp;There") << QStringLiteral("Hello&nbsp;&nbsp;&nbsp;&nbsp;There") << QStringLiteral("Hello\tThere") << QStringLiteral("Roman") << QStringLiteral("Gaurav") << QStringLiteral("Icon");
    QTest::newRow("Empty Case") << QString() << QString() << QString() << QString() << QString() << QString() << QString();
}

void MessageTest::testMessageCreation()
{
    QFETCH(QString, fromContactId);
    QFETCH(QString, toContactId);
    QFETCH(QString, icon);
    QFETCH(QString, escapedTest);
    QFETCH(QString, unescapedTest);
    QFETCH(QString, escapedExpected);
    QFETCH(QString, unescapedExpected);
    
    Kopete::MetaContact *parentMetaContact = new Kopete::MetaContact();
    DummyProtocol *dummyProtocol = new DummyProtocol();
    DummyAccount *dummyAccount = new DummyAccount(dummyProtocol);
    DummyContact *fromContact = new DummyContact(dummyAccount, fromContactId, parentMetaContact, icon);
    DummyContact *toContact = new DummyContact(dummyAccount, toContactId, parentMetaContact, icon);
    Kopete::Message msg(fromContact, toContact);
    QCOMPARE(escapedExpected, msg.escape(escapedTest));
    QCOMPARE(unescapedExpected, msg.unescape(unescapedTest));
    delete dummyAccount; // Contact gets automatically deleted
    delete dummyProtocol;
    delete parentMetaContact;
}

QTEST_MAIN(MessageTest)
#include "kopetemessagetest.moc"
