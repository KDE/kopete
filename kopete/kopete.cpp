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

#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "appearanceconfig.h"
#include "kopetecontactlist.h"
#include "kopeteuserpreferences.h"
#include "kopetewindow.h"
#include "pluginloader.h"
#include "pluginconfig.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

Kopete::Kopete()
: KUniqueApplication( true, true, true )
{
	m_mainWindow = new KopeteWindow( 0, "mainWindow" );
	setMainWidget( m_mainWindow );

	// Since the main window has no parent we must delete it in the Kopete
	// destructor (we can't leak it, some code depends on the destructor
	// being called). But since a KMainWindow usually has W_DestructiveClose
	// set we can't be sure it exists by then either. Therefore track its
	// deletion to make sure:
	connect( m_mainWindow, SIGNAL( destroyed() ),
		SLOT( slotMainWindowDestroyed() ) );

	// Create the plugin preferences module
	new PluginConfig( this );

	new AppearanceConfig( m_mainWindow );
	new KopeteUserPreferencesConfig( m_mainWindow );

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
	 *
	 * Additionally, it makes the GUI appear less 'blocking' during startup, so
	 * there is a secondary benefit as well here. (Martijn)
	 */
	QTimer::singleShot( 0, this, SLOT( slotLoadPlugins() ) );
}

Kopete::~Kopete()
{
	KopeteContactList::contactList()->save();
	delete m_mainWindow;
//	kdDebug(14000) << "Kopete::~Kopete: done" <<endl;
}

void Kopete::slotLoadPlugins()
{
	KopeteContactList::contactList()->load();

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
		modules.append("contactnotes.plugin");
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

void Kopete::slotMainWindowDestroyed()
{
	m_mainWindow = 0L;
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:

