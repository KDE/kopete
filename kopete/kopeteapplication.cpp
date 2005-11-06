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

#include "kopeteapplication.h"

#include <qtimer.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>

#include "addaccountwizard.h"
#include "kabcpersistence.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopeteglobal.h"
#include "kopetemimesourcefactory.h"
#include "kopetemimetypehandler.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"
#include "kopetewindow.h"
#include "kopeteprefs.h"
#include "kopeteviewmanager.h"
#include "videodevice.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KopeteApplication::KopeteApplication()
: KUniqueApplication( true, true, true )
{
	m_isShuttingDown = false;
	m_mainWindow = new KopeteWindow( 0, "mainWindow" );

	Kopete::PluginManager::self();

	Kopete::UI::Global::setMainWidget( m_mainWindow );

	/*
	 * FIXME: This is a workaround for a quite odd problem:
	 * When starting up kopete and the msn plugin gets loaded it can bring up
	 * a messagebox, in case the msg configuration is missing. This messagebox
	 * will result in a QApplication::enter_loop() call, an event loop is
	 * created. At this point however the loop_level is 0, because this is all
	 * still inside the KopeteApplication constructor, before the exec() call from main.
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

	m_mimeFactory = new Kopete::MimeSourceFactory;
	QMimeSourceFactory::addFactory( m_mimeFactory );

	//Create the emoticon installer
	m_emoticonHandler = new Kopete::EmoticonMimeTypeHandler;
}

KopeteApplication::~KopeteApplication()
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	delete m_mainWindow;
	delete m_emoticonHandler;
	delete m_mimeFactory;
	//kdDebug( 14000 ) << k_funcinfo << "Done" << endl;
}

void KopeteApplication::slotLoadPlugins()
{
	// we have to load the address book early, because calling this enters the Qt event loop when there are remote resources.
	// The plugin manager is written with the assumption that Kopete will not reenter the event loop during plugin load,
	// otherwise lots of things break as plugins are loaded, then contacts are added to incompletely initialised MCLVIs
	Kopete::KABCPersistence::self()->addressBook();

	//Create the command handler (looks silly)
	Kopete::CommandHandler::commandHandler();

	//Create the view manager
	KopeteViewManager::viewManager();

	Kopete::AccountManager::self()->load();
	Kopete::ContactList::self()->load();

	KConfig *config = KGlobal::config();

	// Parse command-line arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	bool showConfigDialog = false;

	config->setGroup( "Plugins" );

	/* FIXME: This is crap, if something purged that groups but your accounts
	 * are still working kopete will load the necessary plugins but still show the
	 * stupid accounts dialog (of course empty at that time because account data
	 * gets loaded later on). [mETz - 29.05.2004]
	 */
	if ( !config->hasGroup( "Plugins" ) )
		showConfigDialog = true;

	// Listen to arguments
	/*
	// TODO: conflicts with emoticon installer and the general meaning
	// of %U in kopete.desktop
	if ( args->count() > 0 )
	{
		showConfigDialog = false;
		for ( int i = 0; i < args->count(); i++ )
			Kopete::PluginManager::self()->setPluginEnabled( args->arg( i ), true );
	}
	*/

	// Prevent plugins from loading? (--disable=foo,bar)
	QStringList disableArgs = QStringList::split( ',', args->getOption( "disable" ) );
	for ( QStringList::ConstIterator it = disableArgs.begin(); it != disableArgs.end(); ++it )
	{
		showConfigDialog = false;
		Kopete::PluginManager::self()->setPluginEnabled( *it, false );
	}

	// Load some plugins exclusively? (--load-plugins=foo,bar)
	if ( args->isSet( "load-plugins" ) )
	{
		config->deleteGroup( "Plugins", true );
		showConfigDialog = false;
		QStringList plugins = QStringList::split( ',', args->getOption( "load-plugins" ) );
		for ( QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it )
			Kopete::PluginManager::self()->setPluginEnabled( *it, true );
	}

	config->sync();

	// Disable plugins altogether? (--noplugins)
	if ( !args->isSet( "plugins" ) )
	{
		// If anybody reenables this I'll get a sword and make a nice chop-suy out
		// of your body :P [mETz - 29.05.2004]
		// This screws up kopeterc because there is no way to get the Plugins group back!
		//config->deleteGroup( "Plugins", true );

		showConfigDialog = false;
		// pretend all plugins were loaded :)
		QTimer::singleShot(0, this, SLOT( slotAllPluginsLoaded() ));
	}
	else
	{
		Kopete::PluginManager::self()->loadAllPlugins();
	}

	connect( Kopete::PluginManager::self(), SIGNAL( allPluginsLoaded() ),
		this, SLOT( slotAllPluginsLoaded() ));

	if( showConfigDialog )
	{
		// No plugins specified. Show the config dialog.
		// FIXME: Although it's a bit stupid it is theoretically possible that a user
		//        explicitly configured Kopete to not load plugins on startup. In this
		//        case we don't want this dialog. We need some other config setting
		//        like a bool hasRunKopeteBefore or so to trigger the loading of the
		//        wizard. Maybe using the last run version number is more useful even
		//        as it also allows for other features. - Martijn
		// FIXME: Possibly we need to influence the showConfigDialog bool based on the
		//        command line arguments processed below. But how exactly? - Martijn
		// NB: the command line args are completely broken atm.  
		// I don't want to fix them for 3.5 as plugin loading will change for KDE4.	- Will
		AddAccountWizard *m_addwizard = new AddAccountWizard( Kopete::UI::Global::mainWidget(), "addAccountWizard", true, true );
		m_addwizard->exec();
		Kopete::AccountManager::self()->save();
	}
}

