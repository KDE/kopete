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
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kpushbutton.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdguiitem.h>

// Local Includes
#include "wpprotocol.h"
#include "wpdebug.h"
#include "wpcontact.h"
#include "wpaddcontact.h"
#include "wppreferences.h"

// Kopete Includes
#include "kopete.h"
#include "kopetecontactlistview.h"
#include "kopetemetacontact.h"
#include "statusbaricon.h"
#include "systemtray.h"

class KPopupMenu;

WPProtocol *WPProtocol::sProtocol = 0;

K_EXPORT_COMPONENT_FACTORY(kopete_wp, KGenericFactory<WPProtocol>);

// WP Protocol
WPProtocol::WPProtocol(QObject *parent, QString name, QStringList) : KopeteProtocol(parent, name)
{
	DEBUG(WPDMETHOD, "WPProtocol::WPProtocol()");
	
	if(!sProtocol) sProtocol = this;

	DEBUG(WPDINFO, "Loading WinPopup Plugin...");

	theInterface = 0;

	// Load icons
	initIcons();

	// Load Status Actions
	initActions();

	// Create statusbar Icon
	statusBarIcon = new StatusBarIcon();
	QObject::connect(statusBarIcon, SIGNAL(rightClicked(const QPoint &)), this, SLOT(slotIconRightClicked(const QPoint &)));
	setAvailable();
	Connect();

	// Create preferences menu
	mPrefs = new WPPreferences("wp_icon", this);
	connect(mPrefs, SIGNAL(saved(void)), this, SLOT(slotSettingsChanged(void)));

	// Set up initial settings
	KGlobal::config()->setGroup("WinPopup");
	QString theSMBClientPath = KGlobal::config()->readEntry("SMBClientPath", "/usr/local/bin/smbclient");
	QString theInitialSearchHost = KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1");
	QString theHostName = "";
	QString theAwayMessage = KGlobal::config()->readEntry("AwayMessage", i18n("Sorry, I'm not here right now."));
	bool theSendAwayMessage = KGlobal::config()->readBoolEntry("SendAwayMessage", true);
	bool theEmailDefault = KGlobal::config()->readBoolEntry("EmailDefault", true);
	if(theHostName == "")
	{	QFile infile("/etc/HOSTNAME");
		if(infile.open(IO_ReadOnly))
		{	QTextStream in(&infile);
			char c;
			for(in >> c; c != '.' && (!infile.atEnd()); in >> c)
				theHostName = theHostName + char((c >= 65 && c < 91) ? c : (c - 32));
			infile.close();
		}
		else
			theHostName = KGlobal::config()->readEntry("HostName", "LOCALHOST");
	}
	KGlobal::config()->writeEntry("HostName", theHostName);
	KGlobal::config()->writeEntry("SMBClientPath", theSMBClientPath);
	KGlobal::config()->writeEntry("InitialSearchHost", theInitialSearchHost);
	KGlobal::config()->writeEntry("AwayMessage", theAwayMessage);
	KGlobal::config()->writeEntry("SendAwayMessage", theSendAwayMessage);
	KGlobal::config()->writeEntry("EmailDefault", theEmailDefault);

	// ask for installation.
	if(KMessageBox::questionYesNo(mPrefs, i18n("The Samba configuration file needs to be modified in order for Kopete to receive WinPopup messages. Would you like to do this now?"), i18n("Modify Samba Configuration Now?"), KGuiItem(), KGuiItem(), "WPFirstTime") == KMessageBox::Yes)
		installSamba();

	// Create the interface...
	theInterface = new KopeteWinPopup(theSMBClientPath, theInitialSearchHost, theHostName);

	// Call slotSettingsChanged() to get it all registered.
	slotSettingsChanged();

	// FIXME: I guess 'myself' should be a metacontact as well...
	theMyself = new WPContact(theHostName, this, 0L);		// XXX: Should be from config file!!!
	connect(theInterface, SIGNAL(newMessage(const QString &, const QDateTime &, const QString &)), this, SLOT(slotGotNewMessage(const QString &, const QDateTime &, const QString &)));
}

