/***************************************************************************
                          ircprotocol.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
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
#include <klocale.h>
#include "kopete.h"
#include <ircaddcontactpage.h>
#include "ircchatview.h"

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

IRCProtocol::IRCProtocol(): QObject(0, "IRC"), IMProtocol()
{
	kdDebug() << "\nIRC Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
 	initIcons();
	manager = new IRCServerManager();
	kdDebug() << "IRC Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();

	kdDebug() << "IRC Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(protocolSmallIcon);

	kdDebug() << "IRC Protocol Plugin: Creating Config Module\n";
	new IRCPreferences(protocolIcon, this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>Sorry, you haven't setup your IRC settings for the first time, please do so by going to File->Configure Kopete->IRC Plugin. Once you are done there, please try connecting again.</qt>"), "Preferences non-existant");
		return;
	}

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
}

void IRCProtocol::slotNamesList(const QString &channel, const QString &name, int userClass)
{
	kdDebug() << "IRC Plugin: User \"" << name << "\" in channel \"" << channel << "\" as user class type " << userClass << endl;
}

void IRCProtocol::slotConnectedToHost()
{

}

void IRCProtocol::slotUserJoinedChannel(const QString &user, const QString &channel)
{
	kdDebug() << "IRC Plugin: User " << user << " joining channel " << channel << endl;
}

void IRCProtocol::slotIncomingMotd(const QString &message)
{

}

void IRCProtocol::addContact(const QString &server, const QString &contact, bool connectNow, bool joinNow)
{
	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	QString serverAndNick = nick;
	serverAndNick.append("@");
	serverAndNick.append(server);
	IRCServerContact *serverContact = manager->findServer(serverAndNick);
	if (serverContact != 0)
	{
		IRCContact *listContact = new IRCContact((QListViewItem *)serverContact, server, contact, 6667, joinNow, serverContact);
	} else {
		IRCServerContact *serverItem = manager->addServer(serverAndNick, connectNow);
		if (serverItem != 0)
		{
			IRCContact *listContact = new IRCContact(serverItem, server, contact, 6667, joinNow, serverItem);
		}
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
	protocolSmallIcon = QPixmap(loader->loadIcon("irc_protocol_small", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("irc_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("irc_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("irc_away", KIcon::User));
	joinIcon = QPixmap(loader->loadIcon("irc_join", KIcon::User));
	privmsgIcon = QPixmap(loader->loadIcon("irc_privmsg", KIcon::User));
}


