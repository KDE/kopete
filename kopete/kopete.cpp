/*
    kopete.cpp

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002  by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002-2003  by the Kopete developers    <kopete-devel@kde.org>

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
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "kopeteaccount.h"
#include "kopeteprotocol.h"
#include "appearanceconfig.h"
#include "behaviorconfig.h"
#include "kopetecontactlist.h"
#include "kopeteaccountmanager.h"
#include "kopetecommandhandler.h"
#include "kopetewindow.h"
#include "pluginloader.h"
#include "preferencesdialog.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

Kopete::Kopete()
: KUniqueApplication( true, true, true )
{
	m_isShuttingDown = false;
	m_mainWindow = new KopeteWindow( 0, "mainWindow" );
	setMainWidget( m_mainWindow );

	// Since the main window has no parent we must delete it in the Kopete
	// destructor (we can't leak it, some code depends on the destructor
	// being called). But since a KMainWindow usually has W_DestructiveClose
	// set we can't be sure it exists by then either. Therefore track its
	// deletion to make sure:
	connect( m_mainWindow, SIGNAL( destroyed() ),
		SLOT( slotMainWindowDestroyed() ) );

	// Create the preferences module
	new AppearanceConfig( m_mainWindow );
	new BehaviorConfig( m_mainWindow );

	/*
	 * FIXME: This is a workaround for a quite odd problem:
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
	KopeteAccountManager::manager()->save();
	delete m_mainWindow;
//	kdDebug(14000) << "Kopete::~Kopete: done" <<endl;
}

void Kopete::slotLoadPlugins()
{
	kapp->processEvents();
	//Create the command handler (looks silly)
	KopeteCommandHandler::commandHandler();

	KopeteAccountManager::manager()->load();
	KopeteContactList::contactList()->load();

	KConfig *config = KGlobal::config();
	config->setGroup("");

	// Parse command-line arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	QStringList modules;
	bool showConfigDialog = false;

	if( config->hasKey( "Plugins" ) )
	{
		modules = config->readListEntry( "Plugins" );
	}
	else
	{
		// No plugins specified. Show the config dialog.
		// FIXME: Although it's a bit stupid it is theoretically possible that a user
		//        explicitly configured Kopete to not load plugins on startup. In this
		//        case we don't want this dialog. We need some other config setting
		//        like a bool hasRunKopeteBefore or so to trigger the loading of the
		//        wizard. Maybe using the last run version number is more useful even
		//        as it also allows for other features. - Martijn
		// FIXME: Of course this is not a too-good GUI because a first-timer would need
		//        some kind of "welcome" dialog or wizard. But for now it's better than
		//        nothing at all. - Martijn
		// FIXME: Possibly we need to influence the showConfigDialog bool based on the
		//        command line arguments processed below. But how exactly? - Martijn
		showConfigDialog = true;
	}

	// Listen to arguments
	if (args->count() > 0)
		modules.clear();

	for (int i = 0; i < args->count(); i++)
	{
		QString argument = args->arg(i);
		if (!argument.endsWith(".desktop"))
			argument.append(".desktop");

		modules.append(argument);
	}

	// Prevent plugins from loading?
	QCStringList disableArgs = args->getOptionList("disable");
	for (QCStringList::ConstIterator i = disableArgs.begin(); i != disableArgs.end(); ++i)
	{
		QString argument = QString::fromLatin1(*i);
		if (!argument.endsWith(".desktop"))
			argument.append(".desktop");

		modules.remove(argument);
	}

	// --noplugins specified?
	if (!args->isSet("plugins"))
	{
		modules.clear();
	}

	config->writeEntry( "Plugins", modules );

	LibraryLoader::pluginLoader()->loadAll();

	//load the default chatwindow
	LibraryLoader::pluginLoader()->loadPlugin( "chatwindow.desktop" );


	// --noconnect not specified?
	if (args->isSet("connect"))
	{
		KopeteAccountManager::manager()->autoConnect();
	}

	QCStringList connectArgs = args->getOptionList("autoconnect");
	for (QCStringList::ConstIterator i = connectArgs.begin(); i != connectArgs.end(); ++i)
	{
		QString id = QString::fromLatin1(*i);

		QRegExp rx( QString::fromLatin1("([^\\|]*)\\|\\|(.*)"));
		rx.search(id);
		QString protocolId=rx.cap(1);
		QString accountId=rx.cap(2);

		if(accountId.isEmpty())
		{
			if(protocolId.isEmpty())
				accountId=id;
			else
				continue;
		}

		QPtrListIterator<KopeteAccount> it( KopeteAccountManager::manager()->accounts() );
		KopeteAccount *account;
		while ( ( account = it.current() ) != 0 )
		{
			++it;

			if( ( account->accountId() == accountId) )
			{
				if( protocolId.isEmpty() || account->protocol()->pluginId() == protocolId )
				{
					account->connect();
					break;
				}
			}
		}
	}


	if( showConfigDialog )
		PreferencesDialog::preferencesDialog()->show();
}

void Kopete::slotMainWindowDestroyed()
{
	m_mainWindow = 0L;
}

void Kopete::quitKopete()
{
	m_isShuttingDown = true;
	quit();
}

void Kopete::commitData( QSessionManager &sm )
{
	m_isShuttingDown = true;
	KUniqueApplication::commitData(sm);
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:

