/*
    kopetewindow.cpp  -  Kopete Main Window

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn <sgehn@gmx.net>

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

#include "kopetewindow.h"

#include <qhbox.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kglobalaccel.h>
#include <kwin.h>
#include <kdeversion.h>

#include "addcontactwizard.h"
#include "kopete.h"
#include "kopeteaccount.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopeteaccountmanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopeteviewmanager.h"
#include "kopeteglobalawaydialog.h"
#include "pluginloader.h"
#include "preferencesdialog.h"
#include "systemtray.h"
#include "kopeteaccountstatusbaricon.h"
#include "kopeteprotocolstatusbaricon.h"

//#include "addaccountwizard.h"


KopeteWindow::KopeteWindow( QWidget *parent, const char *name )
: KMainWindow( parent, name )
{
//	kdDebug(14000) << k_funcinfo << "called." << endl;

	// Applications should ensure that their StatusBar exists before calling createGUI()
	// so that the StatusBar is always correctly positioned when KDE is configured to use
	// a MacOS-style MenuBar.
	// This fixes a "statusbar drawn over the top of the toolbar" bug
	// e.g. it can happen when you switch desktops on Kopete startup
	m_statusBarWidget = new QHBox(statusBar(), "m_statusBarWidget");
	m_statusBarWidget->setSpacing( 2 );
	m_statusBarWidget->setMargin( 2 );
	statusBar()->addWidget(m_statusBarWidget, 0, true);

	connect( KopetePrefs::prefs(), SIGNAL( saved() ),
		SLOT( slotSettingsChanged() ) );

	/* -------------------------------------------------------------------------------- */
	initView();
	initActions();
	initSystray();
	/* -------------------------------------------------------------------------------- */

	loadOptions();
	// Trap all loaded plugins, so we can add their status bar icons accordingly
	connect( LibraryLoader::pluginLoader(),
		SIGNAL( pluginLoaded( KopetePlugin * ) ),
		this, SLOT( slotPluginLoaded( KopetePlugin * ) ) );

	connect( KopeteAccountManager::manager(), SIGNAL(accountRegistered(KopeteAccount*)), this, SLOT(slotAccountRegistered(KopeteAccount*)));
	connect( KopeteAccountManager::manager(), SIGNAL(accountUnregistered(KopeteAccount*)), this, SLOT(slotAccountUnregistered(KopeteAccount*)));

}

void KopeteWindow::initView()
{
	contactlist = new KopeteContactListView(this);
	setCentralWidget(contactlist);

	// Initialize the Away message selection dialog
	m_awayMessageDialog = new KopeteGlobalAwayDialog(this);
}

void KopeteWindow::initActions()
{
	actionAddContact = new KAction( i18n( "&Add Contact..." ), "bookmark_add",
		0, this, SLOT( showAddContactDialog() ),
		actionCollection(), "AddContact" );

	actionConnect = new KAction( i18n( "&Connect All" ), "connect_creating",
		0, KopeteAccountManager::manager(), SLOT( connectAll() ),
		actionCollection(), "ConnectAll" );

	actionDisconnect = new KAction( i18n( "&Disconnect All" ), "connect_no",
		0, KopeteAccountManager::manager(), SLOT( disconnectAll() ),
		actionCollection(), "DisconnectAll" );

	actionConnectionMenu = new KActionMenu( i18n("Connection"),"connect_established",
							actionCollection(), "Connection" );

	actionConnectionMenu->insert(actionConnect);
	actionConnectionMenu->insert(actionDisconnect);

	actionSetAway = new KAction( i18n( "&Set Away Globally" ), "kopeteaway",
		0, this, SLOT( slotGlobalAwayMessageSelect() ),
		actionCollection(), "SetAwayAll" );

	actionSetAvailable = new KAction( i18n( "Set Availa&ble Globally" ),
		"kopeteavailable", 0 , KopeteAccountManager::manager(),
		SLOT( setAvailableAll() ), actionCollection(),
		"SetAvailableAll" );

	actionAwayMenu = new KActionMenu( i18n("Status"),"kopetestatus",
							actionCollection(), "Status" );
	actionAwayMenu->setDelayed( false );
	actionAwayMenu->insert(actionSetAvailable);
	actionAwayMenu->insert(actionSetAway);
	actionPrefs = KStdAction::preferences(
		this, SLOT( slotShowPreferencesDialog() ),
		actionCollection(), "settings_prefs" );

	actionSave = new KAction( i18n("Save &Contact List"), "filesave", KStdAccel::shortcut(KStdAccel::Save),
							this, SLOT(slotSaveContactList()),
							actionCollection(), "save_contactlist" );

	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

	toolbarAction = KStdAction::showToolbar(this, SLOT(showToolbar()), actionCollection(), "settings_showtoolbar" );
	menubarAction = KStdAction::showMenubar(this, SLOT(showMenubar()), actionCollection(), "settings_showmenubar" );
	statusbarAction = KStdAction::showStatusbar(this, SLOT(showStatusbar()), actionCollection(), "settings_showstatusbar");

	KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection(), "settings_keys");
	new KAction(i18n("Configure &Global Shortcuts"), "configure_shortcuts", 0, this,
			SLOT(slotConfGlobalKeys()), actionCollection(), "settings_global");

	KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection(), "settings_toolbars" );

	actionShowOffliners = new KToggleAction( i18n("Show Offline &Users"), "viewmag", CTRL+Key_V,
			this, SLOT(slotToggleShowOffliners()), actionCollection(), "settings_show_offliners" );

	// sync actions, config and prefs-dialog
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	slotConfigChanged();

	globalAccel = new KGlobalAccel( this );
	globalAccel->insert( QString::fromLatin1("Read Message"), i18n("Read Message"), i18n("Read the next pending message"),
		CTRL+SHIFT+Key_I, KKey::QtWIN+CTRL+Key_I, KopeteViewManager::viewManager(), SLOT(nextEvent()) );

	globalAccel->insert( QString::fromLatin1("Show / Hide Contact List"), i18n("Show / Hide Contact List"), i18n("Show or hide the contact list"),
		CTRL+SHIFT+Key_C, KKey::QtWIN+CTRL+Key_C, this, SLOT(slotShowHide()) );

	globalAccel->readSettings();
        globalAccel->updateConnections();

	createGUI ( "kopeteui.rc" );
}

