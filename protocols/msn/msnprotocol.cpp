/***************************************************************************
                          msnprotocol.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "msnprotocol.h"
#include <msnadd.h>
#include "kopete.h"
#include <msnaddcontactpage.h>

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

MSNProtocol::MSNProtocol(): QObject(0, "MSN"), IMProtocol()
{
	// Remember to move all this to init()

	kdDebug() << "\nMSN Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
 	initIcons();
	
	kdDebug() << "MSN Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();
	
	kdDebug() << "MSN Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(&offlineIcon);

	kdDebug() << "MSN Protocol Plugin: Creating Config Module\n";
	new MSNPreferences(protocolIcon, this);
	kdDebug() << "MSN Protocol Plugin: Done\n";

	
	/** Autoconnect if is selected in config */
	KGlobal::config()->setGroup("MSN");
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
	
}

MSNProtocol::~MSNProtocol()
{

}

///////////////////////////////////////////////////
//           Plugin Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::init()
{

}

bool MSNProtocol::unload()
{
	kdDebug() << "MSN Protocol: Unloading...\n";
	kopeteapp->statusBar()->removeWidget(statusBarIcon);
	delete statusBarIcon;
	// heh!
	return 1;
}

///////////////////////////////////////////////////
//           IMProtocol Class reimplementation
///////////////////////////////////////////////////

void MSNProtocol::Connect()
{
	KGlobal::config()->setGroup("MSN");
	kdDebug() << "Attempting to connect to MSN" << endl;
	kdDebug() << "Setting Monopoly mode..." << endl;
	kdDebug() << "Using Micro$oft UserID " << KGlobal::config()->readEntry("UserID", "0") << " with password " << KGlobal::config()->readEntry("Password", "") << endl;
}

bool MSNProtocol::isConnected()
{
	return false;	
}

/** This i used for al protocol selection dialogs */
QPixmap MSNProtocol::getProtocolIcon()
{
	return protocolIcon;
}

AddContactPage *MSNProtocol::getAddContactWidget(QWidget *parent)
{
	return (new MSNAddContactPage(this,parent));	
}

///////////////////////////////////////////////////
//           Internal functions implementation
///////////////////////////////////////////////////

/** No descriptions */
void MSNProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();

	protocolIcon = QPixmap(loader->loadIcon("msn_protocol", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("msn_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("msn_offline", KIcon::User));
}
/*
void MSNProtocol::slotConnected()
{
	statusBarIcon->setPixmap(onlineIcon);
}
*/

