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
#include "bonjourfakeserver.h"
#include "bonjourprotocol.h"
#include "bonjourcontactconnection.h"


BonjourAccount::BonjourAccount( BonjourProtocol *parent, const QString& accountID )
: Kopete::Account ( parent, accountID )
{
	// Init the myself contact
	setMyself( new BonjourContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );
	m_server = new BonjourFakeServer();;

	service = NULL;
	localServer = NULL;
	listeningPort = 0;
	browser = NULL;

	// All Contacts Go To The Bonjour Group
	bonjourGroup = Kopete::ContactList::self()->findGroup("Bonjour");

	// Clean out Contacts from last time when kopete starts up
	wipeOutAllContacts();
}

BonjourAccount::~BonjourAccount()
{
	if (isConnected())
		disconnect();
	delete m_server;
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

	browser->startBrowse();
}

void BonjourAccount::startPublish()
{
	if (! fullName.contains('@')) {
		fullName.append("@");
		fullName.append(getLocalHostName().toUtf8());
	}

	service = new DNSSD::PublicService(fullName, "_presence._tcp", listeningPort);

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

        service->publish();
}

void BonjourAccount::connect( const Kopete::OnlineStatus& /* initialStatus */ )
{
	if (fullName.isEmpty())
		fullName = accountId().toUtf8();

	if (! startLocalServer())
		return;

	startPublish();

	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOnline );

	startBrowse();

	QObject::connect ( m_server, SIGNAL ( messageReceived( const QString & ) ),
			this, SLOT ( receivedMessage( const QString & ) ) );
}

void BonjourAccount::comingOnline(DNSSD::RemoteService::Ptr pointer)
{
	pointer->resolve();

	kDebug()<<"\nComing Online\n";
	
	if (pointer->serviceName() == fullName)			// Don't Add Ourselves
		return;

	QString display = pointer->serviceName().split("@")[0];

	Kopete::MetaContact *mc;

	// FIXME: The Standard Has Specifications on What To Do in case of a clash
	// We Ignore them over here.
	mc = addContact(pointer->serviceName(), display, bonjourGroup);

	Kopete::Contact *c = mc->contacts()[0];

	//FIXME: QObject is needed to be called here as there is a conflict fo setproperty
	c->QObject::setProperty("remoteHostName", pointer->hostName());
	c->QObject::setProperty("remotePort", pointer->port());
	c->setOnlineStatus(Kopete::OnlineStatus::Online);
}

void BonjourAccount::goingOffline(DNSSD::RemoteService::Ptr pointer)
{
	pointer->resolve();

	Kopete::Contact *c = contacts()[pointer->serviceName()];
	wipeOutContact(c);
}

void BonjourAccount::wipeOutContact(Kopete::Contact *c)
{
	if (c == myself())
		return;

	Kopete::MetaContact *mc = c->metaContact();

	c->setOnlineStatus(Kopete::OnlineStatus::Offline);
	mc->removeContact(c);

	// FIXME: DeleteContact task should be extended and used
	Kopete::DeleteContactTask task(c);
	task.start();

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
	localServer->close();
	service->stop();

	delete browser;
	browser = NULL;

	delete localServer;
	localServer = NULL;
	listeningPort = 0;

	delete service;
	service = NULL;

	wipeOutAllContacts();

	myself()->setOnlineStatus( BonjourProtocol::protocol()->bonjourOffline );
	QObject::disconnect ( m_server, 0, 0, 0 );
}

BonjourFakeServer * BonjourAccount::server()
{
	return m_server;
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

	kDebug( 14210 ) << " got a message from " << from << ", " << messageSender << ", is: " << message;
	// Pass it on to the contact to process and display via a KMM
	if ( messageSender )
		messageSender->receivedMessage( message );
	else
		kWarning(14210) << "unable to look up contact for delivery";
}

void BonjourAccount::updateContactStatus()
{
	QHashIterator<QString, Kopete::Contact*>itr( contacts() );
	for ( ; itr.hasNext(); ) {
		itr.next();
		itr.value()->setOnlineStatus( myself()->onlineStatus() );
	}
}

void BonjourAccount::setfullName(const QByteArray &n_fullName)
{
	fullName = n_fullName;
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

const QByteArray BonjourAccount::getfullName() const
{
	return fullName;
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

	unknownConnections << bcc;
}

#include "bonjouraccount.moc"
