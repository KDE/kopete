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
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <solid/networking.h>

#include "addaccountwizard.h"
#include "kabcpersistence.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetestatusmanager.h"
#include "kopetestatusitems.h"
#include "kopetebehaviorsettings.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopeteglobal.h"
#include "kopetefileengine.h"
#include "kopetemimetypehandler.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"
#include "kopetewindow.h"
#include "kopeteviewmanager.h"
#include "kopeteidentitymanager.h"
#include "kopetedbusinterface.h"

KopeteApplication::KopeteApplication()
: KUniqueApplication( true, true )
{
	setQuitOnLastWindowClosed( false );
	m_isShuttingDown = false;

	//Create the identity manager
	Kopete::IdentityManager::self()->load();

	m_mainWindow = new KopeteWindow( 0 );

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
	 * KXmlGuiWindow (a KopeteWindow instance exists already) is connected. KXmlGuiWindow's
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
	QTimer::singleShot( 0, this, SLOT(slotLoadPlugins()) );

	m_fileEngineHandler = new Kopete::FileEngineHandler();

	//Create the emoticon installer
	m_emoticonHandler = new Kopete::EmoticonMimeTypeHandler;

	//Create the DBus interface for org.kde.kopete
	new KopeteDBusInterface(this);
}

KopeteApplication::~KopeteApplication()
{
	kDebug( 14000 ) ;

	if ( ! m_isShuttingDown )
	{
		// destruct was called without proper shutdown, dbus quit maybe?
		m_isShuttingDown = true;

		// close all windows
		QList<KMainWindow*> members = KMainWindow::memberList();
		QList<KMainWindow*>::iterator it, itEnd = members.end();
		for ( it = members.begin(); it != itEnd; ++it )
			(*it)->close();

		// shutdown plugin manager
		Kopete::PluginManager::self()->shutdown();

		// destroy all plugins until KopeteApplication is alive
		Kopete::PluginList list = Kopete::PluginManager::self()->loadedPlugins();
		foreach ( Kopete::Plugin *plugin, list )
			delete plugin;
	}

	delete m_fileEngineHandler;
	delete m_emoticonHandler;
	//kDebug( 14000 ) << "Done";
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

	// the account manager should be created after the identity manager is created
	Kopete::AccountManager::self()->load();
	Kopete::ContactList::self()->load();

	KSharedConfig::Ptr config = KGlobal::config();

	// Parse command-line arguments
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	bool showConfigDialog = false;

	KConfigGroup pluginsGroup = config->group( "Plugins" );

	/* FIXME: This is crap, if something purged that groups but your accounts
	 * are still working kopete will load the necessary plugins but still show the
	 * stupid accounts dialog (of course empty at that time because account data
	 * gets loaded later on). [mETz - 29.05.2004]
	 */
	if ( !pluginsGroup.exists() )
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
	foreach ( const QString &disableArg, args->getOption( "disable" ).split( ',' ))
	{
		showConfigDialog = false;
		Kopete::PluginManager::self()->setPluginEnabled( disableArg, false );
	}

	// Load some plugins exclusively? (--load-plugins=foo,bar)
	if ( args->isSet( "load-plugins" ) )
	{
		pluginsGroup.deleteGroup( KConfigBase::Global );
		showConfigDialog = false;
		foreach ( const QString &plugin, args->getOption( "load-plugins" ).split( ',' ))
			Kopete::PluginManager::self()->setPluginEnabled( plugin, true );
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
		QTimer::singleShot(0, this, SLOT(slotAllPluginsLoaded()));
	}
	else
	{
		Kopete::PluginManager::self()->loadAllPlugins();
	}

	connect( Kopete::PluginManager::self(), SIGNAL(allPluginsLoaded()),
		this, SLOT(slotAllPluginsLoaded()));

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
		AddAccountWizard *m_addwizard = new AddAccountWizard( Kopete::UI::Global::mainWidget(), true );
		m_addwizard->exec();
		Kopete::AccountManager::self()->save();
	}
}


