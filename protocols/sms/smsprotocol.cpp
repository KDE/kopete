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
#include "smspreferences.h"

typedef KGenericFactory<SMSProtocol> SMSProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_sms, SMSProtocolFactory( "kopete_sms" )  );

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

	SMSPreferences *p = new SMSPreferences("sms_protocol", this);
	connect(p, SIGNAL(saved()), this, SLOT(loadConfig()));
	loadConfig();

	addAddressBookField("messaging/sms", KopetePlugin::MakeIndexField);
}

SMSProtocol::~SMSProtocol()
{
	s_protocol = 0L;
}

void SMSProtocol::loadConfig()
{
	KGlobal::config()->setGroup("SMS");
	theSubEnable = KGlobal::config()->readBoolEntry("SubEnable", false);
	theSubCode = KGlobal::config()->readEntry("SubCode", "+44");
	theLongMsgAction = (SMSMsgAction)KGlobal::config()->readNumEntry("MsgAction", ACT_ASK);
}

void SMSProtocol::translateNumber(QString &theNumber)
{
	if(theNumber[0] == QChar('0') && theSubEnable)
		theNumber.replace(0, 1, theSubCode);
}

AddContactPage *SMSProtocol::createAddContactWidget(QWidget *parent, KopeteAccount */*i*/)
{
	return new SMSAddContactPage(parent);
}

EditAccountWidget* SMSProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new SMSEditAccountWidget(this, account, parent);
}

SMSProtocol* SMSProtocol::protocol()
{
	return s_protocol;
}

const bool SMSProtocol::splitNowMsgTooLong(int max, int msgLength)
{
	if(theLongMsgAction == ACT_CANCEL) return false;
	if(theLongMsgAction == ACT_SPLIT) return true;
	if(KMessageBox::questionYesNo(0L, i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?").arg(max).arg(msgLength / max + 1),
		i18n("Message Too Long")) == KMessageBox::Yes)
		return true;
	else
		return false;
}

void SMSProtocol::deserializeContact(KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
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
		return;
	}

	/*SMSContact* c =*/ new SMSContact(account, contactId, displayName, metaContact);
}

KopeteAccount* SMSProtocol::createNewAccount(const QString &accountId)
{
	return new SMSAccount(this, accountId);
}

#include "smsprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

