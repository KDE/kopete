/*
    kopete.cpp

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopete.h"

#include <qglobal.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qstylesheet.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addwizardimpl.h"
#include "appearanceconfig.h"
#include "kopeteaway.h"
#include "kopetecontactlist.h"
#include "kopeteemoticons.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetenotifier.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopetetransfermanager.h"
#include "kopeteuserpreferences.h"
#include "kopetewindow.h"
#include "pluginloader.h"
#include "pluginmodule.h"
#include "preferencesdialog.h"

Kopete::Kopete()
: KUniqueApplication( true, true, true )
{
	/*
	 * This is a workaround for a quite odd problem:
	 * When starting up kopete and the msn plugin gets loaded it can bring up
	 * a messagebox, in case the msg configuration is missing. This messagebox
	 * will result in a QApplication::enter_loop() call, an event loop is
	 * created. At this point however the loop_level is 0, because this is all
	 * still inside the Kopete constructor, before the exec() call from main.
	 * When the messagebox is finished the loop_level will drop down to zero and
	 * QApplication thinks the application shuts down (this is usually the case
	 * when the loop_level goes down to zero) . So it emits aboutToQuit(), to
	 * which KApplication is connected and re-emits shutdown() , to which again
	 * KMainWindow (a KopeteWindow instance exists already) is connected. KMainWindow's
	 * shuttingDown() slot calls queryExit() which results in KopeteWindow::queryExit()
	 * calling unloadPlugins() . This of course is wrong and just shouldn't happen.
	 * The workaround is to simply delay the initialization of all this to a point
	 * where the loop_level is already > 0 . That is why I moved all the code from
	 * the constructor to the initialize() method and added this single-shot-timer
	 * setup. (Simon)
	 */
	QTimer::singleShot( 0, this, SLOT( initialize() ) );
}

void Kopete::initialize()
{
	mLibraryLoader = new LibraryLoader();

	// TODO: move that to mainwindow!
	mPref = new PreferencesDialog();
	mPref->hide();

	mPluginsModule = new Plugins(this);

	m_mainWindow = new KopeteWindow( 0, "m_mainWindow" );
	setMainWidget(m_mainWindow);
	connect( m_mainWindow, SIGNAL( destroyed() ),
				this, SLOT( slotMainWindowDestroyed() ) );

	mAppearance = new AppearanceConfig(m_mainWindow);
	mUserPreferencesConfig = new KopeteUserPreferencesConfig(m_mainWindow);

	connect( KopetePrefs::prefs() , SIGNAL(saved()), this, SIGNAL(signalSettingsChanged()));
	mNotifier = new KopeteNotifier(this, "mNotifier");
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( messageReceived( KopeteMessage & ) ),
		SIGNAL( aboutToDisplay( KopeteMessage & ) ) );
	connect( KopeteMessageManagerFactory::factory(),
		SIGNAL( messageQueued( KopeteMessage & ) ),
		SIGNAL( aboutToSend( KopeteMessage & ) ) );

	QTimer::singleShot( 0, this, SLOT( slotLoadPlugins() ) );

	// Ok, load saved plugins

	KopeteContactList::contactList()->load();
	mTransferManager = KopeteTransferManager::transferManager();
}

Kopete::~Kopete()
{
	kdDebug() << "[Kopete] ~Kopete()" << endl;

	KopeteContactList::contactList()->save();

	delete mPref;
	delete mLibraryLoader;

	kdDebug() << "[Kopete] END ~Kopete()" << endl;
}