void KopeteWindow::slotShowHide()
{
	if(isActiveWindow())
		hide();
	else
	{
		show();
		//raise() and show() should normaly deIconify the window. but it doesn't do here due
		// to a bug in QT or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
		if(isMinimized())
			KWin::deIconifyWindow(winId());
		if(!KWin::info(winId()).onAllDesktops)
			KWin::setOnDesktop(winId(), KWin::currentDesktop());
		raise();
		setActiveWindow();
	}
}

void KopeteWindow::initSystray()
{
	tray = KopeteSystemTray::systemTray( this, "KopeteSystemTray" );
	KPopupMenu *tm = tray->contextMenu();

	// NOTE: This is in reverse order because we insert
	// at the top of the menu, not at bottom!
	actionAddContact->plug( tm,1 );
	actionPrefs->plug(tm,1);
	tm->insertSeparator(1);
	actionAwayMenu->plug( tm,1 );
	actionConnectionMenu->plug ( tm,1 );
	tm->insertSeparator(1);

	QObject::connect(tray, SIGNAL(aboutToShowMenu(KPopupMenu*)), this, SLOT(slotTrayAboutToShowMenu(KPopupMenu*)));

#if KDE_VERSION >= 306
	QObject::connect(tray,SIGNAL(quitSelected()),this,SLOT(slotQuit()));
#endif
}

KopeteWindow::~KopeteWindow()
{
//	delete tray;
//	kdDebug(14000) << "[KopeteWindow] ~KopeteWindow()" << endl;
}

bool KopeteWindow::queryExit()
{
//	kdDebug(14000) << "[KopeteWindow] queryExit()" << endl;
	saveOptions();
	//Now in Kopete::~Kopete().
	// (KDE3.1 beta2 didn't save contact-list on exit)
	/*	KopeteContactList::contactList()->save();*/
	return true;
}


void KopeteWindow::loadOptions()
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->applySettings( config, "ToolBar Settings" );

	applyMainWindowSettings( config, "General Options" );

	QPoint pos = config->readPointEntry("Position");
	move(pos);

	QSize size = config->readSizeEntry("Geometry");
	if(size.isEmpty())
	{
		// Default size
		resize( QSize(220, 350) );
	}
	else
	{
		resize(size);
	}

	QString tmp = config->readEntry("State", "Shown");
	if ( tmp == "Minimized" )
	{
		showMinimized();
	}
	else if ( tmp == "Hidden" )
	{
		hide();
	}
	else
	{
		KopetePrefs *p = KopetePrefs::prefs();
		// with no systray-icon we force calling show()
		if ( !p->startDocked() || !p->showTray() )
			show();
	}

	toolbarAction->setChecked( !toolBar("mainToolBar")->isHidden() );
	menubarAction->setChecked( !menuBar()->isHidden() );
	statusbarAction->setChecked( !statusBar()->isHidden() );
}