void KopeteApplication::slotAllPluginsLoaded()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	// --noconnect not specified?
	if ( args->isSet( "connect" )  && KopetePrefs::prefs()->autoConnect() )
		Kopete::AccountManager::self()->connectAll();


	// Handle things like '--autoconnect foo,bar --autoconnect foobar'
	QCStringList connectArgsC = args->getOptionList( "autoconnect" );
	QStringList connectArgs;

	for ( QCStringList::ConstIterator it = connectArgsC.begin(); it != connectArgsC.end(); ++it )
	{
		QStringList split = QStringList::split( ',', QString::fromLatin1( *it ) );

		for ( QStringList::ConstIterator it2 = split.begin(); it2 != split.end(); ++it2 )
		{
			connectArgs.append( *it2 );
		}
	}
	
	for ( QStringList::ConstIterator i = connectArgs.begin(); i != connectArgs.end(); ++i )
	{
		QRegExp rx( QString::fromLatin1( "([^\\|]*)\\|\\|(.*)" ) );
		rx.search( *i );
		QString protocolId = rx.cap( 1 );
		QString accountId = rx.cap( 2 );

		if ( accountId.isEmpty() )
		{
			if ( protocolId.isEmpty() )
				accountId = *i;
			else
				continue;
		}

		QPtrListIterator<Kopete::Account> it( Kopete::AccountManager::self()->accounts() );
		Kopete::Account *account;
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

	// Parse any passed URLs/files
	handleURLArgs();
}

int KopeteApplication::newInstance()
{
//	kdDebug(14000) << k_funcinfo << endl;
	handleURLArgs();

	/**
	 * The following three lines work around a problem that
	 * Kopete has since it has multiple main windows so
	 * qapp->mainWidget() returns 0, which breaks the normal
	 * KUniqueApplication::newInstance() behavior that raises
	 * the window when we run a new instance.
	 *
	 * 1. Set the main widget to the contact list window
	 * 2. Call KUniqueApplication::newInstance()
	 * 3. Set the main widget back to 0
	 *
	 * This little workaround fixes the problem. -Matt
	 */

	setMainWidget( m_mainWindow );
	int kUniqAppReturnCode = KUniqueApplication::newInstance();
	setMainWidget( 0L );

	return kUniqAppReturnCode;
}

void KopeteApplication::handleURLArgs()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
//	kdDebug(14000) << k_funcinfo << "called with " << args->count() << " arguments to handle." << endl;

	if ( args->count() > 0 )
	{
		for ( int i = 0; i < args->count(); i++ )
		{
			KURL u( args->url( i ) );
			if ( !u.isValid() )
				continue;

			Kopete::MimeTypeHandler::dispatchURL( u );
		} // END for()
	} // END args->count() > 0
}

void KopeteApplication::quitKopete()
{
	kdDebug( 14000 ) << k_funcinfo << endl;

	m_isShuttingDown = true;

	// close all windows
	QPtrListIterator<KMainWindow> it(*KMainWindow::memberList);
	for (it.toFirst(); it.current(); ++it)
	{
		if ( !it.current()->close() )
		{
			m_isShuttingDown = false;
			break;
		}
	}
}


void KopeteApplication::commitData( QSessionManager &sm )
{
	m_isShuttingDown = true;
	KUniqueApplication::commitData( sm );
}

#include "kopeteapplication.moc"
// vim: set noet ts=4 sts=4 sw=4:
