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

// QT Includes

// KDE Includes
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

// Kopete Includes
#include "kopeteaccountmanager.h"
#include "kopeteuiglobal.h"

// Local Includes
#include "wpprotocol.h"
#include "wpdebug.h"
#include "wpeditaccount.h"
#include "wpaccount.h"

class KPopupMenu;

WPProtocol *WPProtocol::sProtocol = 0;

typedef KGenericFactory<WPProtocol> WPProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_wp, WPProtocolFactory( "kopete_wp" )  )

// WP Protocol
WPProtocol::WPProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: KopeteProtocol( WPProtocolFactory::instance(), parent, name ),
	WPOnline(  KopeteOnlineStatus::Online,  25, this, 0,  QString::null,    i18n( "Go O&nline" ),   i18n( "Online" ) ),
	WPAway(    KopeteOnlineStatus::Away,    20, this, 1,  "wp_away",      i18n( "Go &Away" ),     i18n( "Away" ) ),
	WPOffline( KopeteOnlineStatus::Offline, 0,  this, 2,  QString::null,   i18n( "Go O&ffline" ),  i18n( "Offline" ) )
{
	DEBUG(WPDMETHOD, "WPProtocol::WPProtocol()");

	sProtocol = this;

	// Load Status Actions
//	initActions();
	// TODO: Maybe use this in the future?

	// Set up initial settings
	KGlobal::config()->setGroup("WinPopup");
	QString theSMBClientPath = KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient");
	QString theInitialSearchHost = KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1");
	int theHostCheckFrequency = KGlobal::config()->readNumEntry("HostCheckFrequency", 60);
	int theMessageCheckFrequency = KGlobal::config()->readNumEntry("MessageCheckFrequency", 5);
	KGlobal::config()->writeEntry("SMBClientPath", theSMBClientPath);
	KGlobal::config()->writeEntry("InitialSearchHost", theInitialSearchHost);
	KGlobal::config()->writeEntry("HostCheckFrequency", theHostCheckFrequency);
	KGlobal::config()->writeEntry("MessageCheckFrequency", theMessageCheckFrequency);

	// Call slotSettingsChanged() to get it all registered.
	slotSettingsChanged();

	connect(this, SIGNAL(settingsChanged()), SLOT(slotSettingsChanged()));

	addAddressBookField( "messaging/winpopup", KopetePlugin::MakeIndexField );
}

// Destructor
WPProtocol::~WPProtocol()
{
	DEBUG(WPDMETHOD, "WPProtocol::~WPProtocol()");

	sProtocol = 0L;
}

AddContactPage *WPProtocol::createAddContactWidget(QWidget *parent, KopeteAccount *theAccount)
{
	DEBUG(WPDMETHOD, "WPProtocol::~createAddContactWidget(<parent>, " << theAccount << ")");

	return new WPAddContact(this, dynamic_cast<WPAccount *>(theAccount), parent);
}

KopeteWinPopup *WPProtocol::createInterface(const QString &theHostName)
{
	KGlobal::config()->setGroup("WinPopup");
	QString theSMBClientPath = KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient");
	QString theInitialSearchHost = KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1");
	int theHostCheckFrequency = KGlobal::config()->readNumEntry("HostCheckFrequency", 60);
	int theMessageCheckFrequency = KGlobal::config()->readNumEntry("MessageCheckFrequency", 5);
	KopeteWinPopup *newOne = new KopeteWinPopup(theSMBClientPath, theInitialSearchHost, theHostName, theHostCheckFrequency, theMessageCheckFrequency);
	theInterfaces.append(newOne);
	return newOne;
}

void WPProtocol::destroyInterface(KopeteWinPopup *theInterface)
{
	theInterfaces.removeRef(theInterface);
	delete theInterface;
}

KopeteContact *WPProtocol::deserializeContact( KopeteMetaContact *metaContact,
	const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];

	WPAccount *theAccount = static_cast<WPAccount *>(KopeteAccountManager::manager()->findAccount(protocol()->pluginId(), accountId));
	if(!theAccount)
	{	DEBUG(WPDINFO, "Account " << accountId << " not found");
		return 0;
	}

	if(theAccount->contacts()[contactId])
	{	DEBUG(WPDINFO, "User " << contactId << " already in contacts map");
		return 0;
	}

	theAccount->addContact(contactId, serializedData["displayName"], metaContact, KopeteAccount::DontChangeKABC, serializedData["group"]);
	return theAccount->contacts()[contactId];
}

KopeteEditAccountWidget *WPProtocol::createEditAccountWidget(KopeteAccount *account, QWidget *parent)
{
	return new WPEditAccount(this, account, parent);
}

KopeteAccount *WPProtocol::createNewAccount(const QString &accountId)
{
	return new WPAccount(this, accountId);
}

void WPProtocol::slotSettingsChanged()
{
	DEBUG(WPDMETHOD, "WPProtocol::slotSettingsChanged()");

	KGlobal::config()->setGroup("WinPopup");
	for(KopeteWinPopup *i = theInterfaces.first(); i != theInterfaces.last(); i = theInterfaces.next())
	{	i->setSMBClientPath(KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient"));
		i->setInitialSearchHost(KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1"));
		i->setHostCheckFrequency(KGlobal::config()->readNumEntry("HostCheckFrequency", 60));
		i->setMessageCheckFrequency(KGlobal::config()->readNumEntry("MessageCheckFrequency", 5));
	}
}

void WPProtocol::installSamba()
{
	DEBUG(WPDMETHOD, "WPProtocol::installSamba()");

	QStringList args;
	args += KStandardDirs::findExe("winpopup-install.sh");
	args += KStandardDirs::findExe("winpopup-send.sh");
	if (KApplication::kdeinitExecWait("kdesu", args) == 0)
		KMessageBox::information(Kopete::UI::Global::mainWidget(), i18n("The Samba configuration file is modified."), i18n("Configuration Succeeded"));
	else
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Updating the Samba configuration file failed."), i18n("Configuration Failed"));
}

#include "wpprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

