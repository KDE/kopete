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
#include "yahooprotocol.h"

// Kopete Includes
#include "kopete.h"
#include "statusbaricon.h"

// QT Includes
#include <qcursor.h>

// KDE Includes
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <klocale.h>

class KPopupMenu;


// Yahoo Protocol
YahooProtocol::YahooProtocol() : QObject(0, "YahooProtocol"), KopeteProtocol()
{
	DEBUG(0, "YahooProtocol::YahooProtocol");

	kdDebug() << "Yahoo plugin: Loading ..." << endl;

	// Load icons
    initIcons();

	// Load Status Actions
    initActions();

	// Create statusbar Icon
    statusBarIcon = new StatusBarIcon();
    QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)),
		     this, SLOT(slotIconRightClicked(const QPoint)));
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

// Unload statusbar icon
bool YahooProtocol::unload()
{
	DEBUG(0, "YahooProtocol::unload");

    kdDebug() << "Yahoo plugin: Unloading...\n";
    if (kopeteapp->statusBar()) {
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
    }

    emit protocolUnloading();
    return true;
}


// Connect to server
void YahooProtocol::Connect()
{
	DEBUG(0, "YahooProtocol::Connect");
}


// Disconnect from server
void YahooProtocol::Disconnect()
{
	DEBUG(0, "YahooProtocol::Disconnect");
}


// Set user available
void YahooProtocol::setAvailable()
{
	DEBUG(0, "YahooProtocol::setAvailable");
}


// Set user away
void YahooProtocol::setAway()
{
	DEBUG(0, "YahooProtocol::setAway");
}


// Return true if connected
bool YahooProtocol::isConnected() const
{
	DEBUG(0, "YahooProtocol::isConnect");
	return false; // XXX 
}


// Return true if away
bool YahooProtocol::isAway() const
{
	DEBUG(0, "YahooProtocol::isAway");

	return false; // XXX 
}


// Return protocol icon name
QString YahooProtocol::protocolIcon() const
{
	DEBUG(0, "YahooProtocol::protocolIcon");

	return ""; // XXX
}

// Return "add contact" dialog
AddContactPage *YahooProtocol::createAddContactWidget(QWidget * parent)
{
	DEBUG(0, "YahooProtocol::createAddContactWidget");

	return NULL; // XXX
}


// CallBack when clicking on statusbar icon
void YahooProtocol::slotIconRightClicked(const QPoint)
{
	DEBUG(0, "YahooProtocol::slotIconRightClicked");

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
	DEBUG(0, "YahooProtocol::slotSettingsChanged");

    mUsername = KGlobal::config()->readEntry("UserID", "");
    mPassword = KGlobal::config()->readEntry("Password", "");
    mServer   = KGlobal::config()->readEntry("Server", "cs.yahoo.com");
    mPort     = KGlobal::config()->readNumEntry("Port", 5050);
}


// Private initIcons
void YahooProtocol::initIcons()
{
	DEBUG(0, "YahooProtocol::initIcons");

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
    actionGoOnline = new KAction(i18n("Online"), "yahoo_online", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoOffline = new KAction(i18n("Offline"), "yahoo_offline", 
			0, this, SLOT(Disconnect()), this, "actionYahooDisconnect");
    actionGoStatus001 = new KAction(i18n("Be Right Back"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus002 = new KAction(i18n("Busy"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus003 = new KAction(i18n("Not At Home"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus004 = new KAction(i18n("Not At My Desk"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus005 = new KAction(i18n("Not In The Office"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus006 = new KAction(i18n("On The Phone"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus007 = new KAction(i18n("On Vacation"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus008 = new KAction(i18n("Out To Lunch"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus009 = new KAction(i18n("Stepped Out"), "yahoo_busy", 
			0, this, SLOT(Connect()), this, "actionYahooConnect");
    actionGoStatus012 = new KAction(i18n("Invisible"), "yahoo_offline", 
			0, this, SLOT(Connect()), this, "actionYahooConnect"); // XXX Connect with invisible on
    actionGoStatus099 = new KAction(i18n("Custom"), "yahoo_online", 
			0, this, SLOT(Connect()), this, "actionYahooConnect"); // XXX Get some dialogbox
    actionGoStatus999 = new KAction(i18n("Idle"), "yahoo_idle", 
			0, this, SLOT(Connect()), this, "actionYahooConnect"); // XXX Get some dialogbox

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

// vim: set ts=4 sts=4 sw=4 noet:
