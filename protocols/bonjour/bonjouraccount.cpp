/*
    bonjouraccount.cpp - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "bonjouraccount.h"

#include <QAction>
#include <kdebug.h>
#include <KLocalizedString>
#include <kactionmenu.h>

#include <kmessagebox.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetedeletecontacttask.h"
#include "kopeteuiglobal.h"

#include "bonjourcontact.h"
#include "bonjourprotocol.h"
#include "bonjourcontactconnection.h"

static const char AvailabilityStatusAvailId[] = "avail";
static const char AvailabilityStatusAwayId[] = "away";
// TODO: add this status to the account
// static const char AvailabilityStatusDnDId[] =   "dnd";

BonjourAccount::BonjourAccount(BonjourProtocol *parent, const QString &accountID)
    : Kopete::Account(parent, accountID)
    , username()
    , firstName()
    , emailAddress()
    , lastName()
    , service(NULL)
    , localServer(NULL)
    , listeningPort(0)
    , bonjourGroup(NULL)
    , browser(NULL)
    , unknownConnections()
{
    // Init the myself contact
    setMyself(new BonjourContact(this, accountId(), Kopete::ContactList::self()->myself()));
    myself()->setOnlineStatus(BonjourProtocol::protocol()->bonjourOffline);

    // All Contacts Go To The Bonjour Group
    bonjourGroup = Kopete::ContactList::self()->findGroup(QStringLiteral("Bonjour"));

    // Clean out Contacts from last time when kopete starts up
    wipeOutAllContacts();

    parseConfig();
}

void BonjourAccount::parseConfig()
{
    username = configGroup()->readEntry("username").toLocal8Bit();
    firstName = configGroup()->readEntry("firstName").toLocal8Bit();
    lastName = configGroup()->readEntry("lastName").toLocal8Bit();
    emailAddress = configGroup()->readEntry("emailAddress").toLocal8Bit();
}

BonjourAccount::~BonjourAccount()
{
    if (isConnected()) {
        disconnect();
    }
}

#if 0
KActionMenu *BonjourAccount::actionMenu()
{
    KActionMenu *mActionMenu = Kopete::Account::actionMenu();

    return mActionMenu;
}

#endif
bool BonjourAccount::createContact(const QString &contactId, Kopete::MetaContact *parentContact)
{
    BonjourContact *newContact = new BonjourContact(this, contactId, parentContact);
    return newContact != 0L;
}

void BonjourAccount::setAway(bool away, const QString & /* reason */)
{
    if (away) {
        slotGoAway();
    } else {
        slotGoOnline();
    }
}

void BonjourAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions & /*options*/)
{
    if (status.status() == Kopete::OnlineStatus::Online
        && myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline) {
        slotGoOnline();
    } else if (status.status() == Kopete::OnlineStatus::Online
               && (myself()->onlineStatus().status() == Kopete::OnlineStatus::Away
                   || myself()->onlineStatus().status() == Kopete::OnlineStatus::Away)) {
        setAway(false, reason.message());
    } else if (status.status() == Kopete::OnlineStatus::Offline) {
        slotGoOffline();
    } else if (status.status() == Kopete::OnlineStatus::Away) {
        slotGoAway(/* reason */);
    }
}

void BonjourAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
    setOnlineStatus(myself()->onlineStatus(), statusMessage, Kopete::Account::KeepSpecialFlags);
}

// This Function Starts a new Local Server
// It runs on local port listeningPort
// Make Sure IP Tables lets this port through!!
bool BonjourAccount::startLocalServer()
{
    int port = 5298;

    localServer = new QTcpServer();

    while (port < 5305) {               // No of Attempts
        if (localServer->listen(QHostAddress::Any, port)) {
            QObject::connect(localServer, SIGNAL(newConnection()),
                             this, SLOT(newIncomingConnection()));
            listeningPort = port;
            break;
        } else {
            port++;
        }
    }

    qDebug()<<"Listening On Port: "<<listeningPort;

    return localServer->isListening();
}

void BonjourAccount::startBrowse()
{
    // Delete All Contacts Before we start looking for new ones
    wipeOutAllContacts();

    browser = new KDNSSD::ServiceBrowser(QStringLiteral("_presence._tcp"));

    QObject::connect(browser, SIGNAL(serviceAdded(KDNSSD::RemoteService::Ptr)),
                     this, SLOT(comingOnline(KDNSSD::RemoteService::Ptr)));
    QObject::connect(browser, SIGNAL(serviceRemoved(KDNSSD::RemoteService::Ptr)),
                     this, SLOT(goingOffline(KDNSSD::RemoteService::Ptr)));

    qDebug()<<"Starting Browser";
    browser->startBrowse();
}

