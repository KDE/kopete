/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
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
#include <kconfig.h>
#include <kmessagebox.h>

#include "kopeteaccountmanager.h"

#include "smsprotocol.h"
#include "smseditaccountwidget.h"
#include "smscontact.h"
#include "smsaddcontactpage.h"
#include "smsaccount.h"

typedef KGenericFactory<SMSProtocol> SMSProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_sms, SMSProtocolFactory( "kopete_sms" )  )

SMSProtocol* SMSProtocol::s_protocol = 0L;

SMSProtocol::SMSProtocol(QObject *parent, const char *name, const QStringList &/*args*/)
: KopeteProtocol( SMSProtocolFactory::instance(), parent, name ),
	SMSOnline(  KopeteOnlineStatus::Online,  25, this, 0,  QString::null,  i18n( "Go O&nline" ),   i18n( "Online" ) ),
	SMSUnknown( KopeteOnlineStatus::Unknown, 25, this, 1,  QString::null, "FIXME: Make optional", i18n( "Unknown" ) ),
	SMSOffline( KopeteOnlineStatus::Offline, 25, this, 2,  QString::null, i18n( "Go O&ffline" ),  i18n( "Offline" ) )
{
	if (s_protocol)
		kdWarning( 14160 ) << k_funcinfo << "s_protocol already defined!" << endl;
	else
		s_protocol = this;

	addAddressBookField("messaging/sms", KopetePlugin::MakeIndexField);
}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

AddContactPage *SMSProtocol::createAddContactWidget(QWidget *parent, KopeteAccount */*i*/)
{
	return new SMSAddContactPage(parent);
}

KopeteEditAccountWidget* SMSProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new SMSEditAccountWidget(this, account, parent);
}

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

KopeteContact *SMSProtocol::deserializeContact(KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData["contactId"];
	QString accountId = serializedData["accountId"];
	QString displayName = serializedData["displayName"];

	QDict<KopeteAccount> accounts=KopeteAccountManager::manager()->accounts(this);

	KopeteAccount *account = accounts[accountId];
	if (!account)
	{
		kdDebug(14160) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	return new SMSContact(account, contactId, displayName, metaContact);
}

KopeteAccount* SMSProtocol::createNewAccount(const QString &accountId)
{
	return new SMSAccount(this, accountId);
}

#include "smsprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

