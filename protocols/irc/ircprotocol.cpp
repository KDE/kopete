/***************************************************************************
                          ircprotocol.cpp  -  description
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
#include <klocale.h>
#include "ircprotocol.h"
#include "irccontact.h"
#include <ircadd.h>
#include "kopete.h"
#include <ircaddcontactpage.h>

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

IRCProtocol::IRCProtocol(): QObject(0, "IRC"), IMProtocol()
{
	kdDebug() << "\nIRC Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
 	initIcons();
	
	kdDebug() << "IRC Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();
	
	kdDebug() << "IRC Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(&offlineIcon);

	kdDebug() << "IRC Protocol Plugin: Creating Config Module\n";
	new IRCPreferences(protocolIcon, this);
	
	KGlobal::config()->setGroup("IRC");

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
	
}

IRCProtocol::~IRCProtocol()
{

}

///////////////////////////////////////////////////
//           Plugin Class reimplementation
///////////////////////////////////////////////////

void IRCProtocol::init()
{

}

bool IRCProtocol::unload()
{
	kdDebug() << "IRC Protocol: Unloading...\n";
	kopeteapp->statusBar()->removeWidget(statusBarIcon);
	delete statusBarIcon;
	// heh!
	return 1;
}

///////////////////////////////////////////////////
//           IMProtocol Class reimplementation
///////////////////////////////////////////////////

void IRCProtocol::Connect()
{
	
}

void IRCProtocol::Disconnect()
{

}


bool IRCProtocol::isConnected()
{
	return false;
}

/** This i used for al protocol selection dialogs */
QPixmap IRCProtocol::getProtocolIcon()
{
	return protocolIcon;
}

AddContactPage *IRCProtocol::getAddContactWidget(QWidget *parent)
{
	return (new IRCAddContactPage(this,parent));	
}

///////////////////////////////////////////////////
//           Internal functions implementation
///////////////////////////////////////////////////

/** No descriptions */
void IRCProtocol::initIcons()
{
	KIconLoader *loader = KGlobal::iconLoader();

	protocolIcon = QPixmap(loader->loadIcon("irc_protocol", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("irc_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("irc_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("irc_away", KIcon::User));
	naIcon = QPixmap(loader->loadIcon("irc_na", KIcon::User));
}

		