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
	mPluginsModule = new Plugins(this);

	KopeteWindow *mainWindow = new KopeteWindow( 0, "mainWindow" );
	setMainWidget( mainWindow );

	mAppearance = new AppearanceConfig( mainWindow );
	mUserPreferencesConfig = new KopeteUserPreferencesConfig( mainWindow );

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
}

Kopete::~Kopete()
{
	kdDebug() << "[Kopete] ~Kopete()" << endl;

	KopeteContactList::contactList()->save();

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

	LibraryLoader::pluginLoader()->loadAll();
}

/** Add a contact through Wizard */
void Kopete::slotAddContact()
{
	AddWizardImpl *tmpdialog = new AddWizardImpl( qApp->mainWidget() );
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

void Kopete::slotShowTransfers()
{
	KopeteTransferManager::transferManager()->show();
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:

