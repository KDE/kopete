/*
    Tests for Kopete::OnlineStatusManager

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

class OnlineStatusManagerTest : public QObject
{
    Q_OBJECT
private:
    DummyProtocol *protocol = new DummyProtocol();
private slots:
    void testManagerSelf();
    void testPersistance(); 
};

void OnlineStatusManagerTest::testManagerSelf()
{
    Kopete::OnlineStatusManager *osm = Kopete::OnlineStatusManager::self();
    QStringList list;
    list.append(QStringLiteral("icon 1"));
    list.append(QStringLiteral("icon 2"));
    list.append(QStringLiteral("icon 3"));
    QString description = QStringLiteral("Description");
    QString caption = QStringLiteral("Caption");
    unsigned int uzero = 0;
    Kopete::OnlineStatus::StatusType status = Kopete::OnlineStatus::Online;
    Kopete::OnlineStatusManager::Options option = Kopete::OnlineStatusManager::HasStatusMessage;
    Kopete::OnlineStatusManager::Categories category = Kopete::OnlineStatusManager::Offline;
    Kopete::OnlineStatus os(status, uzero, protocol, uzero, list, description, caption, category, option);
    // It should be automatically registered
    QList<Kopete::OnlineStatus> statusList = osm->registeredStatusList(this->protocol);
    QVERIFY(statusList.contains(os));
    Kopete::OnlineStatus expected = osm->onlineStatus(this->protocol, category);
    QCOMPARE(expected.description(), description);
    QCOMPARE(expected.internalStatus(), uzero);
}
// This test is supposed to check if the same intance is returned
void OnlineStatusManagerTest::testPersistance()
{
    Kopete::OnlineStatusManager *osm = Kopete::OnlineStatusManager::self();
    Kopete::OnlineStatus::StatusType status = Kopete::OnlineStatus::Online;
    Kopete::OnlineStatusManager::Options option = Kopete::OnlineStatusManager::HasStatusMessage;
    Kopete::OnlineStatusManager::Categories category = Kopete::OnlineStatusManager::Offline;
    Kopete::OnlineStatus expected = osm->onlineStatus(this->protocol, category);
    QStringList list;
    unsigned int uzero = 0;
    list.append(QStringLiteral("icon 1"));
    list.append(QStringLiteral("icon 2"));
    list.append(QStringLiteral("icon 3"));
    QString description = QStringLiteral("Description");
    QString caption = QStringLiteral("Caption");
    QCOMPARE(expected.status(), status);
    QCOMPARE(expected.description(), description);
    QCOMPARE(expected.internalStatus(), uzero);
    QCOMPARE(expected.weight(), uzero);
    QCOMPARE(expected.caption(), caption);
    QCOMPARE(expected.options(), option);
    QCOMPARE(expected.categories(), category);
    
    for(const QString &str : list)
    {
        QVERIFY(expected.overlayIcons().contains(str));
    }

    QSignalSpy spy(osm, &Kopete::OnlineStatusManager::iconsChanged);
    QVERIFY(spy.isValid());
}

QTEST_MAIN(OnlineStatusManagerTest)
#include "kopeteonlinestatusmanagertest.moc"
