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

#include "ircprotocol.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qregexp.h>

#include <kaction.h>
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
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol::IRCProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	kdDebug(14120) << k_funcinfo << endl;
	// Load all ICQ icons from KDE standard dirs
	setStatusIcon( "irc_protocol_small" );

	m_actionMenu = new KActionMenu( "IRC", this );
	m_actionMenu->popupMenu()->insertTitle(
		SmallIcon( "irc_protocol_small" ), i18n( "IRC" ) );
	m_actionMenu->insert( new KAction( i18n( "Open New IRC Console" ), 0,
		this, SLOT( slotNewConsole() ), this, "m_newConsoleAction" ) );

	m_serverManager = new IRCServerManager();

	setStatusIcon( "irc_protocol_small" );

	new IRCPreferences("irc_protocol", this);

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->hasKey("Nickname") == false)
	{
//		KMessageBox::sorry( qapp->mainWidget(), i18n("<qt>You haven't setup your IRC settings for the first time. Please do so by going to File->Configure Kopete->IRC Plugin. Once you have done that, try connecting again.</qt>"), i18n("Preferences Nonexistent"));
		return;
	}

	KConfig *cfg = KGlobal::config();
	cfg->setGroup("IRC");
	QString listVersion=cfg->readEntry( "ContactList Version", "0.4.x" ) ;
	if ( listVersion=="0.4.x" )
	{
		kdDebug(14120) << "IRCProtocol::IRCProtocol: import contact from kopete 0.4.x" << endl;
		importOldContactList();
		cfg->setGroup("IRC");
		cfg->writeEntry ( "ContactList Version", "0.5" );
	}

	addAddressBookField( "messaging/irc", KopetePlugin::MakeIndexField );

	KGlobal::config()->setGroup("IRC");
	if (KGlobal::config()->readBoolEntry("HideConsole", false) == false)
		slotNewConsole();

	/** Autoconnect if is selected in config */
	if ( KGlobal::config()->readBoolEntry( "AutoConnect", false ) )
		connect();
}

IRCProtocol::~IRCProtocol()
{

}

KActionMenu* IRCProtocol::protocolActions()
{
	return m_actionMenu;
}

const QString IRCProtocol::protocolIcon()
{
	return "irc_protocol_small";
}

void IRCProtocol::slotNewConsole()
{
	kdDebug(14120) << k_funcinfo << endl;
	
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
		m_serverManager->addServer(serverAndNick, true, this);
}

void IRCProtocol::addContact(  const QString &server, const QString &contact, bool connectNow, bool joinNow, KopeteMetaContact *meta)
{
	QString protocolID = pluginId();
	QString contactID=contact+"@"+server;

	KopeteContact *c = contacts()[contactID];
	KopeteMetaContact *m = 0L;
	
	if (c) m = c->metaContact();
	
	if(m)
	{
		kdWarning(14120) << k_funcinfo << "Contact already exists " << contactID << endl;
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
			m->setDisplayName(contact);
	}

	KGlobal::config()->setGroup("IRC");
	QString nick = KGlobal::config()->readEntry("Nickname", "KopeteUser");
	QString serverAndNick = nick;
	serverAndNick.append("@");
	serverAndNick.append(server);

	IRCServerContact *serverContact = m_serverManager->findServer(serverAndNick);
	if (serverContact != 0)
		m->addContact( new IRCContact(server, contact, 0, joinNow, serverContact, m, this));
	else
	{
		IRCServerContact *serverItem = m_serverManager->addServer(serverAndNick, connectNow, this);
		if (serverItem != 0)
		{
			m->addContact(new IRCContact(server, contact, 0, joinNow, serverItem, m, this));
		}
	}
}


///////////////////////////////////////////////////
//           Plugin Class reimplementation
///////////////////////////////////////////////////

void IRCProtocol::init()
{

}

///////////////////////////////////////////////////
//           KopeteProtocol Class reimplementation
///////////////////////////////////////////////////

void IRCProtocol::connect()
{

}

void IRCProtocol::disconnect()
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

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent)
{
	return (new IRCAddContactPage(this,parent));
}

void IRCProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	// This seems to not be required? Results in a duplicate contact adding warning
	// (Martijn:) yes, it is. If you get dups you are apparently storing your contact
	//            list somewhere else too and not only in Kopete's contactlist.xml

	QString contactId = serializedData[ "contactId" ];
	if( !contacts()[ contactId ] )
	{
		QString displayName = serializedData[ "displayName" ];
		if( displayName.isEmpty() )
			displayName = contactId;

		addContact( serializedData[ "serverName" ], displayName, false, false, metaContact );
	}
}

///////////////////////////////////////////////////
//           Internal functions implementation
///////////////////////////////////////////////////

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
			continue;

		QString server = m_config->readEntry("Server", "");
		if (server.isEmpty())
		{
			KGlobal::config()->setGroup("IRC");
			server = KGlobal::config()->readEntry("Server", "irc.unknown.com");
		}
		KopeteMetaContact *m = new KopeteMetaContact();
//		m->addToGroup(groupName);
		addContact( server, (*it), false, false,m);
		KopeteContactList::contactList()->addMetaContact(m);
	}
}

#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