void BonjourAccount::startPublish()
{
    if (!username.contains('@')) {
        username.append("@");
        username.append(KDNSSD::ServiceBrowser::getLocalHostName().toUtf8());
    }

    service = new KDNSSD::PublicService(username, QStringLiteral("_presence._tcp"), listeningPort);

    QMap <QString, QByteArray> map;
    map.insert(QStringLiteral("1st"), firstName);
    map.insert(QStringLiteral("email"), emailAddress);
    map.insert(QStringLiteral("last"), lastName);
    map.insert(QStringLiteral("node"), "kopete");
    map.insert(QStringLiteral("port.p2pj"), QByteArray::number(listeningPort));     // This Number Actually Ignored
    map.insert(QStringLiteral("status"), AvailabilityStatusAvailId);
    map.insert(QStringLiteral("txtvers"), "1");
    map.insert(QStringLiteral("vc"), "!");
    map.insert(QStringLiteral("ver"), "0.0.1");

    service->setTextData(map);

    qDebug()<<"Starting Publish";
    QObject::connect(service, SIGNAL(published(bool)), this, SLOT(published(bool)));
    service->publishAsync();
}

void BonjourAccount::published(bool success)
{
    // If we have successfully published, great :)
    if (success) {
        qDebug()<<"Publish Successful";
    } else {
        qDebug()<<"Publish Failed";
        disconnect();
        KMessageBox::error(Kopete::UI::Global::mainWidget(),
                           i18n("Unable to publish Bonjour service. Currently the Bonjour plugin only works with Avahi."));
    }
}

void BonjourAccount::connect(const Kopete::OnlineStatus & /* initialStatus */)
{
    if (username.isEmpty()) {
        username = accountId().toUtf8();
    }

    if (KDNSSD::ServiceBrowser::isAvailable() != KDNSSD::ServiceBrowser::Working) {
        KMessageBox::error(Kopete::UI::Global::mainWidget(),
                           i18n("Unable to connect to the local mDNS server. Please ensure the Avahi daemon is running."));
        return;
    }

    if (!startLocalServer()) {
        return;
    }

    startPublish();

    myself()->setOnlineStatus(BonjourProtocol::protocol()->bonjourOnline);

    startBrowse();
}

void BonjourAccount::comingOnline(KDNSSD::RemoteService::Ptr pointer)
{
    if (!pointer->resolve()) {
        qDebug()<<"Unable to Resolve! Dumping Contact";
    }

    qDebug()<<"Coming Online:"<<pointer->serviceName();

    if (pointer->serviceName() == username) {       // Don't Add Ourselves
        return;
    }

    QMap <QString, QByteArray> map = pointer->textData();
    QString cfirst = QString::fromLocal8Bit(map[QStringLiteral("1st")]);
    QString clast = QString::fromLocal8Bit(map[QStringLiteral("last")]);

    QString display;
    if (!cfirst.isEmpty() && !clast.isEmpty()) {
        display = cfirst + ' ' + clast;
    } else if (!cfirst.isEmpty()) {
        display = cfirst;
    } else if (!clast.isEmpty()) {
        display = clast;
    } else {
        display = pointer->serviceName().split('@')[0];
    }

    QString hostName = pointer->hostName();
    qDebug()<<"Hostname is:"<<hostName;
    if (!hostName.isEmpty()) {
        QHostAddress hostAddress = KDNSSD::ServiceBrowser::resolveHostName(hostName);
        qDebug()<<"Host Address is:"<<hostAddress;

        if (hostAddress != QHostAddress()) {
            Kopete::MetaContact *mc;

            mc = addContact(pointer->serviceName(), display, bonjourGroup);

            BonjourContact *c = (BonjourContact *)mc->findContact(
                protocol()->pluginId(),
                accountId(),
                pointer->serviceName());

            c->setremoteHostName(hostName);
            c->setremoteAddress(hostAddress);
            c->setremotePort(pointer->port());
            c->settextdata(pointer->textData());
            c->setusername(pointer->serviceName());
            c->setOnlineStatus(Kopete::OnlineStatus::Online);
        }
    }
}

void BonjourAccount::goingOffline(KDNSSD::RemoteService::Ptr pointer)
{
    pointer->resolve();

    // In case we have lost connection, this may return NULL
    Kopete::Contact *c = contacts().value(pointer->serviceName());

    if (c) {
        c->setOnlineStatus(Kopete::OnlineStatus::Offline);
    }
}

void BonjourAccount::wipeOutContact(Kopete::Contact *c)
{
    if (c == myself() || c == NULL) {
        return;
    }

    Kopete::MetaContact *mc = c->metaContact();

    c->setOnlineStatus(Kopete::OnlineStatus::Offline);
    mc->removeContact(c);

    // FIXME: DeleteContact task should be extended and used
    c->deleteLater();

    if (mc->contacts().isEmpty()) {
        Kopete::ContactList::self()->removeMetaContact(mc);
    }
}