// Destructor
WPProtocol::~WPProtocol()
{
	DEBUG(WPDMETHOD, "WPProtocol::~WPProtocol()");

	delete theInterface;
	delete theMyself;
}

/*KopeteContact *WPProtocol::createContact(KopeteMetaContact *parent, const QString &serializedData)
{
	DEBUG(WPDMETHOD, "WPPreferences::createContact(parent, " << serializedData << ")");
	// FIXME: serializedData contains much more than just the screen name,
	// but for now it hopefully suffices.

	// FIXME: more error-proof deserialize would be useful :)
	QStringList data = QStringList::split( ' ', serializedData );
	QString userId   = data[ 0 ].replace( QRegExp( "%20" ), " " );
	QString name     = data[ 1 ].replace( QRegExp( "%20" ), " " );
	QString group    = data[ 2 ].replace( QRegExp( "%20" ), " " );

	return new WPContact(userId, this, parent);
}*/

bool WPProtocol::serialize(KopeteMetaContact *metaContact, QStringList &strList) const
{
	DEBUG(WPDMETHOD, "WPProtocol::serialize(metaContact => " << metaContact->displayName() << ", <strList>)");

	QStringList addressList;
	QPtrList<KopeteContact> contacts = metaContact->contacts();

	bool done = false;
	for(KopeteContact *c = contacts.first(); c; c = contacts.next())
		if(c->protocol() == this->id())
		{
			WPContact *curContact = static_cast<WPContact*>(c);
			DEBUG(WPDINFO, "Sub-Contact " << curContact->host() << " is ours - serialising.");
			strList << curContact->host();
//			addressList << curContact->host();
			done = true;
		}

//	QString addresses = addressList.join(",");
//	if(!addresses.isEmpty())
//		metaContact->setAddressBookField(WPProtocol::protocol(), "messaging/winpopup", addresses);

	DEBUG(WPDINFO, "Finished with strList = " << strList.join(","));
	return done;
}

QStringList WPProtocol::addressBookFields() const
{
	DEBUG(WPDMETHOD, "WPProtocol::addressBookFields()");

	return QStringList("messaging/winpopup");
}

void WPProtocol::deserialize(KopeteMetaContact *metaContact, const QStringList &strList)
{
	DEBUG(WPDMETHOD, "WPProtocol::deserialize(metaContact => " << metaContact->displayName() << ", " << strList.join(",") << ")");

	// not using the kabc thingy for now it would seem...
//	QStringList hosts = QStringList::split("\n", metaContact->addressBookField(this, "messaging/winpopup"));

	for(unsigned i = 0; i < strList.count(); i++)
	{
		QString host = strList[i];

		DEBUG(WPDINFO, "Sub-Contact " << host << " is deserialised.");

		WPContact *newContact = new WPContact(host, this, metaContact);
		metaContact->addContact(newContact);
	}
}

WPContact *WPProtocol::getContact(const QString &Name, KopeteMetaContact* theMetaContact)
{
	DEBUG(WPDMETHOD, "WPProtocol::getContact(" << Name << ", " << theMetaContact << ")");

	KopeteContactList *l = KopeteContactList::contactList();

	if(!theMetaContact)
	{
		// Should really ask to see if they want the contact adding to their list first...
		theMetaContact = l->findContact(this->id(), Name, "smb://" + Name);
		if(!theMetaContact)
		{	DEBUG(WPDINFO, "Adding " << Name << " to the contact list...");
			theMetaContact = new KopeteMetaContact();
			l->addMetaContact(theMetaContact);
		}
	}

	KopeteContact *theContact = theMetaContact->findContact(this->id(), Name, "smb://" + Name);
	if(!theContact)
	{	theContact = new WPContact(Name, this, theMetaContact);
		theMetaContact->addContact(theContact);
	}

	return dynamic_cast<WPContact *>(theContact);
}

//		QString names = theMetaContact->addressBookField(this, "messaging/winpopup") + "\n" + Name;
//		theMetaContact->setAddressBookField(this, "messaging/winpopup", names);