void KopeteApplication::slotAllPluginsLoaded()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	//FIXME: this should probably ask for the identities to connect instead of all accounts
	// --noconnect not specified?

	Kopete::OnlineStatusManager::Category initStatus = Kopete::OnlineStatusManager::self()->initialStatus();
	Kopete::OnlineStatusManager::Category setStatus = Kopete::OnlineStatusManager::Offline;

	if ( args->isSet( "connect" )  &&  initStatus != Kopete::OnlineStatusManager::Offline &&
			( Solid::Networking::status() == Solid::Networking::Unknown ||
			  Solid::Networking::status() == Solid::Networking::Connected ) ){

		setStatus = initStatus;

	}

	QList <Kopete::Status::StatusItem *> statusList = Kopete::StatusManager::self()->getRootGroup()->childList();
	QString message, title;
	bool found = false;

	//find first Status for OnlineStatus
	for ( QList <Kopete::Status::StatusItem *>::ConstIterator it = statusList.constBegin(); it != statusList.constEnd(); ++it ) {
		if ( ! (*it)->isGroup() && (*it)->category() == setStatus ) {
			title = (*it)->title();
			message = (static_cast <Kopete::Status::Status*> (*it))->message(); //if it is not group, it status
			found = true;
			break;
		}
	}

	if ( found )
	{

		if ( setStatus != Kopete::OnlineStatusManager::Offline )
		{
			Kopete::AccountManager::self()->setOnlineStatus(initStatus, Kopete::StatusMessage(title, message), Kopete::AccountManager::ConnectIfOffline);
		}

		Kopete::StatusManager::self()->setGlobalStatus(setStatus, Kopete::StatusMessage(title, message));

	} else {

		if ( setStatus != Kopete::OnlineStatusManager::Offline )
		{
			Kopete::AccountManager::self()->setOnlineStatus(initStatus, QString(), Kopete::AccountManager::ConnectIfOffline);
		}

	}

 	kDebug(14000)<< "initial status set in config: " << initStatus;

	QStringList connectArgs = args->getOptionList( "autoconnect" );

	// toConnect will contain all the protocols to connect to
	QStringList toConnect;

	for ( QStringList::ConstIterator i = connectArgs.constBegin(); i != connectArgs.constEnd(); ++i )
	{
		foreach ( const QString &connectArg, (*i).split(','))
			toConnect.append( connectArg );
	}

	for ( QStringList::ConstIterator i = toConnect.constBegin(); i != toConnect.constEnd(); ++i )
	{
		QRegExp rx( QLatin1String( "([^\\|]*)\\|\\|(.*)" ) );
		rx.indexIn( *i );
		QString protocolId = rx.cap( 1 );
		QString accountId = rx.cap( 2 );

		if ( accountId.isEmpty() )
		{
			if ( protocolId.isEmpty() )
				accountId = *i;
			else
				continue;
		}

		QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
		Kopete::Account *account;
		while ( it.hasNext() )
		{
			account = it.next();
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
//	kDebug(14000) ;
	handleURLArgs();

	return KUniqueApplication::newInstance();
}

void KopeteApplication::handleURLArgs()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
//	kDebug(14000) << "called with " << args->count() << " arguments to handle.";

	if ( args->count() > 0 )
	{
		for ( int i = 0; i < args->count(); i++ )
		{
			KUrl u( args->url( i ) );
			if ( !u.isValid() )
				continue;

			Kopete::MimeTypeHandler::dispatchURL( u );
		} // END for()
	} // END args->count() > 0
}


void KopeteApplication::quitKopete()
{
	kDebug( 14000 ) ;

	m_isShuttingDown = true;

	// close all windows
	QList<KMainWindow*> members = KMainWindow::memberList();
	QList<KMainWindow*>::iterator it, itEnd = members.end();
	for ( it = members.begin(); it != itEnd; ++it)
	{
		if ( !(*it)->close() )
		{
			m_isShuttingDown = false;
			break;
		}
	}

	if ( m_isShuttingDown && m_mainWindow )
	{
		m_mainWindow->deleteLater();
		m_mainWindow = 0;
	}
}


void KopeteApplication::commitData( QSessionManager &sm )
{
	m_isShuttingDown = true;
	KUniqueApplication::commitData( sm );
}

#include "kopeteapplication.moc"
// vim: set noet ts=4 sts=4 sw=4:
