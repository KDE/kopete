/***************************************************************************
                          yahooprotocol.cpp  -  Yahoo Plugin
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// Local Includes
#include "yahoodebug.h"
#include "yahoostatus.h"
#include "yahooprotocol.h"
#include "yahoocontact.h"

// Kopete Includes
#include "kopete.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "statusbaricon.h"
#include "systemtray.h"
// QT Includes
#include <qcursor.h>
#include <qwidget.h>
#include <qobject.h>
// KDE Includes
#include <kdebug.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kstatusbar.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>

class KPopupMenu;

// Yahoo Protocol
K_EXPORT_COMPONENT_FACTORY( kopete_yahoo, KGenericFactory<YahooProtocol> );

YahooProtocol::YahooProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	DEBUG(YDMETHOD, "YahooProtocol::YahooProtocol()");

	DEBUG(YDINFO, "Loading Yahoo Plugin...");

	if ( !protocolStatic_ )
		protocolStatic_ = this;
	else
		kdDebug() << "YahooProtocol already initialized" << endl;

	// Create Connection
    m_engine = new KYahoo;
    connect(m_engine, SIGNAL(newContact(QString, QString, QString)), this,
	    SLOT(slotNewContact(QString, QString, QString)));

	// Load icons
    initIcons();

	// Load Status Actions
    initActions();

	// Create statusbar Icon
    statusBarIcon = new StatusBarIcon();
    QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)),
		     this, SLOT(slotIconRightClicked(const QPoint&)));
    statusBarIcon->setPixmap(offlineIcon);

	// Create preferences menu
    mPrefs = new YahooPreferences("yahoo_protocol_32", this);
    connect(mPrefs, SIGNAL(saved(void)), this,
	    SLOT(slotSettingsChanged(void)));

	// Ensure that we are disconnectd
	mIsConnected = false;

	/* Call slotSettingsChanged() to get it all registered. */
	slotSettingsChanged();

	if (KGlobal::config()->readBoolEntry("AutoConnect", "0"))
		Connect();
}

/***************************************************************************
 *                                                                         *
 *   Re-implementation of Plugin class methods                             *
 *                                                                         *
 ***************************************************************************/

bool YahooProtocol::serialize( KopeteMetaContact *metaContact,
                             QStringList &strList ) const
{
/*
//	kdDebug() << "[ICQProtocol] Serializing metacontact" << endl;
	KopeteContact *c;
	bool done = false;
	for( c = metaContact->contacts().first(); c ; c = metaContact->contacts().next() )
	{
		if ( c->protocol() == this->id() )
		{
			kdDebug() << "Found ICQ-Contact in MetaContact" << endl;
			ICQContact *g = static_cast<ICQContact*>(c);

			if ( !g )		// try the next one :)
				continue;

			// TODO, only use displayName() and make sure that var is always valid
			if ( !QString(g->mUser->Alias).isEmpty() )
				strList << QString(g->mUser->Alias);
			else
				strList << g->displayName();

			metaContact->setAddressBookField( ICQProtocol::protocol(), "messaging/icq" , g->id() );
			done = true;
		}
	}
	return done;
	*/
	return true;
}

YahooProtocol *YahooProtocol::protocol()
{
	return protocolStatic_;
}


void YahooProtocol::deserialize( KopeteMetaContact *metaContact,
                           const QStringList &strList )
{
//	kdDebug() << "[ICQProtocol] Deserializing metacontact" << endl;
    /*
	QString protocolId = this->id();

	QString uin, alias;
	uin = metaContact->addressBookField( this, "messaging/icq" ) ;

	alias = strList.first();

	if ( alias == "TODOdisplayName" ) // fix for stupid old icq-plugin
		alias = "";

	uint UIN = uin.toUInt();

	if (UIN <= 0 || contactsMap.contains(UIN)) return ;
	ICQUser *user = engine->getUser(UIN, true);
	if (!user) return;

	//ICQGroup *group = findGroup(groupName, true);
	//if (!group) return 0L;

	//user->GrpId = group->Id;
	//engine->moveUser(user, group);

	//if (!alias.isEmpty() && QString(user->Alias).isEmpty())
	user->Alias = alias;

	ICQContact *contact = new ICQContact(protocolId, metaContact, user, this);
	KopeteContact *c = contact;

	metaContact->addContact( c, QStringList() );
	contactsMap.insert(UIN, contact);

//	ICQEvent e(EVENT_INFO_CHANGED);
//	engine->process_event(&e);
	*/
}

