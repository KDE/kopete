/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Olivier Goffart <ogoffart@tiscalinet.be>

  Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqcontact.h"
#include "icqaddcontactpage.h"
#include "icqeditaccountwidget.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>

#include "oscarpreferences.h"
#include "oscarsocket.h"


K_EXPORT_COMPONENT_FACTORY( kopete_icq, KGenericFactory<ICQProtocol> );

ICQProtocol* ICQProtocol::protocolStatic_ = 0L;

ICQProtocol::ICQProtocol(QObject *parent, const char *name, const QStringList&)
	: KopeteProtocol(parent,name),
	statusOnline(KopeteOnlineStatus::Online, 1, this, OSCAR_ONLINE,"icq_online" , i18n("Online"), i18n("Online")),
	statusFFC(KopeteOnlineStatus::Online, 2, this, OSCAR_FFC, "icq_ffc", i18n("&Free For Chat"), i18n("Free For Chat")),
	statusOffline(KopeteOnlineStatus::Offline, 1, this, OSCAR_OFFLINE, "icq_offline", i18n("Offline"), i18n("Offline")),
	statusAway(KopeteOnlineStatus::Away, 1, this, OSCAR_AWAY, "icq_away", i18n("Away"), i18n("Away")),
	statusDND(KopeteOnlineStatus::Away, 2, this, OSCAR_DND, "icq_dnd", i18n("&Do Not Disturb"), i18n("Do not Disturb")),
	statusNA(KopeteOnlineStatus::Away, 3, this, OSCAR_NA, "icq_na", i18n("Not A&vailable"), i18n("Not Available")),
	statusOCC(KopeteOnlineStatus::Away, 4, this, OSCAR_OCC,"icq_occupied" , i18n("O&ccupied"), i18n("Occupied")),
	statusConnecting(KopeteOnlineStatus::Connecting, 99, this, OSCAR_CONNECTING, "icq_connecting", i18n("Connecting..."), i18n("Connecting..."))

{
	if (protocolStatic_)
		kdDebug(14200) << k_funcinfo << "ICQ plugin already initialized" << endl;
	else
	{
		protocolStatic_ = this;
		// Create the config widget, this does it's magic I think
		new OscarPreferences("icq_protocol", this);
	}
	addAddressBookField("messaging/icq", KopetePlugin::MakeIndexField);
}

// Called when we want to return the active instance of the protocol
ICQProtocol *ICQProtocol::protocol()
{
	return protocolStatic_;
}

bool ICQProtocol::canSendOffline() const
{
	return true;
}

// This will be called when Kopete reads the contact list
void ICQProtocol::deserializeContact(KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/*addressBookData*/)
{
	QString contactId=serializedData["contactId"];
	QString accountId=serializedData["accountId"];
	QString displayName=serializedData["displayName"];

	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts(this);


	if(accountId.isNull())
	{
		//Kopete 0.6.x contactlist
		KGlobal::config()->setGroup("ICQ");
		// Set the account ID to whatever the old single account used to be
 		accountId = KGlobal::config()->readEntry("UIN", "");
	}

	// Get the account it belongs to
	KopeteAccount *account = accounts[accountId];
	if(!account)
	{
		kdDebug(14200) << k_funcinfo << "Account '" <<
			accountId << "' was not found, creating new account..." << endl;

		if(!accountId.isEmpty())
			account = createNewAccount(accountId);
		else
			return;
		// FIXME: What is account failed?
		if (!account)
			kdDebug(14200) << k_funcinfo << "Error creating new account" << endl;
	}

	new ICQContact(contactId, displayName, static_cast<ICQAccount*>(account), metaContact);
}

AddContactPage *ICQProtocol::createAddContactWidget(QWidget *parent, KopeteAccount *account)
{
	return (new ICQAddContactPage(static_cast<ICQAccount*>(account) , parent));
}

EditAccountWidget *ICQProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return (new ICQEditAccountWidget(this, account, parent));
}

KopeteAccount *ICQProtocol::createNewAccount(const QString &accountId)
{
	return (new ICQAccount(this, accountId));
}

#include "icqprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
