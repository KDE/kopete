/*
    Tests for Kopete Contact

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

#include <QObject>
#include <QtTest>

Q_DECLARE_METATYPE(Kopete::Contact::NameType);

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

class ContactTest : public QObject
{
    Q_OBJECT
private slots:
    void testContactCreation();
    void testContactCreation_data();
    void testContactStatus();
    void testContactStatus_data();
};

void ContactTest::testContactCreation_data()
{
    QTest::addColumn<Kopete::Contact::NameType>("TypeInEnum");
    QTest::addColumn<QString>("TypeInString");

    QTest::newRow("nickName") << Kopete::Contact::NickName << QStringLiteral("nickName");
    QTest::newRow("formattedName") << Kopete::Contact::FormattedName << QStringLiteral("formattedName");
    QTest::newRow("contactId") << Kopete::Contact::ContactId << QStringLiteral("contactId");
    QTest::newRow("customName") << Kopete::Contact::CustomName << QStringLiteral("customName");
}

void ContactTest::testContactCreation()
{
    Kopete::MetaContact *parentMetaContact = new Kopete::MetaContact();
    DummyProtocol *dummyProtocol = new DummyProtocol();
    DummyAccount *dummyAccount = new DummyAccount(dummyProtocol);
    const QString contactId = QStringLiteral("ContactId");
    const QString icon = QStringLiteral("Icon");
    DummyContact *testContact = new DummyContact(dummyAccount, contactId, parentMetaContact, icon);
    // test conversion of nametype to string and string to name 
    QFETCH(Kopete::Contact::NameType, TypeInEnum);
    QFETCH(QString, TypeInString);
    QCOMPARE(testContact->nameTypeToString(TypeInEnum), TypeInString);
    QCOMPARE(testContact->nameTypeFromString(TypeInString), TypeInEnum);
}

void ContactTest::testContactStatus_data()
{
    QTest::addColumn<QString>("Title");
    QTest::addColumn<QString>("Message");
    QTest::addColumn<int>("SpyCount");

    QTest::newRow("EmptyMessage") << QString("") << QString("") << 0;
    QTest::newRow("FullMessage") << QStringLiteral("Hello fromm the server") << QStringLiteral("This is a test") << 1;
}

void ContactTest::testContactStatus()
{
    Kopete::MetaContact *parentMetaContact = new Kopete::MetaContact();
    DummyProtocol *dummyProtocol = new DummyProtocol();
    DummyAccount *dummyAccount = new DummyAccount(dummyProtocol);
    const QString contactId = QStringLiteral("ContactId");
    const QString icon = QStringLiteral("Icon");
    DummyContact *testContact = new DummyContact(dummyAccount, contactId, parentMetaContact, icon);
    // test Status Message 
    QFETCH(QString, Title);
    QFETCH(QString, Message);
    QFETCH(int, SpyCount);
    Kopete::StatusMessage testStatus = Kopete::StatusMessage(Title, Message);
    QSignalSpy spy(testContact, &Kopete::Contact::statusMessageChanged);
    testContact->setStatusMessage(testStatus);
    QVERIFY(spy.isValid());
    QCOMPARE(spy.count(), SpyCount);
    Kopete::StatusMessage expected = testContact->statusMessage();
    QCOMPARE(testStatus.title(), expected.title());
    QCOMPARE(testStatus.message(), expected.message());
}

QTEST_MAIN(ContactTest)
#include "kopetecontacttest.moc"
