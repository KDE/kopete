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

// KDE Includes
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>


// Yahoo Protocol
YahooProtocol::YahooProtocol() : QObject(0, "YahooProtocol"), KopeteProtocol()
{
	DEBUG(0, "YahooProtocol::YahooProtocol");

	kdDebug() << "Yahoo plugin: Loading ..." << endl;

	// Load icons
    initIcons();

	// Create statusbar Icon
    statusBarIcon = new StatusBarIcon();
    QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)),
		     this, SLOT(slotIconRightClicked(const QPoint)));
    statusBarIcon->setPixmap(offlineIcon);

	// Create preferences menu
    mPrefs = new YahooPreferences("yahoo_protocol_32", this);
    connect(mPrefs, SIGNAL(saved(void)), this,
	    SLOT(settingsChanged(void)));

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

#include "yahooprotocol.moc"

// vim: set ts=4 sts=4 sw=4 noet:
