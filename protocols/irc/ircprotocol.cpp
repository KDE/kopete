/*
    ircprotocol - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

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
#include "ircpreferences.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "ircchannelcontact.h"
#include "kirc.h"

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol::IRCProtocol( QObject *parent, const char *name,
	const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	m_actionMenu = 0L;
	actionConnect = new KAction ( i18n("Online"), "", 0, this, SLOT(connect()), this, "actionIRCConnect" );
	actionDisconnect = new KAction ( i18n("Offline"), "", 0, this, SLOT(disconnect()), this, "actionIRCDisconnect" );

	kdDebug(14120) << k_funcinfo << endl;
	// Load all ICQ icons from KDE standard dirs

	setStatusIcon( "irc_protocol_offline" );

	new IRCPreferences("irc_protocol", this);

	KConfig *cfg = KGlobal::config();
	cfg->setGroup("IRC");
	identity = new IRCIdentity(cfg->readEntry("Server", "irc.freenode.net"), cfg->readEntry("Port", "6667").toUInt(), cfg->readEntry("Nickname", "KopeteUser"), cfg->readEntry("Password", ""), this);
	QObject::connect(identity->engine(), SIGNAL(connectedToServer()), this, SLOT(slotConnectedToServer()));
	QObject::connect(identity->engine(), SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));

	/** Autoconnect if is selected in config */
	if ( cfg->readBoolEntry( "AutoConnect", false ) )
		connect();
}

IRCProtocol::~IRCProtocol()
{
	delete identity;
}

KActionMenu* IRCProtocol::protocolActions()
{
	if (!m_actionMenu)
	{
		m_actionMenu = new KActionMenu( "IRC", this );
		m_actionMenu->popupMenu()->insertTitle(SmallIcon( "irc_protocol_small" ), i18n( "IRC" ) );
		m_actionMenu->insert(actionConnect);
		m_actionMenu->insert(actionDisconnect);
	}
	return m_actionMenu;
}

const QString IRCProtocol::protocolIcon()
{
	return "irc_protocol_small";
}

void IRCProtocol::addContact(  const QString &server, const QString &contact, bool isChannel, KopeteMetaContact *m)
{
	kdDebug(14120) << "[IRCProtocol] addContact called" << endl;
	IRCContact *c;

	if( !m )
	{
		m = new KopeteMetaContact();
		KopeteContactList::contactList()->addMetaContact(m);
	}

	if (isChannel)
		c = static_cast<IRCContact*>( identity->findChannel(contact, m) );
	else
		c = static_cast<IRCContact*>( identity->findUser(contact, m) );


	if( c->metaContact()->isTemporary() )
		c->metaContact()->setTemporary(false);

	if( identity->engine()->state() == QSocket::Connected )
		c->setOnlineStatus( KopeteContact::Online );
}

void IRCProtocol::slotConnectedToServer()
{
	setStatusIcon( "irc_protocol_small" );
}

void IRCProtocol::slotConnectionClosed()
{
	setStatusIcon( "irc_protocol_offline" );
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
	KConfig *cfg = KGlobal::config();
	cfg->setGroup("IRC");
	identity->engine()->connectToServer(cfg->readEntry("Nickname", "KopeteUser"));
}

void IRCProtocol::disconnect()
{
 	identity->engine()->quitIRC("Kopete IRC 2.0. http://kopete.kde.org");
}


bool IRCProtocol::isConnected() const
{
	return identity->engine()->isLoggedIn();
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
/* TODO: FIXME:*/

	QString contactId = serializedData[ "contactId" ];
	if( !contacts()[ contactId ] )
	{
		QString displayName = serializedData[ "displayName" ];
		if( displayName.isEmpty() )
			displayName = contactId;

		addContact( serializedData[ "serverName" ], contactId, true, metaContact );
	}
}

#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