void BonjourAccount::wipeOutAllContacts()
{
    QList <Kopete::Contact *> list = contacts().values();

    for (QList <Kopete::Contact *>::Iterator i = list.begin(); i != list.end(); i++) {
        wipeOutContact(*i);
    }
}

void BonjourAccount::disconnect()
{
    wipeOutAllContacts();

    delete browser;
    browser = NULL;

    if (localServer) {
        localServer->close();
        delete localServer;
        localServer = NULL;
    }

    listeningPort = 0;

    if (service) {
        service->stop();
        delete service;
        service = NULL;
    }

    myself()->setOnlineStatus(BonjourProtocol::protocol()->bonjourOffline);
}

void BonjourAccount::slotGoOnline()
{
    qDebug();

    if (!isConnected()) {
        connect();
    } else {
        if (service) {
            QMap <QString, QByteArray> map = service->textData();
            map[QStringLiteral("status")] = AvailabilityStatusAvailId;
            service->setTextData(map);
        }
        myself()->setOnlineStatus(BonjourProtocol::protocol()->bonjourOnline);
    }
}

void BonjourAccount::slotGoAway()
{
    qDebug();

    if (!isConnected()) {
        connect();
    }

    if (service) {
        QMap <QString, QByteArray> map = service->textData();
        map[QStringLiteral("status")] = AvailabilityStatusAwayId; // "dnd" would be another option here
        service->setTextData(map);
    }

    myself()->setOnlineStatus(BonjourProtocol::protocol()->bonjourAway);
}

void BonjourAccount::slotGoOffline()
{
    qDebug();

    if (isConnected()) {
        disconnect();
    }
}

void BonjourAccount::setusername(const QByteArray &n_username)
{
    username = n_username;
}

void BonjourAccount::setfirstName(const QByteArray &n_firstName)
{
    firstName = n_firstName;
}

void BonjourAccount::setlastName(const QByteArray &n_lastName)
{
    lastName = n_lastName;
}

void BonjourAccount::setemailAddress(const QByteArray &n_emailAddress)
{
    emailAddress = n_emailAddress;
}

const QByteArray BonjourAccount::getusername() const
{
    return username;
}

const QByteArray BonjourAccount::getfirstName() const
{
    return firstName;
}

const QByteArray BonjourAccount::getlastName() const
{
    return lastName;
}

const QByteArray BonjourAccount::getemailAddress() const
{
    return emailAddress;
}

QList <BonjourContact *> BonjourAccount::getContactsByAddress(const QHostAddress &addr)
{
    QList <BonjourContact *> list;

    QList <Kopete::Contact *> c = contacts().values();

    for (QList <Kopete::Contact *>::iterator i = c.begin(); i != c.end(); i++) {
        BonjourContact *c = (BonjourContact *)*i;
        if (c->isRemoteAddress(addr)) {
            list<<c;
        }
    }

    return list;
}

void BonjourAccount::newIncomingConnection()
{
    // Get Next Connection
    QTcpSocket *sock = localServer->nextPendingConnection();

    BonjourContactConnection *bcc = new BonjourContactConnection(sock);
    QObject::connect(bcc, SIGNAL(discoveredUserName(BonjourContactConnection *,QString)),
                     this, SLOT(discoveredUserName(BonjourContactConnection *,QString)));
    QObject::connect(bcc, SIGNAL(usernameNotInStream(BonjourContactConnection *)),
                     this, SLOT(usernameNotInStream(BonjourContactConnection *)));

    unknownConnections << bcc;
}

void BonjourAccount::discoveredUserName(BonjourContactConnection *conn, const QString &user)
{
    qDebug()<<"User Making Contact (unverified): "<<user;

    BonjourContact *c;

    if (!(c = verifyUser(conn, user))) {
        qDebug()<<"Ignoring Unverified User: "<<user;
        return;
    }

    qDebug()<<"User Verified: "<<user;

    unknownConnections.removeAll(conn);

    c->setConnection(conn);
}

void BonjourAccount::usernameNotInStream(BonjourContactConnection *conn)
{
    QList <BonjourContact *> list = getContactsByAddress(conn->getHostAddress());

    qDebug()<<"Looking Up Via IP Address"<<conn->getHostAddress()<<list;

    // Set this connection to first user in the list
    if (list.size()) {
        BonjourContact *c = list[0];

        qDebug()<<"Assigned to Contact: "<<c->getusername();

        unknownConnections.removeAll(conn);

        conn->setRemoteAndLocal(c->getusername(), username);
        c->setConnection(conn);
    }
}

BonjourContact *BonjourAccount::verifyUser(BonjourContactConnection *conn, const QString &user)
{
    // First Check the User Exists
    if (!contacts().value(user)) {
        return NULL;
    }

    BonjourContact *c = (BonjourContact *)contacts().value(user);

    if (c->getremoteAddress() != conn->getHostAddress()) {
        return NULL;
    }

    return c;
}
