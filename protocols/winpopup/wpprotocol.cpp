/***************************************************************************
                          wpprotocol.cpp  -  WP Plugin
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wpprotocol.h"

// QT Includes
#include <QMap>
#include <QList>
#include <QHash>

// KDE Includes
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>

// Kopete Includes
#include "kopeteaccountmanager.h"
#include "kopeteuiglobal.h"

// Local Includes
#include "wpeditaccount.h"
#include "wpaccount.h"
#include "wpcontact.h"

class KMenu;

WPProtocol *WPProtocol::sProtocol = 0;

K_PLUGIN_FACTORY( WPProtocolFactory, registerPlugin<WPProtocol>(); )
K_EXPORT_PLUGIN( WPProtocolFactory( "kopete_wp" ) )

// WP Protocol
WPProtocol::WPProtocol( QObject *parent, const QVariantList & /* args */ )
: Kopete::Protocol( WPProtocolFactory::componentData(), parent ),
	WPOnline(  Kopete::OnlineStatus::Online,  25, this, 0,  QStringList(), i18n("Online"),  i18n("Online")),
	WPAway(    Kopete::OnlineStatus::Away,    20, this, 1,  QStringList(QString::fromLatin1("wp_away")),     i18n("Away"),    i18n("Away")),
	WPOffline( Kopete::OnlineStatus::Offline, 0,  this, 2,  QStringList(), i18n("Offline"), i18n("Offline"))
{
//	kDebug(14170) << "WPProtocol::WPProtocol()";

	sProtocol = this;

	// Load Status Actions
//	initActions();
	// TODO: Maybe use this in the future?

	addAddressBookField( "messaging/winpopup", Kopete::Plugin::MakeIndexField );

	readConfig();

	popupClient = new WinPopupLib(smbClientBin, groupCheckFreq);
	QObject::connect(popupClient, SIGNAL(signalNewMessage(QString,QDateTime,QString)),
		this, SLOT(slotReceivedMessage(QString,QDateTime,QString)));
}

// Destructor
WPProtocol::~WPProtocol()
{
//	kDebug(14170) <<  "WPProtocol::~WPProtocol()";

	sProtocol = 0;
}

AddContactPage *WPProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *theAccount)
{
//	kDebug(14170) << "WPProtocol::createAddContactWidget(<parent>, " << theAccount << ")";

	return new WPAddContact(parent, dynamic_cast<WPAccount *>(theAccount));
}

Kopete::Contact *WPProtocol::deserializeContact( Kopete::MetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

	WPAccount *theAccount = static_cast<WPAccount *>(Kopete::AccountManager::self()->findAccount(protocol()->pluginId(), accountId));
	if(!theAccount)	{
		kDebug(14170) <<  "Account " << accountId << " not found";
		return 0;
	}

	if(theAccount->contacts().value(contactId)) {
		kDebug(14170) << "User " << contactId << " already in contacts map";
		return 0;
	}

	theAccount->addContact(contactId, metaContact, Kopete::Account::DontChangeKABC);

	Kopete::Contact *c = theAccount->contacts().value(contactId);
	if (!c)
		return 0;

	c->setPreferredNameType(nameType);
	return c;
}

KopeteEditAccountWidget *WPProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new WPEditAccount(parent, account);
}

Kopete::Account *WPProtocol::createNewAccount(const QString &accountId)
{
	return new WPAccount(this, accountId);
}

void WPProtocol::settingsChanged()
{
	kDebug(14170) <<  "WPProtocol::slotSettingsChanged()";

	readConfig();
	popupClient->settingsChanged(smbClientBin, groupCheckFreq);
}

void WPProtocol::readConfig()
{
	KConfigGroup group = KGlobal::config()->group("WinPopup");
	smbClientBin = group.readEntry("SmbcPath", "/usr/bin/smbclient");
	groupCheckFreq = group.readEntry("HostCheckFreq", 60);
}

void WPProtocol::installSamba()
{
//	kDebug(14170) <<  "WPProtocol::installSamba()" endl;

	QStringList args;
	args += KStandardDirs::findExe("winpopup-install");
	args += KStandardDirs::findExe("winpopup-send");
	if (KToolInvocation::kdeinitExecWait("kdesu", args) == 0)
		KMessageBox::information(Kopete::UI::Global::mainWidget(), i18n("The Samba configuration file has been modified."), i18n("Configuration Successful"));
	else
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Updating the Samba configuration file failed."), i18n("Configuration Failed"));
}

/**
 * search the contact for the new message and give it to its account
 */
void WPProtocol::slotReceivedMessage(const QString &Body, const QDateTime &Time, const QString &From)
{
	bool foundContact = false;
	QList<Kopete::Account*> Accounts = Kopete::AccountManager::self()->accounts(protocol());
	Kopete::Account *theAccount = 0;
	foreach(Kopete::Account *account, Accounts) {
		Kopete::Contact *theContact = account->contacts().value(From);
		if (theContact) {
			foundContact = true;
			theAccount = account;
			dynamic_cast<WPAccount *>(account)->slotGotNewMessage(Body, Time, From);
			break;
		}
	}

	// What to do with messages with no contact?
	// Maybe send them to the next online account? GF
	if (!foundContact) {
		if (theAccount)
			dynamic_cast<WPAccount *>(theAccount)->slotGotNewMessage(Body, Time, From);
		else
			kDebug(14170) << "No contact or connected account found!";
	}
}

void WPProtocol::sendMessage(const QString &Body, const QString &Destination)
{
	popupClient->sendMessage(Body, Destination);
}

#include "wpprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