void KopeteWindow::saveOptions()
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->saveSettings ( config, "ToolBar Settings" );

	saveMainWindowSettings( config, "General Options" );

	globalAccel->writeSettings();
	config->setGroup("General Options");
	config->writeEntry("Position", pos());
	config->writeEntry("Geometry", size());

	if(isMinimized())
	{
		config->writeEntry("State", "Minimized");
	}
	else if(isHidden())
	{
		config->writeEntry("State", "Hidden");
	}
	else
	{
		config->writeEntry("State", "Shown");
	}

	config->sync();
}

void KopeteWindow::showToolbar()
{
	if( toolbarAction->isChecked() )
		toolBar("mainToolBar")->show();
	else
		toolBar("mainToolBar")->hide();
}

void KopeteWindow::showMenubar()
{
	if( menubarAction->isChecked() )
		menuBar()->show();
	else
		menuBar()->hide();
}

void KopeteWindow::showStatusbar()
{
	if( statusbarAction->isChecked() )
		statusBar()->show();
	else
		statusBar()->hide();
}

void KopeteWindow::slotToggleShowOffliners()
{
	KopetePrefs *p = KopetePrefs::prefs();
	p->setShowOffline ( actionShowOffliners->isChecked() );

	disconnect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	p->save();
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
}

void KopeteWindow::slotConfigChanged()
{
	KopetePrefs *pref = KopetePrefs::prefs();

	if( isHidden() && !pref->showTray()) // user disabled systray while kopete is hidden, show it!
		show();

	actionShowOffliners->setChecked( pref->showOffline() );
}


void KopeteWindow::slotConfKeys()
{
	KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}

void KopeteWindow::slotConfGlobalKeys()
{
	KKeyDialog::configureKeys( globalAccel );
}

void KopeteWindow::slotConfToolbar()
{
	saveMainWindowSettings(KGlobal::config(), "General Options");
	KEditToolbar *dlg = new KEditToolbar(actionCollection(), "kopeteui.rc");
	if (dlg->exec())
	{
		createGUI("kopeteui.rc");
		applyMainWindowSettings(KGlobal::config(), "General Options");
	}
	delete dlg;
}

void KopeteWindow::slotGlobalAwayMessageSelect()
{
	// Show the dialog and set the message
	// This also tells the account manager to
	// set the away on all protocols
	m_awayMessageDialog->show();
}

void KopeteWindow::closeEvent( QCloseEvent *e )
{
	Kopete *kopeteapp = static_cast<Kopete*>(kapp);
	if (!kopeteapp)
		return;

	// also close if our tray icon is hidden!
	if(kopeteapp->isShuttingDown() || !KopetePrefs::prefs()->showTray() || !isShown() )
	{
		KMainWindow::closeEvent( e );
		return;
	}

#if KDE_VERSION >= 306
	KMessageBox::information( this,
		i18n( "<qt>Closing the main window will keep Kopete running in the "
		"system tray. Use Quit from the File menu to quit the "
		"application.</qt>" ), i18n( "Docking in System Tray" ),
		"hideOnCloseInfo" );
	hide();
	e->ignore();
#else
	KMainWindow::closeEvent( e );
#endif
}

void KopeteWindow::slotQuit()
{
	qApp->quit();
}

void KopeteWindow::slotPluginLoaded( KopetePlugin */* p  */)
{
//	kdDebug(14000) << "KopeteWindow::slotPluginLoaded()" << endl;
/*
	KopeteProtocol *proto = dynamic_cast<KopeteProtocol *>( p );
	if( !proto )
		return;

	connect( proto,
		SIGNAL( statusIconChanged( const KopeteOnlineStatus& ) ),
		SLOT( slotProtocolStatusIconChanged( const KopeteOnlineStatus& ) ) );
	connect( proto, SIGNAL( destroyed( QObject * ) ),
		SLOT( slotProtocolDestroyed( QObject * ) ) );

	StatusBarIcon *i = new StatusBarIcon( proto, m_statusBarWidget );
	connect( i, SIGNAL( rightClicked( KopeteProtocol *, const QPoint & ) ),
		SLOT( slotProtocolStatusIconRightClicked( KopeteProtocol *,
		const QPoint & ) ) );

	m_statusBarIcons.insert( proto, i );

	//slotProtocolStatusIconChanged( proto, proto->statusIcon() );

	KActionMenu *menu = proto->protocolActions();
	if( menu )
		menu->plug( tray->contextMenu(), 1 );
*/
}