/*WPContact *WPProtocol::addContact(const QString &Name, KopeteMetaContact* theMetaContact)
{
	DEBUG(WPDMETHOD, "WPProtocol::addContact(" << Name << ", <parent>)");

	// If no meta-contact is supplied, this method isn't the right one to call!
	if(!theMetaContact) return 0;

	// Create a new contact if one for Name doesn't yet exist...
	KopeteContact *theContact = theMetaContact->findContact(this->id(), Name, QString::null);
	if(!theContact)
	{
		// does this stuff actually do anything?
//		QString names = theMetaContact->addressBookField(this, "messaging/winpopup") + "\n" + Name;
//		theMetaContact->setAddressBookField(this, "messaging/winpopup", names);

		theContact = new WPContact(Name, this, theMetaContact);
		theMetaContact->addContact(theContact);
	}

	return dynamic_cast<WPContact *>(theContact);
}*/

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

// Unload statusbar icon
bool WPProtocol::unload()
{
	DEBUG(WPDMETHOD, "WPProtocol::unload()");

	DEBUG(WPDINFO, "Unloading WPProtocol...");
	if(kopeteapp->statusBar())
	{
		kopeteapp->statusBar()->removeWidget(statusBarIcon);
		delete statusBarIcon;
	}

	emit protocolUnloading();
	return true;
}

KopeteContact *WPProtocol::myself() const
{
	return theMyself;
}

// Connect to server
void WPProtocol::Connect()
{
	DEBUG(WPDMETHOD, "WPProtocol::Connect()");

	online = true;
	available = true;
	statusBarIcon->setPixmap(iconAvailable);
}

// Disconnect from server
void WPProtocol::Disconnect()
{
	DEBUG(WPDMETHOD, "WPProtocol::Disconnect()");

	online = false;
	statusBarIcon->setPixmap(iconOffline);
}

// Set user available
void WPProtocol::setAvailable()
{
	DEBUG(WPDMETHOD, "WPProtocol::setAvailable()");

	online = true;
	available = true;
	statusBarIcon->setPixmap(iconAvailable);
	// do any other stuff?
}

// Set user away
void WPProtocol::setAway()
{
	DEBUG(WPDMETHOD, "WPProtocol::setAway()");

	available = false;
	online = true;
	statusBarIcon->setPixmap(iconAway);
	// do any other stuff?
}


// Return true if connected
bool WPProtocol::isConnected() const
{
	DEBUG(WPDMETHOD, "WPProtocol::isConnected()");

	return online;
}


// Return true if away
bool WPProtocol::isAway() const
{
	DEBUG(WPDMETHOD, "WPProtocol::isAway()");

	return !available;
}


// Return protocol icon name
QString WPProtocol::protocolIcon() const
{
	DEBUG(WPDMETHOD, "WPProtocol::protocolIcon()");

	return "wp_icon";
}

// Return "add contact" dialog
AddContactPage *WPProtocol::createAddContactWidget(QWidget *parent)
{
	DEBUG(WPDMETHOD, "WPProtocol::createAddContactWidget(" << (int)parent << ")");

	return new WPAddContact(this, parent);
}


// CallBack when clicking on statusbar icon
void WPProtocol::slotIconRightClicked(const QPoint &)
{
	DEBUG(WPDMETHOD, "WPProtocol::slotIconRightClicked(<qpoint>)");

	KGlobal::config()->setGroup("WinPopup");
	QString handle = "WinPopup (" + KGlobal::config()->readEntry("HostName", "") + ")";

	popup = new KPopupMenu(statusBarIcon);
	popup->insertTitle(handle);
	actionGoAvailable->plug(popup);
	actionGoAway->plug(popup);
	actionGoOffline->plug(popup);
	popup->popup(QCursor::pos());
}

// Send a message
void WPProtocol::slotSendMessage(const QString &Body, const QString &Destination)
{
	DEBUG(WPDMETHOD, "WPProtocol::slotSendMessage(" << Body << ", " << Destination << ")");

	theInterface->sendMessage(Body, Destination);
}

