/*
    kopete.cpp

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002  by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2002-2003  by Martijn Klingens         <klingens@kde.org>

    Kopete    (c) 2001-2003  by the Kopete developers    <kopete-devel@kde.org>

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
#include "kopeteaccountmanager.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopetewindow.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

Kopete::Kopete()
: KUniqueApplication( true, true, true )
{
	m_isShuttingDown = false;
	m_mainWindow = new KopeteWindow( 0, "mainWindow" );

	// KApplication sets the reference count to 1 on startup. KopetePluginManager has a
	// reference to us once created, so create it and drop our own reference.
	KopetePluginManager::self();
	deref();

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
	kdDebug( 14000 ) << k_funcinfo << endl;

	delete m_mainWindow;
	//kdDebug( 14000 ) << k_funcinfo << "Done" << endl;
}

void Kopete::slotLoadPlugins()
{
	//Create the command handler (looks silly)
	KopeteCommandHandler::commandHandler();

	KopeteAccountManager::manager()->load();
	KopeteContactList::contactList()->load();

	KConfig *config = KGlobal::config();

	// Parse command-line arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	bool showConfigDialog = false;

	config->setGroup( "Plugins" );

	if ( !config->hasGroup( "Plugins" ) )
		showConfigDialog = true;

	// Listen to arguments
	if ( args->count() > 0 )
	{
		showConfigDialog = false;
		for ( int i = 0; i < args->count(); i++ )
			KopetePluginManager::self()->setPluginEnabled( args->arg( i ), true );
	}

	// Prevent plugins from loading? (--disable=foo,bar)
	QStringList disableArgs = QStringList::split( ',', args->getOption( "disable" ) );
	for ( QStringList::ConstIterator it = disableArgs.begin(); it != disableArgs.end(); ++it )
	{
		showConfigDialog = false;
		KopetePluginManager::self()->setPluginEnabled( *it, false );
	}

	// Load some plugins exclusively? (--load-plugins=foo,bar)
	if ( args->isSet( "load-plugins" ) )
	{
		config->deleteGroup( "Plugins", true );
		showConfigDialog = false;
		QStringList plugins = QStringList::split( ',', args->getOption( "load-plugins" ) );
		for ( QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it )
			KopetePluginManager::self()->setPluginEnabled( *it, true );
	}

	// Disable plugins altogether? (--noplugins)
	if ( !args->isSet( "plugins" ) )
	{
		config->deleteGroup( "Plugins", true );
		showConfigDialog = false;
	}

	config->sync();

	KopetePluginManager::self()->loadAllPlugins();

	connect( KopetePluginManager::self(), SIGNAL( allPluginsLoaded() ),
		this, SLOT( slotAllPluginsLoaded() ));

	//load the default chatwindow
	KopetePluginManager::self()->loadPlugin( "kopete_chatwindow" );


	if( showConfigDialog )
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
		KAction *action = KopeteStdAction::preferences( 0L );
		action->activate();
		delete action;
	}

}


void  Kopete::slotAllPluginsLoaded()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	// --noconnect not specified?
	if ( args->isSet( "connect" ) )
		KopeteAccountManager::manager()->autoConnect();

	QCStringList connectArgs = args->getOptionList( "autoconnect" );
	for ( QCStringList::ConstIterator i = connectArgs.begin(); i != connectArgs.end(); ++i )
	{
		QString id = QString::fromLatin1( *i );

		QRegExp rx( QString::fromLatin1( "([^\\|]*)\\|\\|(.*)" ) );
		rx.search( id );
		QString protocolId = rx.cap( 1 );
		QString accountId = rx.cap( 2 );

		if ( accountId.isEmpty() )
		{
			if ( protocolId.isEmpty() )
				accountId = id;
			else
				continue;
		}

		QPtrListIterator<KopeteAccount> it( KopeteAccountManager::manager()->accounts() );
		KopeteAccount *account;
		while ( ( account = it.current() ) != 0 )
		{
			++it;

			if ( ( account->accountId() == accountId ) )
			{
				if ( protocolId.isEmpty() || account->protocol()->pluginId() == protocolId )
				{
					account->connect();
					break;
				}
			}
		}
	}
}

void Kopete::quitKopete()
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	if ( !m_isShuttingDown )
	{
		m_isShuttingDown = true;

#if KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
		// When we close Kopete through KSystemTray, kdelibs will close all open
		// windows first. However, despite the destructive close the main window
		// is _NOT_ yet deleted at this point (it's a scheduled deleteLater()
		// call).
		// Due to a bug in KMainWindow prior to KDE 3.2 calling close() a second
		// time also derefs KApplication a second time, which causes a premature
		// call to KApplication::quit(), so we never go through the plugin
		// manager's shutdown process.
		// Unfortunately we can't assume close() ever being called though,
		// because the code paths not using the system tray still need this.
		// As a workaround we schedule a call to quitKopete() through a timer,
		// so the event loop is processed and the window is already deleted.
		// - Martijn
		QTimer::singleShot( 0, this, SLOT( quitKopete() ) );
		return;
#endif
	}

	if ( !m_mainWindow.isNull() )
		m_mainWindow->close();

	// First save the contacts and accounts before unloading them
	// FIXME: After KDE 3.2 we _really_ need to find a way to save these lists
	//        whenever they change instead of on exit and explicit save only.
	//        This requires 'dirty' notifications for all involved objects
	//        though, after which we can fire a 1-sec timer to do the saving,
	//        much like the contact list sorting. Too intrusive for now,
	//        that's for sure. - Martijn
	KopeteContactList::contactList()->save();
	KopeteAccountManager::manager()->save();

	//unload plugins and shutdown
	KopetePluginManager::self()->shutdown();
}

void Kopete::commitData( QSessionManager &sm )
{
	m_isShuttingDown = true;
	KUniqueApplication::commitData( sm );
}

#include "kopete.moc"

// vim: set noet ts=4 sts=4 sw=4:

