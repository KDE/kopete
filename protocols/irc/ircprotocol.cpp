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



#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>

#include <qcursor.h>
#include <qregexp.h>

#include "ircprotocol.h"

#include "ircadd.h"
#include "ircaddcontactpage.h"
#include "ircchatview.h"
#include "irccontact.h"
#include "ircmessage.h"
#include "ircpreferences.h"
#include "ircservercontact.h"
#include "ircservermanager.h"
#include "ircchatwindow.h"
#include "ircconsoleview.h"
#include "tabcompleter.h"
#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetewindow.h"
#include "statusbaricon.h"


///////////////////////////////////////////////////
//           Constructor & Destructor
///////////////////////////////////////////////////

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol::IRCProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	kdDebug() << "IRCProtocol::IRCProtocol"<<endl;
	// Load all ICQ icons from KDE standard dirs
	setIcon( "irc_protocol_small" );

	initIcons();
	m_serverManager = new IRCServerManager();
	statusBarIcon = new StatusBarIcon();

	statusBarIcon->setPixmap(protocolSmallIcon);
	connect(statusBarIcon, SIGNAL(rightClicked(const QPoint&)), this, SLOT(slotIconRightClicked(const QPoint&)));

	new IRCPreferences("irc_protocol", this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
		KMessageBox::sorry(kopeteapp->mainWindow(), i18n("<qt>You haven't setup your IRC settings for the first time. Please do so by going to File->Configure Kopete->IRC Plugin. Once you have done that, try connecting again.</qt>"), i18n("Preferences Nonexistent"));
		return;
	}

	KConfig *cfg = KGlobal::config();
	cfg->setGroup("IRC");
	QString listVersion=cfg->readEntry( "ContactList Version", "0.4.x" ) ;
	if ( listVersion=="0.4.x" )
	{
		kdDebug() << "IRCProtocol::IRCProtocol: import contact from kopete 0.4.x" << endl;
		importOldContactList();
		cfg->setGroup("IRC");
		cfg->writeEntry ( "ContactList Version", "0.5" );
	}


	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->readBoolEntry("HideConsole", false) == false)
	{
		slotNewConsole();
	}

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry( "AutoConnect", false ) )
	{
		Connect();
	}
}

IRCProtocol::~IRCProtocol()
{

}

void IRCProtocol::slotIconRightClicked(const QPoint &)
{
	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle("IRC");
	popup->insertItem(i18n("Open New IRC Console"), this, SLOT(slotNewConsole()));
	popup->popup(QCursor::pos());
}

void IRCProtocol::slotNewConsole()
{
	kdDebug() << "IRCProtocol::slotNewConsole";
	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString server = KGlobal::config()->readEntry("Server", "(Console)");
	QString serverAndNick = nick+"@"+server;

	IRCServerContact *serverContact = m_serverManager->findServer(serverAndNick);

	if(serverContact)
	{
		serverContact->chatWindow()->show();
		serverContact->consoleView()->messageEdit()->setFocus();
	}
	else
	{
		m_serverManager->addServer(serverAndNick, true, this);
	}

}

void IRCProtocol::addContact(  const QString &server, const QString &contact, bool connectNow, bool joinNow, KopeteMetaContact *meta)
{
	QString protocolID = this->id();
	QString contactID=contact+"@"+server;

	KopeteMetaContact *m=KopeteContactList::contactList()->findContact( protocolID, QString::null, contactID);
	if(m)
	{
		kdDebug() << "IRCProtocol::slotContactList: Warnign: "
			<< "Contact already exists " << contactID << endl;
		if(m->isTemporary())
			m->setTemporary(false);
		return;
	}
	
	if (meta)
		m=meta;
	else
	{
		m = new KopeteMetaContact();
		KopeteContactList::contactList()->addMetaContact(m);
	}

	if(contact[0]=='#')
	{ //if it is a channel, add to the server metaContact (this is an idea of Nick)
		if(m->displayName().isEmpty())
		{
			m->setDisplayName(server);
		}
	}

	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString serverAndNick = nick;
	serverAndNick.append("@");
	serverAndNick.append(server);

	IRCServerContact *serverContact = m_serverManager->findServer(serverAndNick);
	if (serverContact != 0)
	{
		m->addContact( new IRCContact(server, contact, 0, joinNow, serverContact, m, protocolID));
	}
	else
	{
		IRCServerContact *serverItem = m_serverManager->addServer(serverAndNick, connectNow, this);
		if (serverItem != 0)
		{
			m->addContact(new IRCContact(server, contact, 0, joinNow, serverItem, m, protocolID));
		}
	}
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


bool IRCProtocol::serialize(KopeteMetaContact * metaContact, QStringList & strList) const
{
	bool r=false;

	QPtrList<KopeteContact> contacts = metaContact->contacts();
	for( 	KopeteContact *c = contacts.first(); c ; c = contacts.next() )
	{
		if ( c->protocol() != this->id() ) // not our contact, next one please
				continue;

		IRCContact *g = static_cast<IRCContact*>(c);

		if( g )
		{
			strList << g->id() << g->displayName();
			r=true;
		}
	}
	return r;
}

void IRCProtocol::deserialize( KopeteMetaContact *metaContact, const QStringList &strList )
{
	unsigned int idx = 0;
	while( idx < strList.size() )
	{
		QStringList cID  = QStringList::split( "@", strList[ idx ] );
		QString displayName = strList[ idx + 1 ];
		if(cID.size()!=2)
		{
			kdDebug() << "IRCProtocol::deserialize : WARNING: bad id '" << strList[ idx ] << "' for " << displayName <<endl;
			break;
		}

		addContact( cID[1], cID[0], false, false,metaContact);

		idx += 2;
	}


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

void IRCProtocol::importOldContactList()
{
	//this code import contact list from old irc plugin (Kopete 0.4.x)
	QString filename = locateLocal("data", "kopete/irc.buddylist");
	KSimpleConfig *m_config = new KSimpleConfig(filename);

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
		KopeteMetaContact *m = new KopeteMetaContact();
		m->addToGroup(groupName);
		addContact( server, (*it), false, false,m);
		KopeteContactList::contactList()->addMetaContact(m);
	}
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


