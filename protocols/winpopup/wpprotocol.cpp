/***************************************************************************
                          wpprotocol.cpp  -  WP Plugin
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net

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
#include <qcursor.h>
#include <qprocess.h>
#include <qfile.h>
#include <qregexp.h>

// KDE Includes
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>

// Local Includes
#include "wpprotocol.h"
#include "wpdebug.h"
#include "wpcontact.h"
#include "wpaddcontact.h"
#include "wppreferences.h"

// Kopete Includes
#include "kopetemetacontact.h"

class KPopupMenu;

//WPProtocol *WPProtocol::sProtocol = 0;

K_EXPORT_COMPONENT_FACTORY(kopete_wp, KGenericFactory<WPProtocol>);

// WP Protocol
WPProtocol::WPProtocol(QObject *parent, QString name, QStringList) : KopeteProtocol(parent, name)
{
	DEBUG(WPDMETHOD, "WPProtocol::WPProtocol()");

	theInterface = 0;

	// Load Status Actions
	initActions();

	// Set up initial settings
	KGlobal::config()->setGroup("WinPopup");
	QString theSMBClientPath = KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient");
	QString theInitialSearchHost = KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1");
	QString theHostName = KGlobal::config()->readEntry("HostName", "");
	QString theAwayMessage = KGlobal::config()->readEntry("AwayMessage", i18n("Sorry, I'm not here right now."));
	int theHostCheckFrequency = KGlobal::config()->readNumEntry("HostCheckFrequency", 60);
	int theMessageCheckFrequency = KGlobal::config()->readNumEntry("MessageCheckFrequency", 5);
	bool theSendAwayMessage = KGlobal::config()->readBoolEntry("SendAwayMessage", true);
	bool theEmailDefault = KGlobal::config()->readBoolEntry("EmailDefault", true);
	if(theHostName.isEmpty())
	{	QFile infile("/etc/hostname");
		if(infile.open(IO_ReadOnly))
		{	QTextStream in(&infile);
			char c;
			for(in >> c; c != '.' && (!infile.atEnd()); in >> c)
				theHostName = theHostName + char((c >= 65 && c < 91) ? c : (c - 32));
			infile.close();
		}
		else
			theHostName = "LOCALHOST";
	}
	KGlobal::config()->writeEntry("HostName", theHostName);
	KGlobal::config()->writeEntry("SMBClientPath", theSMBClientPath);
	KGlobal::config()->writeEntry("InitialSearchHost", theInitialSearchHost);
	KGlobal::config()->writeEntry("AwayMessage", theAwayMessage);
	KGlobal::config()->writeEntry("SendAwayMessage", theSendAwayMessage);
	KGlobal::config()->writeEntry("EmailDefault", theEmailDefault);
	KGlobal::config()->writeEntry("HostCheckFrequency", theHostCheckFrequency);
	KGlobal::config()->writeEntry("MessageCheckFrequency", theMessageCheckFrequency);

	// Create preferences menu
	mPrefs = new WPPreferences("wp_icon", this);
	QObject::connect( mPrefs, SIGNAL(saved(void)), this, SLOT(slotSettingsChanged(void)));

	// ask for installation.
	if(KMessageBox::questionYesNo(mPrefs, i18n("The Samba configuration file needs to be modified in order for Kopete to receive WinPopup messages. Would you like to do this now?"), i18n("Modify Samba Configuration Now?"), KGuiItem(), KGuiItem(), "WPFirstTime") == KMessageBox::Yes)
		installSamba();

	// Create the interface...
	theInterface = new KopeteWinPopup(theSMBClientPath, theInitialSearchHost, theHostName, theHostCheckFrequency, theMessageCheckFrequency);

	// Call slotSettingsChanged() to get it all registered.
	slotSettingsChanged();

	setAvailable();
	connect();

	// FIXME: I guess 'myself' should be a metacontact as well...
	theMyself = new WPContact(this, theHostName, 0L);		// XXX: Should be from config file!!!
	QObject::connect( theInterface, SIGNAL(newMessage(const QString &, const QDateTime &, const QString &)), this, SLOT(slotGotNewMessage(const QString &, const QDateTime &, const QString &)));

	addAddressBookField( "messaging/winpopup", KopetePlugin::MakeIndexField );
}

// Destructor
WPProtocol::~WPProtocol()
{
	DEBUG(WPDMETHOD, "WPProtocol::~WPProtocol()");
}

void WPProtocol::deserializeContact( KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	new WPContact( this, serializedData[ "contactId" ], metaContact );
}

WPContact *WPProtocol::getContact(const QString &Name, KopeteMetaContact* theMetaContact)
{
	DEBUG(WPDMETHOD, "WPProtocol::getContact(" << Name << ", " << theMetaContact << ")");

	KopeteContactList *l = KopeteContactList::contactList();

	if(!theMetaContact)
	{
		// Should really ask to see if they want the contact adding to their list first...
		theMetaContact = l->findContact(pluginId(), Name, Name);
		if(!theMetaContact)
		{	DEBUG(WPDINFO, "Adding " << Name << " to the contact list...");
			theMetaContact = new KopeteMetaContact();
			l->addMetaContact(theMetaContact);
		}
	}

	KopeteContact *theContact = theMetaContact->findContact(pluginId(), Name, Name);
	if(!theContact)
	{	theContact = new WPContact(this, Name, theMetaContact);
		theMetaContact->addContact(theContact);
	}

	return dynamic_cast<WPContact *>(theContact);
}

void WPProtocol::slotGotNewMessage(const QString &Body, const QDateTime &Arrival, const QString &From)
{
	DEBUG(WPDMETHOD, "WPProtocol::slotGotNewMessage(" << Body << ", " << Arrival.toString() << ", " << From << ")");

	if(online)
		if(available)
			getContact(From)->slotNewMessage(Body, Arrival);
		else
		{
			// add message quietly?

			// send away message - TODO: should be taken from global settings
			KGlobal::config()->setGroup("WinPopup");
			theInterface->slotSendMessage(KGlobal::config()->readEntry("AwayMessage"), From);
		}
}

bool WPProtocol::unload()
{
	DEBUG(WPDMETHOD, "WPProtocol::unload()");

	delete theInterface;

	return KopeteProtocol::unload();
}

void WPProtocol::connect()
{
	DEBUG(WPDMETHOD, "WPProtocol::Connect()");

	online = true;
	theInterface->goOnline();
	available = true;
	setStatusIcon( "wp_available" );
}

void WPProtocol::disconnect()
{
	DEBUG(WPDMETHOD, "WPProtocol::Disconnect()");

	online = false;
	theInterface->goOffline();
	setStatusIcon( "wp_offline" );
}

void WPProtocol::setAvailable()
{
	DEBUG(WPDMETHOD, "WPProtocol::setAvailable()");

	online = true;
	theInterface->goOnline();
	available = true;
	setStatusIcon( "wp_available" );
	// do any other stuff?
}

void WPProtocol::setAway()
{
	DEBUG(WPDMETHOD, "WPProtocol::setAway()");

	available = false;
	online = true;
	theInterface->goOnline();
	setStatusIcon( "wp_away" );
	// do any other stuff?
}

const QString WPProtocol::protocolIcon()
{
	return "wp_available";
}

KActionMenu* WPProtocol::protocolActions()
{
	return actionStatusMenu;
}

void WPProtocol::slotSendMessage(const QString &Body, const QString &Destination)
{
	DEBUG(WPDMETHOD, "WPProtocol::slotSendMessage(" << Body << ", " << Destination << ")");

	theInterface->sendMessage(Body, Destination);
}

void WPProtocol::slotSettingsChanged()
{
	DEBUG(WPDMETHOD, "WPProtocol::slotSettingsChanged()");

	KGlobal::config()->setGroup("WinPopup");
	theInterface->setSMBClientPath(KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient"));
	theInterface->setInitialSearchHost(KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1"));
	theInterface->setHostName(KGlobal::config()->readEntry("HostName", "LOCAL"));
	theInterface->setHostCheckFrequency(KGlobal::config()->readNumEntry("HostCheckFrequency", 60));
	theInterface->setMessageCheckFrequency(KGlobal::config()->readNumEntry("MessageCheckFrequency", 5));
}

void WPProtocol::initActions()
{
	DEBUG(WPDMETHOD, "WPProtocol::initActions()");

	actionGoAvailable = new KAction("Online", "wp_available", 0, this, SLOT(connect()), this, "actionGoAvailable");
	actionGoOffline = new KAction("Offline", "wp_offline", 0, this, SLOT(disconnect()), this, "actionGoOffline");
	actionGoAway = new KAction("Away", "wp_away", 0, this, SLOT(setAway()), this, "actionGoAway");

	KGlobal::config()->setGroup("WinPopup");
	QString handle = "WinPopup (" + KGlobal::config()->readEntry("HostName", "") + ")";

	actionStatusMenu = new KActionMenu("WinPopup", this);
	actionStatusMenu->popupMenu()->insertTitle(
		SmallIcon( statusIcon() ), handle );

	actionStatusMenu->insert(actionGoAvailable);
	actionStatusMenu->insert(actionGoAway);
	actionStatusMenu->insert(actionGoOffline);
}

void WPProtocol::installSamba()
{
	DEBUG(WPDMETHOD, "WPPreferences::installSamba()");

	QStringList args;
	args += KStandardDirs::findExe("winpopup-install.sh");
	args += KStandardDirs::findExe("winpopup-send.sh");
	if (KApplication::kdeinitExecWait("kdesu", args) == 0)
		KMessageBox::information(mPrefs, i18n("The Samba configuration file is modified."), i18n("Configuration succeeded"));
	else
		KMessageBox::error(mPrefs, i18n("Updating the Samba configuration file failed."), i18n("Configuration failed"));
}

#include "wpprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