void KopeteWindow::slotProtocolDestroyed( QObject */*o */)
{
/*
//	kdDebug(14000) << "KopeteWindow::slotProtocolDestroyed()" << endl;

	StatusBarIcon *i = static_cast<StatusBarIcon *>( m_statusBarIcons[ o ] );
	if( !i )
		return;

	delete i;
	m_statusBarIcons.remove( o );
*/
}

void KopeteWindow::slotAccountRegistered( KopeteAccount *a )
{
	kdDebug(14000) << k_funcinfo << "Called." << endl;

	if ( !a )
		return;

	connect( a,
		SIGNAL( onlineStatusIconChanged( KopeteAccount * ) ),
		SLOT( slotAccountStatusIconChanged( KopeteAccount * ) ) );

	KopeteAccountStatusBarIcon *i = new KopeteAccountStatusBarIcon( a, m_statusBarWidget );
	connect( i, SIGNAL( rightClicked( KopeteAccount *, const QPoint & ) ),
		SLOT( slotAccountStatusIconRightClicked( KopeteAccount *,
		const QPoint & ) ) );
	// Wanted by pmax, not sure if we should leave both in
	// it'll confuse users if we take out the RMB function
	connect( i, SIGNAL( leftClicked( KopeteAccount *, const QPoint & ) ),
		SLOT( slotAccountStatusIconRightClicked( KopeteAccount *,
		const QPoint & ) ) );

	m_accountStatusBarIcons.insert( a, i );

	// FIXME -Will
	//slotProtocolStatusIconChanged( proto, proto->statusIcon() );

	KActionMenu *menu = a->actionMenu();
	if( menu )
		menu->plug( tray->contextMenu(), 1 );
}

void KopeteWindow::slotAccountUnregistered( KopeteAccount *a)
{
	kdDebug(14000) << k_funcinfo << "Called." << endl;

	KopeteAccountStatusBarIcon *i = static_cast<KopeteAccountStatusBarIcon *>( m_accountStatusBarIcons[ a ] );
	if( !i )
		return;

	delete i;
	m_accountStatusBarIcons.remove( a );
}

void KopeteWindow::slotAccountStatusIconChanged( KopeteAccount *account )
{
	KopeteOnlineStatus status = account->myself()->onlineStatus();
	kdDebug(14000) << k_funcinfo << "Icon: '" <<
		status.overlayIcon() << "'" << endl;

	KopeteAccountStatusBarIcon *i = static_cast<KopeteAccountStatusBarIcon *>( m_accountStatusBarIcons[ account ] );
	if( !i )
		return;

	// Because we want null pixmaps to detect the need for a loadMovie
	// we can't use the SmallIcon() method directly
	KIconLoader *loader = KGlobal::instance()->iconLoader();

	QMovie mv = loader->loadMovie( status.overlayIcon(),
		 KIcon::User);

	if ( mv.isNull() )
	{
		// No movie found, fallback to pixmap
		// Get the icon for our status

		//QPixmap pm = SmallIcon( icon );
		QPixmap pm = status.iconFor( account );
		// Compat for the non-themed icons
		// FIXME: When all icons are converted, remove this - Martijn
		if( pm.isNull() )
			pm = loader->loadIcon( status.overlayIcon(),
				 KIcon::User, 0, KIcon::DefaultState, 0L, true );

		if( pm.isNull() )
		{
			/* No Pixmap found, fallback to Unknown */
			kdDebug(14000) << k_funcinfo
				<< "Using unknown pixmap for status icon '" << status.overlayIcon() << "'."
				<< endl;
			i->setPixmap( KIconLoader::unknown() );
		}
		else
		{
			i->setPixmap( pm );
		}
	}
	else
	{
		kdDebug(14000) << "KopeteWindow::slotProtocolStatusIconChanged(): "<< "Using movie."  << endl;
		i->setMovie( mv );
	}
}


void KopeteWindow::slotAccountStatusIconRightClicked( KopeteAccount *account,
	const QPoint &p )
{
	account->actionMenu()->popupMenu()->exec( p );
}