QStringList YahooProtocol::addressBookFields() const
{
    return QStringList("messaging/yahoo");
}

// Unload statusbar icon
bool YahooProtocol::unload()
{
	DEBUG(YDMETHOD, "YahooProtocol::unload()");

    DEBUG(YDINFO, "Unloading YahooProtocol...");
    if (kopeteapp->statusBar()) {
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
    }

    emit protocolUnloading();
    return true;
}

void YahooProtocol::init()
{
}

/*********************************************************************/

// Destructor
YahooProtocol::~YahooProtocol()
{
	DEBUG(YDMETHOD, "YahooProtocol::~YahooProtocol()");
	protocolStatic_ = 0L;
}

YahooProtocol* YahooProtocol::protocolStatic_ = 0L;

KopeteContact *YahooProtocol::myself() const
{
#warning "For future maintainers : reimplement this!"
	return 0L;
}


// Connect to server
void YahooProtocol::Connect()
{
	DEBUG(YDMETHOD, "YahooProtocol::Connect()");

    if (!isConnected()) {
		DEBUG(YDINFO, "Attempting to connect to Yahoo server <"
			<< mServer << ":" << mPort << "< with user <" << mUsername << ">");
		m_engine->Connect(mServer, mPort, mUsername, mPassword);
    }
	else if (isAway()) {	// They're really away, and they want to un-away.
		// XXX slotGoOnline();
    }
	else {			// Nope, just your regular crack junky.
		DEBUG(YDINFO,
		"Yahoo plugin: Ignoring connect request (already connected).");
    }
}


// Disconnect from server
void YahooProtocol::Disconnect()
{
	DEBUG(YDMETHOD, "YahooProtocol::Disconnect()");

    if (isConnected()) {
		DEBUG(YDINFO, "Attempting to disconnect from Yahoo server "
			<< mServer);
		m_engine->Disconnect();
    }
	else {			// Again, what's with the crack? Sheez.
		DEBUG(YDINFO,
			"Ignoring disconnect request (not connected).");
    }
}


// Set user available
void YahooProtocol::setAvailable()
{
	DEBUG(YDMETHOD, "YahooProtocol::setAvailable()");
}


// Set user away
void YahooProtocol::setAway()
{
	DEBUG(YDMETHOD, "YahooProtocol::setAway()");
}


// Return true if connected
bool YahooProtocol::isConnected() const
{
	DEBUG(YDMETHOD, "YahooProtocol::isConnected()");
	return false; // XXX
}


// Return true if away
bool YahooProtocol::isAway() const
{
	DEBUG(YDMETHOD, "YahooProtocol::isAway()");

	return false; // XXX
}


// Return protocol icon name
QString YahooProtocol::protocolIcon() const
{
	DEBUG(YDMETHOD, "YahooProtocol::protocolIcon");

	return ""; // XXX
}

// Return "add contact" dialog
AddContactPage *YahooProtocol::createAddContactWidget(QWidget * parent)
{
	DEBUG(YDMETHOD, "YahooProtocol::createAddContactWidget(<parent>)");

	return NULL; // XXX
}


// CallBack when clicking on statusbar icon
void YahooProtocol::slotIconRightClicked(const QPoint&)
{
	DEBUG(YDMETHOD, "YahooProtocol::slotIconRightClicked(<qpoint>)");

    QString handle = mUsername + "@" + mServer;

    popup = new KPopupMenu(statusBarIcon);
    popup->insertTitle(handle);
    actionGoOnline->plug(popup);
    actionGoOffline->plug(popup);
    actionGoStatus001->plug(popup);
    actionGoStatus002->plug(popup);
    actionGoStatus003->plug(popup);
    actionGoStatus004->plug(popup);
    actionGoStatus005->plug(popup);
    actionGoStatus006->plug(popup);
    actionGoStatus007->plug(popup);
    actionGoStatus008->plug(popup);
    actionGoStatus009->plug(popup);
    actionGoStatus012->plug(popup);
    actionGoStatus099->plug(popup);
    actionGoStatus999->plug(popup);
    popup->popup(QCursor::pos());
}



