/***************************************************************************
                          ircprotocol.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
 ***************************************************************************

 ***************************************************************************
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
#include <kpopupmenu.h>
#include <klocale.h>
#include <qcursor.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "ircprotocol.h"
#include "irccontact.h"
#include "ircadd.h"
#include "kopete.h"
#include "ircaddcontactpage.h"
#include "ircchatview.h"
#include "ircservercontact.h"
#include "ircmessage.h"
#include "ircservermanager.h"
#include "contactlist.h"

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

IRCProtocol::IRCProtocol(): QObject(0, "IRC"), KopeteProtocol()
{
	kdDebug() << "\nIRC Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
	setIcon( "irc_protocol_small" );

	initIcons();
	manager = new IRCServerManager();
	kdDebug() << "IRC Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();

	kdDebug() << "IRC Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(protocolSmallIcon);
	connect(statusBarIcon, SIGNAL(rightClicked(const QPoint)), this, SLOT(slotIconRightClicked(const QPoint)));

	kdDebug() << "IRC Protocol Plugin: Creating Config Module\n";
	new IRCPreferences("irc_protocol", this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>Sorry, you haven't setup your IRC settings for the first time, please do so by going to File->Configure Kopete->IRC Plugin. Once you are done there, please try connecting again.</qt>"), "Preferences non-existant");
		return;
	}

	QString filename = locateLocal("data", "kopete/irc.buddylist");
	mConfig = new KSimpleConfig(filename);

	QStringList contacts = mConfig->groupList();
	for(QStringList::Iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		mConfig->setGroup((*it));
		QString groupName = mConfig->readEntry("Group", "");
		if (groupName.isEmpty())
		{
			continue;
		}
		QString server = mConfig->readEntry("Server", "");
		if (server.isEmpty())
		{
			KGlobal::config()->setGroup("IRC");
			server = KGlobal::config()->readEntry("Server", "irc.unknown.com");
		}
		addContact(groupName, server, (*it), false, false);
	}

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->readBoolEntry("HideConsole", false) == false)
	{
		(void)new IRCServerContact(this);
	}

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry("AutoConnect", "0") )
	{
		Connect();
	}
}

void IRCProtocol::slotIconRightClicked(const QPoint)
{
	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle("IRC");
	popup->insertItem(i18n("Open New IRC Console"), this, SLOT(slotNewConsole()));
	popup->popup(QCursor::pos());
}

void IRCProtocol::slotNewConsole()
{
	(void)new IRCServerContact(this);
}

void IRCProtocol::addContact(const QString &groupName, const QString &server, const QString &contact, bool connectNow, bool joinNow)
{
	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");

	QString serverAndNick = nick;
	serverAndNick.append("@");
	serverAndNick.append(server);
	IRCServerContact *serverContact = manager->findServer(serverAndNick);
	if (serverContact != 0)
	{
		(void)new IRCContact(groupName, server, contact, 6667, joinNow, serverContact);
	} else {
		IRCServerContact *serverItem = manager->addServer(serverAndNick, connectNow, this);
		if (serverItem != 0)
		{
			(void)new IRCContact(groupName, server, contact, 6667, joinNow, serverItem);
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

	if( kopeteapp->statusBar() )
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

	return true;
}

///////////////////////////////////////////////////
//           KopeteProtocol Class reimplementation
///////////////////////////////////////////////////

void IRCProtocol::Connect()
{

}

void IRCProtocol::Disconnect()
{

}


bool IRCProtocol::isConnected() const
{
	return false;
}

void IRCProtocol::setAway(void)
{
// TODO
}

void IRCProtocol::setAvailable(void)
{
// TODO
}

bool IRCProtocol::isAway(void) const
{
// TODO
	return false;
}

/** This i used for al protocol selection dialogs */
QString IRCProtocol::protocolIcon() const
{
	return "irc_protocol";
}

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent)
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

	protocolSmallIcon = QPixmap(loader->loadIcon("irc_protocol_small", KIcon::User));
	onlineIcon = QPixmap(loader->loadIcon("irc_online", KIcon::User));
	offlineIcon = QPixmap(loader->loadIcon("irc_offline", KIcon::User));
	awayIcon = QPixmap(loader->loadIcon("irc_away", KIcon::User));
	joinIcon = QPixmap(loader->loadIcon("irc_join", KIcon::User));
	privmsgIcon = QPixmap(loader->loadIcon("irc_privmsg", KIcon::User));
}


#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