void Kopete::slotLoadPlugins()
{
	KConfig *config = KGlobal::config();
	config->setGroup("");

	// Parse command-line arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	QStringList modules;
	
	if (config->hasKey("Modules"))
		modules = config->readListEntry("Modules");
	else
	{
		// Ups! the user does not have plugins selected.
		// TODO: show "first time" wizard and let user decide which modules to load
		modules.append("autoaway.plugin");
		// Other modules to load for the first time?
	}

	// Listen to arguments
	if (args->count() > 0)
		modules.clear();

	for (int i = 0; i < args->count(); i++)
	{
		QString argument = args->arg(i);
		if (!argument.endsWith(".plugin"))
			argument.append(".plugin");

		modules.append(argument);
	}

	// Prevent plugins from loading?
	QCStringList disableArgs = args->getOptionList("disable");
	for (QCStringList::ConstIterator i = disableArgs.begin(); i != disableArgs.end(); ++i)
	{
		QString argument = QString::fromLatin1(*i);
		if (!argument.endsWith(".plugin"))
			argument.append(".plugin");

		modules.remove(argument);
	}

	// --noplugins specified?
	if (!args->isSet("plugins"))
	{
		modules.clear();
	}

	config->writeEntry("Modules", modules);

	mLibraryLoader->loadAll();
}

void Kopete::slotPreferences()
{
	kdDebug() << "[Kopete] slotPreferences()" << endl;
	mPref->show();
	mPref->raise();
}

/*
void Kopete::slotExit()
{
	kdDebug() << "[Kopete] slotExit()" << endl;
	quit();
}
*/

/** Connect all loaded protocol plugins */
void Kopete::slotConnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] Connect All: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( !(prot->isConnected()))
		{
			prot->Connect();
		}
	}
}

/** Disconnect all loaded protocol plugins */
void Kopete::slotDisconnectAll()
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] Disconnect All: "<<(*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if (prot->isConnected())
		{
			prot->Disconnect();
		}
	}
}


// Set a meta-away in all protocol plugins
// This is a fire and forget thing, we do not check if
// it worked or if the plugin exits away-mode
void Kopete::slotSetAwayAll(void)
{
	KopeteAway::setGlobalAway(true);
	KopeteAway::show();
}

// Set a meta-away in all protocol plugins without showing the dialog
void Kopete::setAwayAll(void)
{
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] slotSetAwayAll() for plugin: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( prot->isConnected() && !prot->isAway() )
		{
			kdDebug() << "[Kopete] setting away-mode for: " << (*i).name << endl;
			prot->setAway(); // sets protocol-plugin into away-mode
		}
	}
}

// Set a meta-available in all protocol plugins
// This is a fire and forget thing, we do not check if
// it worked or if the plugin exits away-mode
void Kopete::slotSetAvailableAll(void)
{
	KopeteAway::setGlobalAway(false);
	QValueList<KopeteLibraryInfo> l = kopeteapp->libraryLoader()->loaded();
	for (QValueList<KopeteLibraryInfo>::Iterator i = l.begin(); i != l.end(); ++i)
	{
		kdDebug() << "[Kopete] slotSetAvailableAll() for plugin: " << (*i).name << endl;
		KopetePlugin *tmpprot = (kopeteapp->libraryLoader())->mLibHash[(*i).specfile]->plugin;
		KopeteProtocol *prot =  dynamic_cast<KopeteProtocol*>(tmpprot);

		if (!prot)
			continue;

		if ( prot->isConnected() && prot->isAway() )
		{
			kdDebug() << "[Kopete] setting available-mode for: " << (*i).name << endl;
			prot->setAvailable(); // sets protocol-plugin into away-mode
		}
	}
}

/** Add a contact through Wizard */
void Kopete::slotAddContact()
{
	AddWizardImpl *tmpdialog = new AddWizardImpl( m_mainWindow );
	tmpdialog->show();
}

/** Add a Event for notify */
void Kopete::notifyEvent( KopeteEvent *event)
{
	/* See KopeteNotifier and KopeteEvent class */
	mNotifier->notifyEvent( event );
}

/** Cancel an event */
void Kopete::cancelEvent( KopeteEvent *event)
{
	/* deleted events are removed automaticly */
	delete event;
}

KStatusBar* Kopete::statusBar() const
{
	return m_mainWindow ? m_mainWindow->statusBar() : 0L;
}

KopeteSystemTray* Kopete::systemTray() const
{
	return m_mainWindow ? m_mainWindow->tray : 0L;
}

void Kopete::slotMainWindowDestroyed()
{
	m_mainWindow = 0L;
}

void Kopete::slotShowTransfers()
{
	transferManager()->show();
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:
