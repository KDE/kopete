/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "smsprotocol.h"
#include "smseditaccountwidget.h"
#include "smscontact.h"
#include "smsaddcontactpage.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "smsaccount.h"
#include "smspreferences.h"
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kconfig.h>


K_EXPORT_COMPONENT_FACTORY( kopete_sms, KGenericFactory<SMSProtocol> );

SMSProtocol::SMSProtocol( QObject *parent, const char *name, const QStringList& /*args*/)
: KopeteProtocol( parent, name ),
	SMSOnline(  KopeteOnlineStatus::Online,  25, this, 0,  "sms_online",  i18n( "Go O&nline" ),   i18n( "Online" ) ),
	SMSUnknown( KopeteOnlineStatus::Unknown, 25, this, 1,  "sms_unknown", "FIXME: Make optional", i18n( "Unknown" ) ),
	SMSOffline( KopeteOnlineStatus::Offline, 25, this, 2,  "sms_offline", i18n( "Go O&ffline" ),  i18n( "Offline" ) )
{
	if( s_protocol )
		kdWarning( 14160 ) << k_funcinfo << "s_protocol already defined!" << endl;
	else
		s_protocol = this;

	new SMSPreferences("sms_protocol", this);

	QString protocolId = pluginId();

	addAddressBookField( "messaging/sms", KopetePlugin::MakeIndexField );
}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

AddContactPage *SMSProtocol::createAddContactWidget(QWidget *parent, KopeteAccount * /*i*/)
{
	return (new SMSAddContactPage(parent));
}

EditAccountWidget* SMSProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new SMSEditAccountWidget(account, parent);
}

SMSProtocol* SMSProtocol::s_protocol = 0L;

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

void SMSProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	QString displayName = serializedData[ "displayName" ];

	QDict<KopeteAccount> accounts=KopeteAccountManager::manager()->accounts(this);

	if(accountId.isNull() && accounts.count() == 0) // FIXME Is this the right way?
	{
		// FIXME import the old one correct (KConfUpdate)
		KGlobal::config()->setGroup("SMS");
		accountId=KGlobal::config()->readEntry( "serviceName" );
	}

	KopeteAccount *account = accounts[accountId];
	if ( !account )
		account = createNewAccount( accountId );
	
	/*SMSContact* c =*/ new SMSContact( account, contactId, displayName, metaContact );
}

KopeteAccount* SMSProtocol::createNewAccount(const QString &accountId)
{
	return new SMSAccount(this, accountId);
}
#include "smsprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