void KopeteWindow::slotProtocolStatusIconChanged( const KopeteOnlineStatus& status )
/*KopeteProtocol * p,	const QString &icon )*/
{
	kdDebug(14000) << k_funcinfo << "Icon: " <<
		status.overlayIcon() << endl;

	KopeteProtocolStatusBarIcon *i = static_cast<KopeteProtocolStatusBarIcon *>( m_protocolStatusBarIcons[ status.protocol() ] );
	if( !i )
		return;

	// Because we want null pixmaps to detect the need for a loadMovie
	// we can't use the SmallIcon() method directly
	KIconLoader *loader = KGlobal::instance()->iconLoader();

	QMovie mv = loader->loadMovie(status.overlayIcon(), KIcon::User);

	if ( mv.isNull() )
	{
		// No movie found, fallback to pixmap
		// Get the icon for our status

		//QPixmap pm = SmallIcon( icon );
		QPixmap pm = status.protocolIcon();
		// Compat for the non-themed icons
		// FIXME: When all icons are converted, remove this - Martijn
		if(pm.isNull())
			pm = loader->loadIcon( status.overlayIcon(), KIcon::User, 0, KIcon::DefaultState, 0L, true );

		if(pm.isNull())
		{
			/* No Pixmap found, fallback to Unknown */
			kdDebug(14000) << k_funcinfo <<
				"Using unknown pixmap for status icon '" << status.overlayIcon() <<
				"'." << endl;
			i->setPixmap( KIconLoader::unknown() );
		}
		else
		{
			i->setPixmap( pm );
		}
	}
	else
	{
		kdDebug(14000) << k_funcinfo << "Using movie." << endl;
		i->setMovie( mv );
	}
}

void KopeteWindow::slotTrayAboutToShowMenu( KPopupMenu * /* me */ )
{
	kdDebug(14000) << k_funcinfo << "Called. EMPTY" << endl;
}

void KopeteWindow::slotProtocolStatusIconRightClicked( KopeteProtocol *proto,
	const QPoint &p )
{
//	kdDebug(14000) << "KopeteWindow::slotProtocolStatusIconRightClicked()" << endl;
	// if the protocol has accounts, show its menu
	// otherwise just show a menu containing "Add New Account"
	KActionMenu *menu = 0L;

	QDict<KopeteAccount> dict=KopeteAccountManager::manager()->accounts( proto );
	if ( dict.count() > 0 )
		menu = proto->protocolActions();
/*	else
	{  // Fuck Kopete, this code won't be reached anyway, see slotAddAccount() [mETz]

		// FIXME: use the commented out KAction when we've solved using the addaccountwizard from
		// this class
		menu = new KActionMenu( proto->displayName(), proto->pluginIcon(), this);
		menu->insert( new KAction( i18n("Create Account"), QString::null, 0, qApp->mainWidget(), SLOT( slotAddAccount() ), menu, "actionKWAddAccount" ) );
	}*/

	if( menu )
	{
		menu->popupMenu()->exec( p );
		delete menu;
	}
}

/*
// TODO: Now I wasted 30mins on fixing addaccountwizard so this code works and
// suddenly I find out it's not needed anyway
// thanks pals for not removing the FIXME note in here :P [mETz]
void KopeteWindow::slotAddAccount()
{
	AddAccountWizard *mAddwizard;
	mAddwizard  = new AddAccountWizard( this , "addAccountWizard" , true);
	//connect(m_addwizard, SIGNAL( destroyed(QObject*)) , this, SLOT (slotAddWizardDone()));
	mAddwizard->show();
}
*/
void KopeteWindow::slotShowPreferencesDialog()
{
	// Although show() itself is a slot too we can't connect actions to it
	// from the KopeteWindow constructor, because we cannot access the
	// preferences dialog there yet.
	// If we did, qApp->mainWidget would still be 0L, and the dialog would
	// get no parent and wouldn't get deleted. In itself not that bad on
	// exit, but the KJanusWidget can't handle it properly and will cause
	// crashes.
	PreferencesDialog::preferencesDialog()->show();
}

void KopeteWindow::slotSaveContactList()
{
	KopeteContactList::contactList()->save();
	KMessageBox::information(this, i18n("Contact list saved."), i18n("Contact List Saved"));
}

void KopeteWindow::showAddContactDialog()
{
	(new AddContactWizard(qApp->mainWidget()))->show();
}

void KopeteWindow::slotSettingsChanged()
{
	// Account colouring may have changed, so tell our status bar to redraw
	kdDebug(14000) << k_funcinfo << endl;
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	QPtrListIterator<KopetePlugin> it( plugins );
	KopetePlugin *plugin = 0L;
	while ( ( plugin = it.current() ) != 0 )
	{
		++it;
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( plugin );
		if( !proto )
			continue;
		QDict<KopeteAccount> dict = KopeteAccountManager::manager()->accounts( proto );
		QDictIterator<KopeteAccount> it( dict );
		KopeteAccount *a;
		while ( ( a = it.current() ) != 0 )
		{
			++it;
			slotAccountStatusIconChanged( a );
		}
	}
}
#include "kopetewindow.moc"
// vim: set noet ts=4 sts=4 sw=4:
