/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */


#include <kgenericfactory.h>
#include <kdebug.h>

#include "aimprotocol.h"
#include "aimaccount.h"
#include "aimaddcontactpage.h"
#include "aimeditaccountwidget.h"

#include "kopeteaccountmanager.h"
#include "aimcontact.h"
#include "oscarpreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_aim, KGenericFactory<AIMProtocol> );


AIMProtocol* AIMProtocol::protocolStatic_ = 0L;

AIMProtocol::AIMProtocol(QObject *parent, const char *name, const QStringList &)
	: KopeteProtocol(parent,name),
	statusOnline(KopeteOnlineStatus::Online, 1, this, OSCAR_ONLINE, "aim_online", i18n("Online"), i18n("Online")),
	statusOffline(KopeteOnlineStatus::Offline, 1, this, OSCAR_OFFLINE, "aim_offline", i18n("Offline"), i18n("Offline")),
	statusAway(KopeteOnlineStatus::Away, 1, this, OSCAR_AWAY, "aim_away", i18n("Away"), i18n("Away")),
	statusConnecting(KopeteOnlineStatus::Connecting, 99, this, OSCAR_CONNECTING, "aim_connecting", i18n("Connecting..."), i18n("Connecting..."))
{
	if (protocolStatic_)
		kdDebug(14190) << k_funcinfo << "AIM plugin already initialized" << endl;
	else
	{
		protocolStatic_ = this;
		// Create the config widget, this does it's magic I think
		new OscarPreferences("aim_protocol", this);
	}
	addAddressBookField("messaging/aim", KopetePlugin::MakeIndexField);
}

AIMProtocol::~AIMProtocol()
{
}

AIMProtocol *AIMProtocol::protocol(void)
{
	return protocolStatic_;
}

void AIMProtocol::deserializeContact(KopeteMetaContact *metaContact,
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
		// TODO: Remove this, we should be using pluginData() now
		KGlobal::config()->setGroup("OSCAR");
		// Set the account ID to whatever the old single account used to be
		accountId = KGlobal::config()->readEntry( "UserID", "" );
	}

	// Get the account it belongs to
	KopeteAccount *account = accounts[accountId];
	if(!account)
	{
		kdDebug(14190) << k_funcinfo << "Account '" <<
			accountId << "' was not found, creating new account..." << endl;

		if(!accountId.isEmpty())
			account = createNewAccount(accountId);
		else
			return;

		if (!account)
			kdDebug(14190) << k_funcinfo << "Error creating new account" << endl;
	}

	// Create Oscar contact, this adds it to the proper account
	//TODO
	new AIMContact(contactId, displayName, static_cast<AIMAccount*>(account), metaContact);

}

AddContactPage *AIMProtocol::createAddContactWidget(QWidget *parent, KopeteAccount *account)
{
	return (new AIMAddContactPage(account->isConnected(), parent));
}

EditAccountWidget *AIMProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return (new AIMEditAccountWidget(this, account, parent));
}

KopeteAccount *AIMProtocol::createNewAccount(const QString &accountId)
{
	return (new AIMAccount(this, accountId));
}

#include "aimprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