// Callback when settings changed
void YahooProtocol::slotSettingsChanged()
{
	DEBUG(YDMETHOD, "YahooProtocol::slotSettingsChanged()");

    mUsername = KGlobal::config()->readEntry("UserID", "");
    mPassword = KGlobal::config()->readEntry("Password", "");
    mServer   = KGlobal::config()->readEntry("Server", "cs.yahoo.com");
    mPort     = KGlobal::config()->readNumEntry("Port", 5050);
}


// Add a contact to main window
void YahooProtocol::slotNewContact(QString userID, QString name,
				    QString group)
{
	DEBUG(YDMETHOD, "YahooProtocol::slotNewContact(" << userID << ", "
			<< name << ", " << group << ")");

    if (group == QString("")) {
		group = i18n("Unknown");
    }
    //kopeteapp->contactList()->addContact( new YahooContact(userID, name, group, this), group);
}


// Private initIcons
void YahooProtocol::initIcons()
{
	DEBUG(YDMETHOD, "YahooProtocol::initIcons()");

    KIconLoader *loader = KGlobal::iconLoader();
    KStandardDirs dir;

    onlineIcon  = QPixmap(loader->loadIcon("yahoo_online",  KIcon::User));
    offlineIcon = QPixmap(loader->loadIcon("yahoo_offline", KIcon::User));
    busyIcon    = QPixmap(loader->loadIcon("yahoo_busy",    KIcon::User));
    idleIcon    = QPixmap(loader->loadIcon("yahoo_idle",    KIcon::User));
    mobileIcon  = QPixmap(loader->loadIcon("yahoo_mobile",  KIcon::User));
}


// Private initActions
void YahooProtocol::initActions()
{
	DEBUG(YDMETHOD, "YahooProtocol::initActions()");

    actionGoOnline = new KAction(YSTAvailable, "yahoo_online",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoOffline = new KAction(i18n("Offline"), "yahoo_offline",
			0, this, SLOT(Disconnect()), this, "actionYahooDisconnect");
    actionGoStatus001 = new KAction(YSTBeRightBack, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus002 = new KAction(YSTBusy, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus003 = new KAction(YSTNotAtHome, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus004 = new KAction(YSTNotAtMyDesk, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus005 = new KAction(YSTNotInTheOffice, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus006 = new KAction(YSTOnThePhone, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus007 = new KAction(YSTOnVacation, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus008 = new KAction(YSTOutToLunch, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus009 = new KAction(YSTSteppedOut, "yahoo_busy",
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus012 = new KAction(i18n("Invisible"), "yahoo_offline",
			0, this, SLOT(Connect()), this, "actionYahooConnect"); // XXX Connect with invisible on
    actionGoStatus099 = new KAction(i18n("Custom"), "yahoo_online",
			0, this, SLOT(Connect()), this, "actionYahooConnect"); // XXX Get some dialogbox
    actionGoStatus999 = new KAction(YSTIdle, "yahoo_idle",
			0, this, SLOT(Connect()), this, "actionYahooConnect");

    actionStatusMenu = new KActionMenu("Yahoo", this);
    actionStatusMenu->insert(actionGoOnline);
    actionStatusMenu->insert(actionGoOffline);
    actionStatusMenu->insert(actionGoStatus001);
    actionStatusMenu->insert(actionGoStatus002);
    actionStatusMenu->insert(actionGoStatus003);
    actionStatusMenu->insert(actionGoStatus004);
    actionStatusMenu->insert(actionGoStatus005);
    actionStatusMenu->insert(actionGoStatus006);
    actionStatusMenu->insert(actionGoStatus007);
    actionStatusMenu->insert(actionGoStatus008);
    actionStatusMenu->insert(actionGoStatus009);
    actionStatusMenu->insert(actionGoStatus012);
    actionStatusMenu->insert(actionGoStatus099);
    actionStatusMenu->insert(actionGoStatus999);
    actionStatusMenu->plug(kopeteapp->systemTray()->contextMenu(), 1);
}

#include "yahooprotocol.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

