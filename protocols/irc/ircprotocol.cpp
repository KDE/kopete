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

	kdDebug() << "IRC Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();

	kdDebug() << "IRC Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(offlineIcon);

	kdDebug() << "IRC Protocol Plugin: Creating Config Module\n";
	new IRCPreferences(protocolIcon, this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>Sorry, you haven't setup your IRC settings for the first time, please do so by going to File->Configure Kopete->IRC Plugin. Once you are done there, please try connecting again.</qt>"), "Preferences non-existant");
		return;
	}

	engine = new KIRC();
	QObject::connect(engine, SIGNAL(incomingMotd(const QString &)), this, SLOT(slotIncomingMotd(const QString &)));
	QObject::connect(engine, SIGNAL(connectedToServer()), this, SLOT(slotConnectedToHost()));
	QObject::connect(engine, SIGNAL(userJoinedChannel(const QString &, const QString &)), this, SLOT(slotUserJoinedChannel(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingNamesList(const QString &, const QString &, const int)), this, SLOT(slotNamesList(const QString &, const QString &, const int)));
	
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

void IRCProtocol::addContact(const QString &server, const QString &contact)
{
	// TODO: Add some sort of check here to see if the room already exists
	IRCChatView *chatView = new IRCChatView(server, contact, this);
	chatView->show();
	QObject::connect(engine, SIGNAL(userJoinedChannel(const QString &, const QString &)), chatView, SLOT(userJoinedChannel(const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingMessage(const QString &, const QString &, const QString &)), chatView, SLOT(incomingMessage(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingPartedChannel(const QString &, const QString &, const QString &)), chatView, SLOT(userPartedChannel(const QString &, const QString &, const QString &)));
	QObject::connect(engine, SIGNAL(incomingNamesList(const QString &, const QString &, const int)), chatView, SLOT(incomingNamesList(const QString &, const QString &, const int)));
	QObject::connect(engine, SIGNAL(incomingAction(const QString &, const QString &, const QString &)), chatView, SLOT(incomingAction(const QString &, const QString &, const QString &)));
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
	KGlobal::config()->setGroup("IRC");
	QString server = KGlobal::config()->readEntry("Server", "irc.openprojects.net");
	unsigned int port = KGlobal::config()->readEntry("Port", "6667").toUInt();
	QString user = "kopeteuser";
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	engine->connectToServer(server, port, user, nick);
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
	joinIcon = QPixmap(loader->loadIcon("irc_join", KIcon::User));
	privmsgIcon = QPixmap(loader->loadIcon("irc_privmsg", KIcon::User));
}


