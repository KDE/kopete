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

#include <QtDBus>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kactionmenu.h>
#include <kmenu.h>
#include <kicon.h>

#include <dnssd/publicservice.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetedeletecontacttask.h"

#include "bonjouraccount.h"
#include "bonjourcontact.h"
#include "bonjourprotocol.h"
#include "bonjourcontactconnection.h"


BonjourAccount::BonjourAccount( BonjourProtocol *parent, const QString& accountID )
: Kopete::Account ( parent, accountID )
{
	// Init the myself contact
	setMyself( new BonjourContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );

	service = NULL;
	localServer = NULL;
	listeningPort = 0;
	browser = NULL;

	// All Contacts Go To The Bonjour Group
	bonjourGroup = Kopete::ContactList::self()->findGroup("Bonjour");

	// Clean out Contacts from last time when kopete starts up
	wipeOutAllContacts();

	parseConfig();
}

void BonjourAccount::parseConfig()
{
	username = configGroup()->readEntry("username").toUtf8();
	firstName = configGroup()->readEntry("firstName").toUtf8();
	lastName = configGroup()->readEntry("lastName").toUtf8();
	emailAddress = configGroup()->readEntry("emailAddress").toUtf8();
}

BonjourAccount::~BonjourAccount()
{
	if (isConnected())
		disconnect();
}

KActionMenu* BonjourAccount::actionMenu()
{
	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	mActionMenu->addSeparator();

	KAction *action;

	action = new KAction (KIcon("bonjour_showvideo"), i18n ("Show my own video..."), mActionMenu );
        //, "actionShowVideo");
	QObject::connect( action, SIGNAL(triggered(bool)), this, SLOT(slotShowVideo()) );
	mActionMenu->addAction(action);
	action->setEnabled( isConnected() );

	return mActionMenu;
}

bool BonjourAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
	BonjourContact* newContact = new BonjourContact( this, contactId, parentContact->displayName(), parentContact );
	return newContact != 0L;
}

void BonjourAccount::setAway( bool away, const QString & /* reason */ )
{
	if ( away )
		slotGoAway();
	else
		slotGoOnline();
}

void BonjourAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason )
{
	if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		slotGoOnline();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
		setAway( false, reason.message() );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		slotGoOffline();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( /* reason */ );
}

void BonjourAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
	Q_UNUSED(statusMessage);
	/* Not used in bonjour */
}

// This Function Starts a new Local Server
// It runs on local port listeningPort
// Make Sure IP Tables lets this port through!!
bool BonjourAccount::startLocalServer()
{
        int port = 5298;

        localServer = new QTcpServer();

        while (port < 5305)             // No of Attempts
                if (localServer->listen(QHostAddress::Any, port)) {
			QObject::connect(localServer, SIGNAL(newConnection()),
					this, SLOT(newIncomingConnection()));
			listeningPort = port;
                        break;
		}
                else
                        port++;

	kDebug()<<"Listening On Port: "<<listeningPort;
        return localServer->isListening();
}

void BonjourAccount::startBrowse()
{
	// Delete All Contacts Before we start looking for new ones
	wipeOutAllContacts();

	browser = new DNSSD::ServiceBrowser("_presence._tcp");
	
	QObject::connect(browser,SIGNAL(serviceAdded(DNSSD::RemoteService::Ptr)),
			this,SLOT(comingOnline(DNSSD::RemoteService::Ptr)));
	QObject::connect(browser,SIGNAL(serviceRemoved(DNSSD::RemoteService::Ptr)),
			this,SLOT(goingOffline(DNSSD::RemoteService::Ptr)));

	kDebug()<<"Starting Browser";
	browser->startBrowse();
}

void BonjourAccount::startPublish()
{
	if (! username.contains('@')) {
		username.append("@");
		username.append(getLocalHostName().toUtf8());
	}

	service = new DNSSD::PublicService(username, "_presence._tcp", listeningPort);

        QMap <QString, QByteArray> map;
        map.insert("1st",  firstName);
        map.insert("email", emailAddress);
        map.insert("last", lastName);
        map.insert("node", "kopete");
        map.insert("port.p2pj", QByteArray::number(listeningPort));	// This Number Actuall Ignored
        map.insert("status", "avail");
        map.insert("txtvers", "1");
        map.insert("vc", "!");
        map.insert("ver", "0.0.1");

        service->setTextData(map);

	kDebug()<<"Starting Publish";
        service->publish();
}

void BonjourAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	if (username.isEmpty())
		username = accountId().toUtf8();

	if (! startLocalServer())
		return;

	startPublish();

	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOnline );

	startBrowse();
}

