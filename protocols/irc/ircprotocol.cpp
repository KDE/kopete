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
#include <qdict.h>
#include <dom/html_element.h>
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
#include "ircaccount.h"
#include "ircaddcontactpage.h"
#include "ircpreferences.h"
#include "kopetemetacontact.h"
#include "ircchannelcontact.h"
#include "kopeteaccountmanager.h"
#include "irceditaccountwidget.h"
#include "kopetemessagemanagerfactory.h"
#include "kactioncollection.h"
#include "kirc.h"
#include "ksparser.h"

K_EXPORT_COMPONENT_FACTORY( kopete_irc, KGenericFactory<IRCProtocol> );

IRCProtocol *IRCProtocol::s_protocol = 0L;

KopeteOnlineStatus IRCProtocol::m_ChannelOnline;
KopeteOnlineStatus IRCProtocol::m_ChannelOffline;
KopeteOnlineStatus IRCProtocol::m_UserOnline;
KopeteOnlineStatus IRCProtocol::m_UserOffline;

IRCProtocol::IRCProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: KopeteProtocol( parent, name )
{
	s_protocol = this;
	mActions = 0L;

	kdDebug(14120) << k_funcinfo << endl;
	// Load all ICQ icons from KDE standard dirs

	m_ChannelOnline = KopeteOnlineStatus( KopeteOnlineStatus::Online,  25, this, 0, "irc_protocol",   i18n( "Go O&nline" ),  i18n( "Online" ));
	m_ChannelOffline = KopeteOnlineStatus( KopeteOnlineStatus::Offline, 25, this, 1, "irc_protocol_offline", i18n( "Go O&ffline" ), i18n( "Offline" ));

	m_UserOnline = KopeteOnlineStatus( KopeteOnlineStatus::Online,  25, this, 0, "irc_normal",   i18n( "Go O&nline" ),  i18n( "Online" ));
	m_UserOffline = KopeteOnlineStatus( KopeteOnlineStatus::Offline, 25, this, 1, "irc_offline", i18n( "Go O&ffline" ), i18n( "Offline" ));

	setStatusIcon( "irc_protocol_offline" );

	new IRCPreferences("irc_protocol", this);

	mParser = new KSParser();

	KConfig *cfg = KGlobal::config();
        cfg->setGroup("IRC");

	//Migration code
	if( cfg->hasKey("Nickname") )
	{
		createNewAccount( cfg->readEntry("Nickname") + "@" + cfg->readEntry("Server") + ":" + cfg->readEntry("Port") );

		cfg->deleteEntry("Nickname");
		cfg->deleteEntry("Server");
		cfg->deleteEntry("Port");
		cfg->deleteEntry("AutoConnect");
		cfg->sync();
	}

	QObject::connect( KopeteMessageManagerFactory::factory(), SIGNAL(aboutToDisplay(KopeteMessage &)), this, SLOT(slotMessageFilter(KopeteMessage &)) );
}

IRCProtocol * IRCProtocol::protocol()
{
	return s_protocol;
}

IRCProtocol::~IRCProtocol()
{
	delete mParser;
}

void IRCProtocol::slotMessageFilter( KopeteMessage &msg )
{
	if( msg.from()->protocol()->inherits("IRCProtocol") )
	{
		kdDebug(14120) << k_funcinfo << endl;
		QString messageText = msg.escapedBody();
		kdDebug(14120) << k_funcinfo << messageText << endl;
		//Add right click for channels
		messageText.replace( QRegExp( QString::fromLatin1("(^|[\\W\\s])(#\\w+)") ), QString::fromLatin1("\\1<span class=\"KopeteIRCChannel\" style=\"cursor:pointer;\">\\2</span>") );

		msg.setBody( messageText, KopeteMessage::RichText );
	}
}

KActionCollection *IRCProtocol::customChatWindowPopupActions( const KopeteMessage &m, DOM::Node &n )
{
	delete mActions;
	mActions = 0L;
	DOM::HTMLElement e = n;

	//isNull checks that the cast was successful
	if( !e.isNull() && !m.to().isEmpty() )
	{
		activeNode = n;
		activeAccount = static_cast<IRCAccount*>( m.from()->account() );
		mActions = new KActionCollection(this);
		if( e.className() == QString::fromLatin1("KopeteIRCChannel") )
		{
			new KAction(i18n("Join %1").arg(e.innerText().string()),0,this,SLOT(slotJoinChannel()),mActions);
		}
	}

	return mActions;
}

void IRCProtocol::slotJoinChannel()
{
	DOM::HTMLElement e = activeNode;
	if( !e.isNull() )
		activeAccount->findChannel( e.innerText().string() )->startChat();
}

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent, KopeteAccount *account)
{
	return new IRCAddContactPage(parent,static_cast<IRCAccount*>(account));
}

EditAccountWidget *IRCProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new IRCEditAccountWidget(this, static_cast<IRCAccount*>(account),parent);
}

KopeteAccount *IRCProtocol::createNewAccount(const QString &accountId)
{
	return new IRCAccount( accountId, this );
}

void IRCProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	kdDebug(14120) << k_funcinfo << endl;

	QString contactId = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];

	if( displayName.isEmpty() )
		displayName = contactId;

	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );
	if( !accounts.isEmpty() )
	{
		IRCAccount *a = static_cast<IRCAccount*>( accounts[ serializedData[ "accountId" ] ] );
		if( a )
			a->addContact( contactId, displayName, metaContact );
		else
			kdDebug(14120) << k_funcinfo << serializedData[ "accountId" ] << " was a contact's account,"
				" but we dont have it in the accounts list" << endl;
	}
	else
		kdDebug(14120) << k_funcinfo << "No accounts loaded!" << endl;
}

#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

