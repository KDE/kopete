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
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <qcursor.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>

#include "ircprotocol.h"
#include "irccontact.h"
#include "ircadd.h"
#include "kopete.h"
#include "kopetemetacontact.h"
#include "ircaddcontactpage.h"
#include "ircchatview.h"
#include "ircservercontact.h"
#include "ircmessage.h"
#include "ircservermanager.h"
#include "kopetecontactlist.h"
#include "kopetewindow.h"

///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol::IRCProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	kdDebug() << "\nIRC Plugin Loading\n";
	// Load all ICQ icons from KDE standard dirs
	setIcon( "irc_protocol_small" );

	initIcons();
	m_serverManager = new IRCServerManager();
	kdDebug() << "IRC Protocol Plugin: Creating Status Bar icon\n";
	statusBarIcon = new StatusBarIcon();

	kdDebug() << "IRC Protocol Plugin: Setting icon offline\n";
	statusBarIcon->setPixmap(protocolSmallIcon);
	connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)), this, SLOT(slotIconRightClicked(const QPoint&)));

	kdDebug() << "IRC Protocol Plugin: Creating Config Module\n";
	new IRCPreferences("irc_protocol", this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>You haven't setup your IRC settings for the first time. Please do so by going to File->Configure Kopete->IRC Plugin. Once you have done that, try connecting again.</qt>"), i18n("Preferences Nonexistent"));
		return;
	}

	QString filename = locateLocal("data", "kopete/irc.buddylist");
	m_config = new KSimpleConfig(filename);

	QStringList contacts = m_config->groupList();
	for(QStringList::Iterator it = contacts.begin(); it != contacts.end(); it++)
	{
		m_config->setGroup((*it));
		QString groupName = m_config->readEntry("Group", "");
		if (groupName.isEmpty())
		{
			continue;
		}
		QString server = m_config->readEntry("Server", "");
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

void IRCProtocol::slotIconRightClicked(const QPoint&)
{
	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle("IRC");
	popup->insertItem(i18n("Open New IRC Console"), this, SLOT(slotNewConsole()));
	popup->popup(QCursor::pos());
}

void IRCProtocol::slotNewConsole()
{
	kdDebug() << "IRCProtocol::slotNewConsole";
	(void)new IRCServerContact(this);
}

void IRCProtocol::addContact( const QString &groupName, const QString &server, const QString &contact, bool connectNow, bool joinNow)
{

	QString protocolID = this->id();
	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString serverAndNick = nick;
	serverAndNick.append("@");
	serverAndNick.append(server);
	IRCServerContact *serverContact = m_serverManager->findServer(serverAndNick);

	KopeteContactList *l = KopeteContactList::contactList();
	KopeteMetaContact *m = l->findContact( this->id(), QString::null, serverAndNick );

	if( m )
	{
		// Existing contact, update data
		// FIXME: TODO!
		kdDebug() << "IRCProtocol::slotContactList: Not implemented: "
			<< "Meta contact already contains contact " << contact
			<< "???" << endl;

		//KopeteContact *c = m->findContact( this->id(), QString::null, serverAndNick );
	}
	else
	{
		m = new KopeteMetaContact();
		if (serverContact != 0)
		{
			m->addContact( new IRCContact(groupName, server, contact, 6667, joinNow, serverContact, m, protocolID));
		} else {
			IRCServerContact *serverItem = m_serverManager->addServer(serverAndNick, connectNow, this);
			if (serverItem != 0)
			{
				m->addContact(new IRCContact(groupName, server, contact, 6667, joinNow, serverItem, m, protocolID));
			}
		}
		KopeteContactList::contactList()->addMetaContact(m);

	}
}

KopeteContact* IRCProtocol::createContact( KopeteMetaContact *parent, const QString &serializedData )
{
    QString protocolID = this->id();
	// FIXME: serializedData contains much more than just the server, target, port, joinonconnect contact

	// FIXME: more error-proof deserialize would be useful :)
	QStringList data    = QStringList::split( ' ', serializedData );
	QString server = data[ 0 ].replace( QRegExp( "%20" ), " " );
	QString target = data[ 1 ].replace( QRegExp( "%20" ), " " );
	unsigned int port =(data[ 2 ].replace( QRegExp( "%20" ), " " )).toUInt();
	bool joinOnConnect = (data[ 3 ].replace( QRegExp( "%20" ), " " )).toUInt();

	return new IRCContact( server, target, port, joinOnConnect, new IRCServerContact(this, true), parent, protocolID );
	//FIXME connectNow in IRCServerContact should not be hardwired to true
	//FIXME is new IRCServerContact good thing?
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



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