// Callback when settings changed
void WPProtocol::slotSettingsChanged()
{
	DEBUG(WPDMETHOD, "WPProtocol::slotSettingsChanged()");

	KGlobal::config()->setGroup("WinPopup");
	theInterface->setSMBClientPath(KGlobal::config()->readEntry("SMBClientPath", "/usr/local/bin/smbclient"));
	theInterface->setInitialSearchHost(KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1"));
	theInterface->setHostName(KGlobal::config()->readEntry("HostName", "LOCAL"));
}

// Private initIcons
void WPProtocol::initIcons()
{
	DEBUG(WPDMETHOD, "WPProtocol::initIcons()");

	KIconLoader *loader = KGlobal::iconLoader();
	KStandardDirs dir;

	iconAvailable  = QPixmap(loader->loadIcon("wp_available",  KIcon::User));
	iconAway = QPixmap(loader->loadIcon("wp_away", KIcon::User));
	iconOffline = QPixmap(loader->loadIcon("wp_offline", KIcon::User));
}

// Private initActions
void WPProtocol::initActions()
{
	DEBUG(WPDMETHOD, "WPProtocol::initActions()");

	actionGoAvailable = new KAction("Online", "wp_available", 0, this, SLOT(Connect()), this, "actionGoAvailable");
	actionGoOffline = new KAction("Offline", "wp_offline", 0, this, SLOT(Disconnect()), this, "actionGoOffline");
	actionGoAway = new KAction("Away", "wp_away", 0, this, SLOT(setAway()), this, "actionGoAway");

	actionStatusMenu = new KActionMenu("WinPopup", this);
	actionStatusMenu->insert(actionGoAvailable);
	actionStatusMenu->insert(actionGoAway);
	actionStatusMenu->insert(actionGoOffline);
	actionStatusMenu->plug(kopeteapp->systemTray()->contextMenu(), 1);
}

void WPProtocol::installSamba()
{
	DEBUG(WPDMETHOD, "WPPreferences::installSamba()");

	QProcess sender;
	sender.addArgument(KStandardDirs::findExe("find"));
	sender.addArgument("/etc");
	sender.addArgument("-name");
	sender.addArgument("smb.conf");
	if(!sender.launch("")) { qDebug("Couldn't launch find!"); return; }

	QString SmbConfPath = "";
	while((sender.isRunning() || sender.canReadLineStdout()) && SmbConfPath == "")		// add timeout?
		if(sender.canReadLineStdout())
			SmbConfPath = sender.readLineStdout();
	if(SmbConfPath == "") { qDebug("Couldn't find smb.conf!"); return; }

	// read config in...
	QString SmbDotConf = "";
	QFile f(SmbConfPath);
	if(f.open(IO_ReadOnly))
	{	QTextStream fin(&f);
		while(!fin.eof())
			SmbDotConf += fin.readLine() + "\n";
		f.close();
	}
	else { qDebug("Couldn't read smb.conf!"); return; }

	// modify the config...
	QRegExp msgLine("\n   message command = [^\n]*\n");
	msgLine.search(SmbDotConf);
	SmbDotConf.replace(msgLine, "\n   message command = " + KStandardDirs::findExe("winpopup-send.sh") + " %s %f &\n");

	// write (modified) config out...
	QString TempFile = "/tmp/smb.conf.new";
	QFile f2(TempFile);
	if(f2.open(IO_WriteOnly))
	{	QTextStream fout(&f2);
		fout << SmbDotConf;
		f2.close();
	}
	else { qDebug("Couldn't write new smb.conf!"); return; }

	QStringList args;
	args += SmbConfPath;
	args += "/tmp/smb.conf.old";
	KApplication::kdeinitExecWait("cp", args);

	args.clear();
	args += "cp";
	args += TempFile;
	args += SmbConfPath;
	KApplication::kdeinitExecWait("kdesu", args);
}

#include "wpprotocol.moc"

// vim: set ts=4 sts=4 sw=4 noet:

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