void BonjourAccount::comingOnline(DNSSD::RemoteService::Ptr pointer)
{
	pointer->resolve();

	kDebug()<<"\nComing Online\n";
	
	if (pointer->serviceName() == username)			// Don't Add Ourselves
		return;

        QMap <QString, QByteArray> map = pointer->textData();
	QString cfirst = map["1st"];
	QString clast = map["last"];
	
	QString display;
	if (cfirst != "" && clast != "")
		display = cfirst + " " + clast;
	else if (cfirst != "")
		display = cfirst;
	else if (clast != "")
		display = clast;
	else
		display = pointer->serviceName().split("@")[0];

	Kopete::MetaContact *mc;

	// FIXME: The Standard Has Specifications on What To Do in case of a clash
	// We Ignore them over here.
	mc = addContact(pointer->serviceName(), display, bonjourGroup);

	BonjourContact *c = (BonjourContact *) mc->contacts()[0];

	//FIXME: QObject is needed to be called here as there is a conflict fo setproperty
	c->setremoteHostName(pointer->hostName());
	c->setremotePort(pointer->port());
	c->settextdata(pointer->textData());
	c->setusername(pointer->serviceName());
	c->setOnlineStatus(Kopete::OnlineStatus::Online);
}

void BonjourAccount::goingOffline(DNSSD::RemoteService::Ptr pointer)
{
	pointer->resolve();

	Kopete::Contact *c = contacts()[pointer->serviceName()];
	c->setOnlineStatus(Kopete::OnlineStatus::Offline);
}

void BonjourAccount::wipeOutContact(Kopete::Contact *c)
{
	if (c == myself() || c == NULL)
		return;

	Kopete::MetaContact *mc = c->metaContact();

	c->setOnlineStatus(Kopete::OnlineStatus::Offline);
	mc->removeContact(c);

	// FIXME: DeleteContact task should be extended and used
	c->deleteLater();

	if (mc->contacts().isEmpty())
		Kopete::ContactList::self()->removeMetaContact(mc);
}

void BonjourAccount::wipeOutAllContacts()
{
	QList <Kopete::Contact *> list = contacts().values();

	for (QList <Kopete::Contact *>::Iterator i = list.begin(); i != list.end(); i++)
		wipeOutContact(*i);
}

void BonjourAccount::disconnect()
{
	wipeOutAllContacts();

	localServer->close();
	service->stop();

	delete browser;
	browser = NULL;

	delete localServer;
	localServer = NULL;
	listeningPort = 0;

	delete service;
	service = NULL;

	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );
}

void BonjourAccount::slotGoOnline ()
{
	kDebug ( 14210 ) ;

	if (!isConnected ())
		connect ();
	else
		myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOnline );
	updateContactStatus();
}

void BonjourAccount::slotGoAway ()
{
	kDebug ( 14210 ) ;

	if (!isConnected ())
		connect();

	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourAway );
	updateContactStatus();
}


void BonjourAccount::slotGoOffline ()
{
	kDebug ( 14210 ) ;

	if (isConnected ())
		disconnect ();
	updateContactStatus();
}

void BonjourAccount::slotShowVideo ()
{
	kDebug ( 14210 ) ;

	if (isConnected ())
	{
		BonjourWebcamDialog *bonjourWebcamDialog = new BonjourWebcamDialog(0, 0);
		Q_UNUSED(bonjourWebcamDialog);
	}
	updateContactStatus();
}

void BonjourAccount::receivedMessage( const QString &message )
{
	// Look up the contact the message is from
	QString from;
	BonjourContact* messageSender;

	from = message.section( ':', 0, 0 );
	Kopete::Contact* contact = contacts()[from];
	messageSender = dynamic_cast<BonjourContact *>( contact );
}

void BonjourAccount::updateContactStatus()
{
	QHashIterator<QString, Kopete::Contact*>itr( contacts() );
	for ( ; itr.hasNext(); ) {
		itr.next();
		itr.value()->setOnlineStatus( myself()->onlineStatus() );
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
		BonjourContact *c = (BonjourContact *) *i;
		if (c->isRemoteAddress(addr))
			list<<c;
	}

	return list;
}

void BonjourAccount::newIncomingConnection()
{
	// Get Next Connection
	QTcpSocket *sock = localServer->nextPendingConnection();

	BonjourContactConnection *bcc = new BonjourContactConnection(sock);
	QObject::connect(bcc, SIGNAL(discoveredUserName(BonjourContactConnection *, QString)),
			this, SLOT(discoveredUserName(BonjourContactConnection *, QString)));;

	unknownConnections << bcc;
}

void BonjourAccount::discoveredUserName(BonjourContactConnection *conn, QString user)
{
	kDebug()<<"User Making Contact (unverified): "<<user;

	BonjourContact *c;

	if (! (c = verifyUser(conn, user))) {
		kDebug()<<"Ignoring Unverified User: "<<user;
		return;
	}
		
	kDebug()<<"User Verified: "<<user;

	unknownConnections.removeAll(conn);

	c->setConnection(conn);
}

BonjourContact *BonjourAccount::verifyUser(BonjourContactConnection *conn, QString user)
{
	// First Check the User Exists
	if (! contacts().keys().contains(user))
		return NULL;

	BonjourContact *c = (BonjourContact *) contacts()[user];

	if (c->getremoteAddress() != conn->getHostAddress())
		return NULL;

	return c;
}
	

#include "bonjouraccount.moc"
