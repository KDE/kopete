/*
    kopetewindow.cpp  -  Kopete Main Window

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005-2006 by Will Stephenson        <wstephenson@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QCursor>
#include <QLayout>
#include <QToolTip>
#include <QTimer>
#include <QPixmap>
#include <QCloseEvent>
#include <QEvent>
#include <QLabel>
#include <QShowEvent>
#include <QLineEdit>
#include <QSignalMapper>
#include <khbox.h>

#include <kaction.h>
#include <kactionclasses.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kmenu.h>

#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kglobalaccel.h>
#include <kwin.h>
#include <kdeversion.h>
#include <kinputdialog.h>
#include <kplugininfo.h>
#include <ksqueezedtextlabel.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <kdialog.h>

#include "addcontactpage.h"
#include "addressbooklinkwidget.h"
#include "ui_groupkabcselectorwidget.h"
#include "kabcexport.h"
#include "kopeteappearancesettings.h"
#include "kopeteapplication.h"
#include "kopeteaccount.h"
#include "kopeteaway.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccountstatusbaricon.h"
#include "kopetebehaviorsettings.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopetegroup.h"
#include "kopetelistviewsearchline.h"
#include "kopetechatsessionmanager.h"
#include "kopetepluginconfig.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteawayaction.h"
#include "kopeteuiglobal.h"
#include "systemtray.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteeditglobalidentitywidget.h"

//BEGIN GlobalStatusMessageIconLabel
GlobalStatusMessageIconLabel::GlobalStatusMessageIconLabel(QWidget *parent, const char *name)
 : QLabel(parent)
{
      setObjectName( name );
}

void GlobalStatusMessageIconLabel::mouseReleaseEvent( QMouseEvent *event )
{
      if( event->button() == Qt::LeftButton || event->button() == Qt::RightButton )
      {
              emit iconClicked( event->globalPos() );
              event->accept();
      }
}
//END GlobalStatusMessageIconLabel

/* KMainWindow is very broken from our point of view - it deref()'s the app
 * when the last visible KMainWindow is destroyed. But when our main window is
 * hidden when it's in the tray,closing the last chatwindow would cause the app
 * to quit. - Richard
 *
 * Fortunately KMainWindow checks queryExit before deref()ing the Kapplication.
 * KopeteWindow reimplements queryExit() and only returns true if it is shutting down
 * (either because the user quit Kopete, or the session manager did).
 *
 * KopeteWindow and ChatWindows are closed by session management.
 * App shutdown is not performed by the KopeteWindow but by KopeteApplication:
 * 1) user quit - KopeteWindow::slotQuit() was called, calls KopeteApplication::quitKopete(),
 *                which closes all chatwindows and the KopeteWindow.  The last window to close
 *                shuts down the PluginManager in queryExit().  When the PluginManager has completed its
 *                shutdown, the app is finally deref()ed, and the contactlist and accountmanager
 *                are saved.
 *                and calling KApplication::quit()
 * 2) session   - KopeteWindow and all chatwindows are closed by KApplication session management.
 *     quit        Then the shutdown proceeds as above.
 *
 * queryClose() is honoured so group chats and chats receiving recent messages can interrupt
 * (session) quit.
 */
 
KopeteWindow::KopeteWindow( QWidget *parent, const char *name )
: KMainWindow( parent, name, Qt::WType_TopLevel )
{
	// Applications should ensure that their StatusBar exists before calling createGUI()
	// so that the StatusBar is always correctly positioned when KDE is configured to use
	// a MacOS-style MenuBar.
	// This fixes a "statusbar drawn over the top of the toolbar" bug
	// e.g. it can happen when you switch desktops on Kopete startup
	m_statusBarWidget = new KHBox(statusBar());
	m_statusBarWidget->setObjectName( "m_statusBarWidget" );
	m_statusBarWidget->setMargin( 2 );
	m_statusBarWidget->setSpacing( 1 );
	statusBar()->addWidget(m_statusBarWidget, 0, true );
	KHBox *statusBarMessage = new KHBox(statusBar());
	m_statusBarWidget->setMargin( 2 );
	m_statusBarWidget->setSpacing( 1 );

	GlobalStatusMessageIconLabel *label = new GlobalStatusMessageIconLabel( statusBarMessage, "statusmsglabel" );
	label->setFixedSize( 16, 16 );
	label->setPixmap( SmallIcon( "kopetestatusmessage" ) );
	connect(label, SIGNAL(iconClicked( const QPoint& )),
		this, SLOT(slotGlobalStatusMessageIconClicked( const QPoint& )));
	QToolTip::add( label, i18n( "Global status message" ) );
	m_globalStatusMessage = new KSqueezedTextLabel( statusBarMessage );
	statusBar()->addWidget(statusBarMessage, 1, false );

	m_pluginConfig = 0L;
	m_autoHideTimer = new QTimer( this );

	// --------------------------------------------------------------------------------
	initView();
	initActions();
	contactlist->initActions(actionCollection());
	initSystray();
	// --------------------------------------------------------------------------------

	// Trap all loaded plugins, so we can add their status bar icons accordingly , also used to add XMLGUIClient
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin * ) ),
		this, SLOT( slotPluginLoaded( Kopete::Plugin * ) ) );
	connect( Kopete::PluginManager::self(), SIGNAL( allPluginsLoaded() ),
		this, SLOT( slotAllPluginsLoaded() ));
	//Connect the appropriate account signals
	/* Please note that I tried to put this in the slotAllPluginsLoaded() function
	 * but it seemed to break the account icons in the statusbar --Matt */

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
		this, SLOT(slotAccountRegistered(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
		this, SLOT(slotAccountUnregistered(const Kopete::Account*)));

	connect( m_autoHideTimer, SIGNAL( timeout() ), this, SLOT( slotAutoHide() ) );
	connect( Kopete::AppearanceSettings::self(), SIGNAL( contactListAppearanceChanged() ),
		this, SLOT( slotContactListAppearanceChanged() ) );
	createGUI ( "kopeteui.rc", false );

	// call this _after_ createGUI(), otherwise menubar is not set up correctly
	loadOptions();

	// If some plugins are already loaded, merge the GUI
	Kopete::PluginList plugins = Kopete::PluginManager::self()->loadedPlugins();
	Kopete::PluginList::ConstIterator it;
	for ( it = plugins.begin(); it != plugins.end(); ++it )
		slotPluginLoaded( *it );

	// If some account alrady loaded, build the status icon
	QListIterator<Kopete::Account *>  accit( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *a;
	while ( accit.hasNext() )
	{
		a = accit.next();
		slotAccountRegistered(a);
	}
    //install an event filter for the quick search toolbar so we can
    //catch the hide events
    toolBar( "quickSearchBar" )->installEventFilter( this );

}

void KopeteWindow::initView()
{
	contactlist = new KopeteContactListView(this);
	setCentralWidget(contactlist);
}

void KopeteWindow::initActions()
{
	// this action menu contains one action per account and is updated when accounts are registered/unregistered
	actionAddContact = new KActionMenu( KIcon("add_user"), i18n( "&Add Contact" ),
		actionCollection(), "AddContact" );
	actionAddContact->setDelayed( false );
	// this signal mapper is needed to call slotAddContact with the correct arguments
	addContactMapper = new QSignalMapper( this );
	connect( addContactMapper, SIGNAL( mapped( const QString & ) ),
		 this, SLOT( slotAddContactDialogInternal( const QString & ) ) );

	/* ConnectAll is now obsolete.  "Go online" has replaced it.
	actionConnect = new KAction( i18n( "&Connect Accounts" ), "connect_creating",
		0, Kopete::AccountManager::self(), SLOT( connectAll() ),
		actionCollection(), "ConnectAll" );
	*/

	actionDisconnect = new KAction( KIcon("connect_no"), i18n( "O&ffline" ), actionCollection(), "DisconnectAll" );
	connect( actionDisconnect, SIGNAL( triggered(bool) ), this, SLOT( slotDisconnectAll() ) );

	actionExportContacts = new KAction( i18n( "&Export Contacts..." ), actionCollection(), "ExportContacts" );
	connect( actionExportContacts, SIGNAL( triggered(bool) ), this, SLOT( showExportDialog() ) );

	/* the connection menu has been replaced by the set status menu
	actionConnectionMenu = new KActionMenu( i18n("Connection"),"connect_established",
							actionCollection(), "Connection" );

	actionConnectionMenu->setDelayed( false );
	actionConnectionMenu->insert(actionConnect);
	actionConnectionMenu->insert(actionDisconnect);
	actionConnect->setEnabled(false);
	*/
	actionDisconnect->setEnabled(false);

	selectAway = new KAction( KIcon("kopeteaway"), i18n("&Away"), actionCollection(), "SetAwayAll" );
	connect( selectAway, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalAway() ) );

	selectBusy = new KAction( KIcon("kopeteaway"), i18n("&Busy"), actionCollection(), "SetBusyAll" );
	connect( selectBusy, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalBusy() ) );


	actionSetInvisible = new KAction( KIcon("kopeteavailable"), i18n( "&Invisible" ), actionCollection(), "SetInvisibleAll" );
	connect( actionSetInvisible, SIGNAL( triggered(bool) ), this, SLOT( slotSetInvisibleAll() ) );

	/*actionSetAvailable = new KAction( i18n( "&Online" ),
		"kopeteavailable", 0 , Kopete::AccountManager::self(),
		SLOT( setAvailableAll() ), actionCollection(),
		"SetAvailableAll" );*/

	actionSetAvailable = new KAction( KIcon("kopeteavailable"), i18n("&Online"), actionCollection(), "SetAvailableAll" );
	connect( actionSetAvailable, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalAvailable() ) );

	actionAwayMenu = new KActionMenu( KIcon("kopeteavailable"), i18n("&Set Status"),
							actionCollection(), "Status" );
	actionAwayMenu->setDelayed( false );
	actionAwayMenu->insert(actionSetAvailable);
	actionAwayMenu->insert(selectAway);
	actionAwayMenu->insert(selectBusy);
	actionAwayMenu->insert(actionSetInvisible);
	actionAwayMenu->insert(actionDisconnect);

	actionPrefs = KopeteStdAction::preferences( actionCollection(), "settings_prefs" );

	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

	setStandardToolBarMenuEnabled(true);
	menubarAction = KStdAction::showMenubar(this, SLOT(showMenubar()), actionCollection(), "settings_showmenubar" );
	statusbarAction = KStdAction::showStatusbar(this, SLOT(showStatusbar()), actionCollection(), "settings_showstatusbar");

	KStdAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), actionCollection(), "settings_keys" );
	KAction *configurePluginAction = new KAction( KIcon("input_devices_settings"), i18n( "Configure Plugins..." ),
		actionCollection(), "settings_plugins" );
	connect( configurePluginAction, SIGNAL( triggered(bool) ), this, SLOT( slotConfigurePlugins() ) );

	KAction *configureGlobalShortcutsAction = new KAction( KIcon("configure_shortcuts"), i18n( "Configure &Global Shortcuts..." ),
		actionCollection(), "settings_global" );
	connect( configureGlobalShortcutsAction, SIGNAL( triggered(bool) ), this, SLOT( slotConfGlobalKeys() ) );

	KStdAction::configureToolbars( this, SLOT(slotConfToolbar()), actionCollection() );
	KStdAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection(), "settings_notifications" );

	actionShowOffliners = new KToggleAction( KIcon("show_offliners"), i18n( "Show Offline &Users" ), actionCollection(), "settings_show_offliners" );
	actionShowOffliners->setShortcut( KShortcut(Qt::CTRL + Qt::Key_U) );
	connect( actionShowOffliners, SIGNAL( triggered(bool) ), this, SLOT( slotToggleShowOffliners() ) );

	actionShowEmptyGroups = new KToggleAction( KIcon("folder_green"), i18n( "Show Empty &Groups" ), actionCollection(), "settings_show_empty_groups" );
	actionShowEmptyGroups->setShortcut( KShortcut(Qt::CTRL + Qt::Key_G) );
	connect( actionShowEmptyGroups, SIGNAL( triggered(bool) ), this, SLOT( slotToggleShowEmptyGroups() ) );

	actionShowOffliners->setCheckedState(i18n("Hide Offline &Users"));
	actionShowEmptyGroups->setCheckedState(i18n("Hide Empty &Groups"));

	// quick search bar
	QLabel *searchLabel = new QLabel( i18n("Se&arch:"), 0, "kde toolbar widget" );
	QWidget *searchBar = new Kopete::UI::ListView::SearchLine( 0, contactlist, "quicksearch_bar" );
	searchLabel->setBuddy( searchBar );
	KWidgetAction *quickSearch = new KWidgetAction( searchBar, i18n( "Quick Search Bar" ), 0, 0, 0, actionCollection(), "quicksearch_bar" );
	new KWidgetAction( searchLabel, i18n( "Search:" ), 0, 0, 0, actionCollection(), "quicksearch_label" );
// 	quickSearch->setAutoSized( true );
	// quick search bar - clear button
	KAction *resetQuickSearch = new KAction( QApplication::isRightToLeft() ? KIcon("clear_left") : KIcon("locationbar_erase"), i18n( "Reset Quick Search" ),
		actionCollection(), "quicksearch_reset" );
	connect( resetQuickSearch, SIGNAL( triggered(bool) ),  searchBar, SLOT( clear() ) );
	resetQuickSearch->setWhatsThis( i18n( "Reset Quick Search\n"
		"Resets the quick search so that all contacts and groups are shown again." ) );

	// Edit global identity widget/bar
	editGlobalIdentityWidget = new KopeteEditGlobalIdentityWidget(this, "editglobalBar");
	editGlobalIdentityWidget->hide();
	KWidgetAction *editGlobalAction = new KWidgetAction( editGlobalIdentityWidget, i18n("Edit Global Identity Widget"), 0, 0, 0, actionCollection(), "editglobal_widget");
// 	editGlobalAction->setAutoSized( true );

	// KActionMenu for selecting the global status message(kopeteonlinestatus_0)
	KActionMenu * setStatusMenu = new KActionMenu( KIcon("kopeteeditstatusmessage"), i18n( "Set Status Message" ), actionCollection(), "SetStatusMessage" );
	setStatusMenu->setDelayed( false );
	connect( setStatusMenu->popupMenu(), SIGNAL( aboutToShow() ), SLOT(slotBuildStatusMessageMenu() ) );
	connect( setStatusMenu->popupMenu(), SIGNAL( activated( int ) ), SLOT(slotStatusMessageSelected( int ) ) );

	// sync actions, config and prefs-dialog
	connect ( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()) );
	slotConfigChanged();

	// Global actions
	KAction *globalReadMessage = new KAction( i18n("Read Message"), actionCollection(), QLatin1String("Read Message") );
	connect( globalReadMessage, SIGNAL( triggered(bool) ), Kopete::ChatSessionManager::self(), SLOT( slotReadMessage() ) );
	globalReadMessage->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I) );
	globalReadMessage->setWhatsThis( i18n("Read the next pending message") );

	KAction *globalShowContactList = new KAction( i18n("Show/Hide Contact List"), actionCollection(), QLatin1String("Show/Hide Contact List") );
	connect( globalShowContactList, SIGNAL( triggered(bool) ), this, SLOT( slotShowHide() ) );
	globalShowContactList->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S) );
	globalShowContactList->setWhatsThis( i18n("Show or hide the contact list") );
	
	KAction *globalSetAway = new KAction( i18n("Set Away/Back"), actionCollection(), QLatin1String("Set Away/Back") );
	connect( globalSetAway, SIGNAL( triggered(bool) ), this, SLOT( slotToggleAway() ) );
	globalSetAway->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_W) );
	
	KGlobalAccel::self()->readSettings();
}

void KopeteWindow::slotShowHide()
{
	if(isActiveWindow())
	{
		m_autoHideTimer->stop(); //no timeouts if active
		hide();
	}
	else
	{
		show();
		//raise() and show() should normaly deIconify the window. but it doesn't do here due
		// to a bug in QT or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
		if(isMinimized())
			KWin::deIconifyWindow(winId());

		if(!KWin::windowInfo(winId(),NET::WMDesktop).onAllDesktops())
			KWin::setOnDesktop(winId(), KWin::currentDesktop());
		raise();
		activateWindow();
	}
}

void KopeteWindow::slotToggleAway()
{
	Kopete::Away *mAway = Kopete::Away::getInstance();
	if ( mAway->globalAway() )
	{
		Kopete::AccountManager::self()->setAvailableAll();
	}
	else
	{
		QString awayReason = mAway->getMessage( 0 );
		slotGlobalAway();
	}
}

void KopeteWindow::initSystray()
{
	m_tray = KopeteSystemTray::systemTray( this );
	Kopete::UI::Global::setSysTrayWId( m_tray->winId() );
	KMenu *tm = m_tray->contextMenu();

	// NOTE: This is in reverse order because we insert
	// at the top of the menu, not at bottom!
	actionAddContact->plug( tm, 1 );
	actionPrefs->plug( tm, 1 );
	tm->insertSeparator( 1 );
	actionAwayMenu->plug( tm, 1 );
	//actionConnectionMenu->plug ( tm, 1 );
	tm->insertSeparator( 1 );

	QObject::connect( m_tray, SIGNAL( aboutToShowMenu( KMenu * ) ),
		this, SLOT( slotTrayAboutToShowMenu( KMenu * ) ) );
	QObject::connect( m_tray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
}

KopeteWindow::~KopeteWindow()
{
	delete m_pluginConfig;
}

bool KopeteWindow::eventFilter( QObject* target, QEvent* event )
{
    KToolBar* toolBar = dynamic_cast<KToolBar*>( target );
    KAction* resetAction = actionCollection()->action( "quicksearch_reset" );

    if ( toolBar && resetAction && resetAction->isPlugged( toolBar ) )
    {

        if ( event->type() == QEvent::Hide )
        {
            resetAction->trigger();
            return true;
        }
        return KMainWindow::eventFilter( target, event );
    }

    return KMainWindow::eventFilter( target, event );
}

void KopeteWindow::loadOptions()
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->applySettings( config, "ToolBar Settings" );
	toolBar("quickSearchBar")->applySettings( config, "QuickSearchBar Settings" );
	toolBar("editGlobalIdentityBar")->applySettings( config, "EditGlobalIdentityBar Settings" );

	// FIXME: HACK: Is there a way to do that automatic ?
// 	editGlobalIdentityWidget->setIconSize(toolBar("editGlobalIdentityBar")->iconSize());
	connect(toolBar("editGlobalIdentityBar"), SIGNAL(modechange()), editGlobalIdentityWidget, SLOT(iconSizeChanged()));

	applyMainWindowSettings( config, "General Options" );
	config->setGroup("General Options");
	QPoint pos = config->readEntry("Position", QPoint());
	move(pos);

	QSize size = config->readEntry("Geometry", QSize() );
	if(size.isEmpty()) // Default size
		resize( QSize(220, 350) );
	else
		resize(size);

	m_autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	m_autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();


	QString tmp = config->readEntry("State", "Shown");
	if ( tmp == "Minimized" && Kopete::BehaviorSettings::self()->showSystemTray())
	{
		showMinimized();
	}
	else if ( tmp == "Hidden" && Kopete::BehaviorSettings::self()->showSystemTray())
	{
		hide();
	}
	else if ( !Kopete::BehaviorSettings::self()->startDocked() || !Kopete::BehaviorSettings::self()->showSystemTray() )
		show();

	menubarAction->setChecked( !menuBar()->isHidden() );
	statusbarAction->setChecked( !statusBar()->isHidden() );
}

void KopeteWindow::saveOptions()
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->saveSettings ( config, "ToolBar Settings" );
	toolBar("quickSearchBar")->saveSettings( config, "QuickSearchBar Settings" );
	toolBar("editGlobalIdentityBar")->saveSettings( config, "EditGlobalIdentityBar Settings" );

	saveMainWindowSettings( config, "General Options" );

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

void KopeteWindow::showMenubar()
{
	if(menubarAction->isChecked())
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
	Kopete::AppearanceSettings::self()->setShowOfflineUsers ( actionShowOffliners->isChecked() );
	Kopete::AppearanceSettings::self()->writeConfig();
}

void KopeteWindow::slotToggleShowEmptyGroups()
{
	Kopete::AppearanceSettings::self()->setShowEmptyGroups ( actionShowEmptyGroups->isChecked() );
	Kopete::AppearanceSettings::self()->writeConfig();
}

void KopeteWindow::slotConfigChanged()
{
	if( isHidden() && !Kopete::BehaviorSettings::self()->showSystemTray()) // user disabled systray while kopete is hidden, show it!
		show();

	actionShowOffliners->setChecked( Kopete::AppearanceSettings::self()->showOfflineUsers() );
	actionShowEmptyGroups->setChecked( Kopete::AppearanceSettings::self()->showEmptyGroups() );
}

void KopeteWindow::slotContactListAppearanceChanged()
{
	m_autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	m_autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();

	startAutoHideTimer();
}

void KopeteWindow::slotConfNotifications()
{
	KNotifyConfigWidget::configure( this );
}

void KopeteWindow::slotConfigurePlugins()
{
	if ( !m_pluginConfig )
		m_pluginConfig = new KopetePluginConfig( this );
	m_pluginConfig->show();

	m_pluginConfig->raise();

	KWin::activateWindow( m_pluginConfig->winId() );
}

void KopeteWindow::slotConfGlobalKeys()
{
	KKeyDialog::configure( actionCollection() );
}

void KopeteWindow::slotConfToolbar()
{
	saveMainWindowSettings(KGlobal::config(), "General Options");
	KEditToolbar *dlg = new KEditToolbar(factory());
	connect( dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotUpdateToolbar()) );
	connect( dlg, SIGNAL(finished()) , dlg, SLOT(deleteLater()));
	dlg->show();
}

void KopeteWindow::slotUpdateToolbar()
{
	applyMainWindowSettings(KGlobal::config(), "General Options");
}

void KopeteWindow::slotGlobalAway()
{
	Kopete::AccountManager::self()->setAwayAll( m_globalStatusMessageStored );
}

void KopeteWindow::slotGlobalBusy()
{
	Kopete::AccountManager::self()->setOnlineStatus(
			Kopete::OnlineStatusManager::Busy, m_globalStatusMessageStored );
}

void KopeteWindow::slotGlobalAvailable()
{
	Kopete::AccountManager::self()->setAvailableAll( m_globalStatusMessageStored );
}

void KopeteWindow::slotSetInvisibleAll()
{
	Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Invisible  );
}

void KopeteWindow::slotDisconnectAll()
{
	m_globalStatusMessage->setText( "" );
	m_globalStatusMessageStored = QString();
	Kopete::AccountManager::self()->disconnectAll();
}

bool KopeteWindow::queryClose()
{
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
	if ( !app->sessionSaving()	// if we are just closing but not shutting down
		&& !app->isShuttingDown()
		&& Kopete::BehaviorSettings::self()->showSystemTray()
		&& !isHidden() )
		// I would make this a KMessageBox::queuedMessageBox but there doesn't seem to be don'tShowAgain support for those
		KMessageBox::information( this,
								  i18n( "<qt>Closing the main window will keep Kopete running in the "
								        "system tray. Use 'Quit' from the 'File' menu to quit the application.</qt>" ),
								  i18n( "Docking in System Tray" ), "hideOnCloseInfo" );
// 	else	// we are shutting down either user initiated or session management
// 		Kopete::PluginManager::self()->shutdown();

	return true;
}

bool KopeteWindow::queryExit()
{
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
 	if ( app->sessionSaving()
		|| app->isShuttingDown() /* only set if KopeteApplication::quitKopete() or
									KopeteApplication::commitData() called */
		|| !Kopete::BehaviorSettings::self()->showSystemTray() /* also close if our tray icon is hidden! */
		|| isHidden() )
	{
		kDebug( 14000 ) << k_funcinfo << " shutting down plugin manager" << endl;
		Kopete::PluginManager::self()->shutdown();
		return true;
	}
	else
		return false;
}

void KopeteWindow::closeEvent( QCloseEvent *e )
{
	// if there's a system tray applet and we are not shutting down then just do what needs to be done if a
	// window is closed.
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
	if ( Kopete::BehaviorSettings::self()->showSystemTray() && !app->isShuttingDown() && !app->sessionSaving() ) {
		// BEGIN of code borrowed from KMainWindow::closeEvent
		// Save settings if auto-save is enabled, and settings have changed
		if ( settingsDirty() && autoSaveSettings() )
			saveAutoSaveSettings();

		if ( queryClose() ) {
			e->accept();
		}
		// END of code borrowed from KMainWindow::closeEvent
		kDebug( 14000 ) << k_funcinfo << "just closing because we have a system tray icon" << endl;
	}
	else
	{
		kDebug( 14000 ) << k_funcinfo << "delegating to KMainWindow::closeEvent()" << endl;
		KMainWindow::closeEvent( e );
	}
}

void KopeteWindow::slotQuit()
{
	saveOptions();
	KopeteApplication *app = static_cast<KopeteApplication *>( kapp );
	app->quitKopete();
}

void KopeteWindow::slotPluginLoaded( Kopete::Plugin *  p  )
{
	guiFactory()->addClient(p);
}

void KopeteWindow::slotAllPluginsLoaded()
{
//	actionConnect->setEnabled(true);
	actionDisconnect->setEnabled(true);
}

void KopeteWindow::slotAccountRegistered( Kopete::Account *account )
{
//	kDebug(14000) << k_funcinfo << "Called." << endl;
	if ( !account )
		return;

	//enable the connect all toolbar button
//	actionConnect->setEnabled(true);
	actionDisconnect->setEnabled(true);

	connect( account->myself(),
		SIGNAL(onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &) ),
		this, SLOT( slotAccountStatusIconChanged( Kopete::Contact * ) ) );

//	connect( account, SIGNAL( iconAppearanceChanged() ), SLOT( slotAccountStatusIconChanged() ) );
	connect( account, SIGNAL( colorChanged(const QColor& ) ), SLOT( slotAccountStatusIconChanged() ) );

	//FIXME:  i don't know why this is require, cmmenting for now)
#if 0
	connect( account->myself(),
		SIGNAL(propertyChanged( Kopete::Contact *, const QString &, const QVariant &, const QVariant & ) ),
		this, SLOT( slotAccountStatusIconChanged( Kopete::Contact* ) ) );
#endif

	KopeteAccountStatusBarIcon *sbIcon = new KopeteAccountStatusBarIcon( account, m_statusBarWidget );
	connect( sbIcon, SIGNAL( rightClicked( Kopete::Account *, const QPoint & ) ),
		SLOT( slotAccountStatusIconRightClicked( Kopete::Account *,
		const QPoint & ) ) );
	connect( sbIcon, SIGNAL( leftClicked( Kopete::Account *, const QPoint & ) ),
		SLOT( slotAccountStatusIconRightClicked( Kopete::Account *,
		const QPoint & ) ) );

	m_accountStatusBarIcons.insert( account, sbIcon );
	slotAccountStatusIconChanged( account->myself() );
	
	// add an item for this account to the add contact actionmenu
	QString s = "actionAdd%1Contact";
	s.arg( account->accountId() );
	KAction *action = new KAction( account->accountLabel(), account->accountIcon(), 0 , addContactMapper, SLOT( map() ), 0, s.toLatin1() );
	addContactMapper->setMapping( action, account->protocol()->pluginId() + QChar(0xE000) + account->accountId() );
	actionAddContact->insert( action );
}

void KopeteWindow::slotAccountUnregistered( const Kopete::Account *account)
{
//	kDebug(14000) << k_funcinfo << "Called." << endl;
	QList<Kopete::Account *> accounts = Kopete::AccountManager::self()->accounts();
	if (accounts.isEmpty())
	{
//		actionConnect->setEnabled(false);
		actionDisconnect->setEnabled(false);
	}

	// the (void*)  is to remove the const.  i don't know why QPtrList doesn't accept const ptr as key.
	KopeteAccountStatusBarIcon *sbIcon = static_cast<KopeteAccountStatusBarIcon *>( m_accountStatusBarIcons[ (void*)account ] );

	if( !sbIcon )
		return;

	m_accountStatusBarIcons.remove( (void*)account );
	delete sbIcon;

	makeTrayToolTip();
	
	// update add contact actionmenu
	QString s = "actionAdd%1Contact";
	s.arg( account->accountId() );
// 	KAction * action = actionCollection()->action( account->accountId() );
	Kopete::Account * myAccount = const_cast< Kopete::Account * > ( account );
	KAction * action = static_cast< KAction *>( myAccount->child( s.toLatin1() ) );
	if ( action )
	{
		kDebug(14000) << " found KAction " << action << " with name: " << action->objectName() << endl;
		addContactMapper->removeMappings( action );
		actionAddContact->remove( action );
	}
}

void KopeteWindow::slotAccountStatusIconChanged()
{
	if ( const Kopete::Account *from = dynamic_cast<const Kopete::Account*>(sender()) )
		slotAccountStatusIconChanged( from->myself() );
}

void KopeteWindow::slotAccountStatusIconChanged( Kopete::Contact *contact )
{
	kDebug( 14000 ) << k_funcinfo << contact->property( Kopete::Global::Properties::self()->statusMessage() ).value() << endl;
	// update the global status label if the change doesn't 
//	QString newAwayMessage = contact->property( Kopete::Global::Properties::self()->awayMessage() ).value().toString();
	Kopete::OnlineStatus status = contact->onlineStatus();
// 	if ( status.status() != Kopete::OnlineStatus::Connecting )
// 	{
// 		QString globalMessage = m_globalStatusMessage->text();
// 		if ( newAwayMessage != globalMessage )
// 			m_globalStatusMessage->setText( ""i18n("status message to show when different accounts have different status messages", "(multiple)" );
// 	}
//	kDebug(14000) << k_funcinfo << "Icons: '" <<
//		status.overlayIcons() << "'" << endl;

	if ( status != Kopete::OnlineStatus::Connecting )
	{
		m_globalStatusMessageStored = contact->property( Kopete::Global::Properties::self()->statusMessage() ).value().toString();
		m_globalStatusMessage->setText( m_globalStatusMessageStored );
	}
	
	KopeteAccountStatusBarIcon *i = static_cast<KopeteAccountStatusBarIcon *>( m_accountStatusBarIcons[ contact->account() ] );
	if( !i )
		return;

	// Adds tooltip for each status icon,
	// useful in case you have many accounts
	// over one protocol
	QToolTip::remove( i );
	QToolTip::add( i, contact->toolTip() );

	// Because we want null pixmaps to detect the need for a loadMovie
	// we can't use the SmallIcon() method directly
	KIconLoader *loader = KGlobal::instance()->iconLoader();
#if 0
	QMovie *mv = new QMovie(loader->loadMovie( status.overlayIcons().first(), K3Icon::Small ));

	if ( mv->isNull() )
	{
		// No movie found, fallback to pixmap
		// Get the icon for our status
#endif
		//QPixmap pm = SmallIcon( icon );
		QPixmap pm = status.iconFor( contact->account() );

		// No Pixmap found, fallback to Unknown
		if( pm.isNull() )
			i->setPixmap( KIconLoader::unknown() );
		else
			i->setPixmap( pm );
#if 0
	}
	else
	{
		//kDebug( 14000 ) << k_funcinfo << "Using movie."  << endl;
		i->setMovie( mv );
	}
#endif
	makeTrayToolTip();
}

void KopeteWindow::makeTrayToolTip()
{
	//the tool-tip of the systemtray.
	if(m_tray)
	{
		QToolTip::remove(m_tray);

		QString tt = QString::fromLatin1("<qt>");
		QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
		Kopete::Account *a;
		while ( it.hasNext() )
		{
			a = it.next();
			Kopete::Contact *self = a->myself();
			tt += i18nc( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)<br/>",
			             "<nobr><img src=\"kopete-account-icon:%3:%4\"> <b>%1:</b> %2 (<i>%5</i>)<br/>",
				     a->protocol()->displayName(), a->accountLabel(), QString(QUrl::toPercentEncoding( a->protocol()->pluginId() )),
				     QString(QUrl::toPercentEncoding( a->accountId() )), self->onlineStatus().description() );
		}
		tt += QString::fromLatin1("</qt>");
		QToolTip::add(m_tray, tt);
	}
}

void KopeteWindow::slotAccountStatusIconRightClicked( Kopete::Account *account, const QPoint &p )
{
	KActionMenu *actionMenu = account->actionMenu();
	if ( !actionMenu )
		return;

	connect( actionMenu->popupMenu(), SIGNAL( aboutToHide() ), actionMenu, SLOT( deleteLater() ) );
	actionMenu->popupMenu()->popup( p );
}

void KopeteWindow::slotTrayAboutToShowMenu( KMenu * popup )
{
	QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() );
	Kopete::Account *a;
	while ( it.hasNext() )
	{
		a = it.next();
		KActionMenu *menu = a->actionMenu();
		if( menu )
			menu->plug(popup, 1 );

		connect(popup , SIGNAL(aboutToHide()) , menu , SLOT(deleteLater()));
	}

}



/*void KopeteWindow::slotProtocolStatusIconRightClicked( Kopete::Protocol *proto, const QPoint &p )
{
	//kDebug( 14000 ) << k_funcinfo << endl;
	if ( Kopete::AccountManager::self()->accounts( proto ).count() > 0 )
	{
		KActionMenu *menu = proto->protocolActions();

		connect( menu->popupMenu(), SIGNAL( aboutToHide() ), menu, SLOT( deleteLater() ) );
		menu->popupMenu()->popup( p );
	}
}*/

void KopeteWindow::showExportDialog()
{
	( new KabcExportWizard( this, "export_contact_dialog" ) )->show();
}

void KopeteWindow::leaveEvent( QEvent * )
{
	startAutoHideTimer();
}

void KopeteWindow::showEvent( QShowEvent * )
{
	startAutoHideTimer();
}

void KopeteWindow::slotAutoHide()
{
	if ( this->geometry().contains( QCursor::pos() ) == false )
	{
		/* The autohide-timer doesn't need to emit
		* timeouts when the window is hidden already. */
		m_autoHideTimer->stop();
		hide();
	}
}

void KopeteWindow::startAutoHideTimer()
{
	if ( m_autoHideTimeout > 0 && m_autoHide == true && isVisible() && Kopete::BehaviorSettings::self()->showSystemTray())
		m_autoHideTimer->start( m_autoHideTimeout * 1000 );
}

// Iterate each connected account, updating its status message bug keeping the 
// same onlinestatus.  Then update Kopete::Away and the UI.
void KopeteWindow::setStatusMessage( const QString & message )
{
	bool changed = false;
	QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
	QList<Kopete::Account*>::const_iterator it, itEnd = accountList.constEnd();
	for ( QList<Kopete::Account*>::const_iterator it = accountList.constBegin(); it != itEnd; ++it )
	{
		Kopete::Contact *self = (*it)->myself();
		bool isInvisible = self && self->onlineStatus().status() == Kopete::OnlineStatus::Invisible;
		if ( (*it)->isConnected() && !isInvisible )
		{
			changed = true;
			(*it)->setOnlineStatus( self->onlineStatus(), message );
		}
	}
	Kopete::Away::getInstance()->setGlobalAwayMessage( message );
	m_globalStatusMessageStored = message;
	m_globalStatusMessage->setText( message );
}

void KopeteWindow::slotBuildStatusMessageMenu()
{
	
#ifdef __GNUC__
#warning Status message box popup needs porting
#endif
#if 0
	QObject * senderObj = const_cast<QObject *>( sender() );
	m_globalStatusMessageMenu = static_cast<KMenu *>( senderObj );
	m_globalStatusMessageMenu->clear();
// pop up a menu containing the away messages, and a lineedit
// see kopeteaway
	//messageMenu = new KPopupMenu( this );
//	messageMenu->insertTitle( i18n( "Status Message" ) );
	KHBox * newMessageBox = new KHBox( 0 );
	newMessageBox->setMargin( 1 );
	QLabel * newMessagePix = new QLabel( newMessageBox );
	newMessagePix->setPixmap( SmallIcon( "edit" ) );
/*	QLabel * newMessageLabel = new QLabel( i18n( "Add " ), newMessageBox );*/
	m_newMessageEdit = new QLineEdit( newMessageBox, "newmessage" );
	
	newMessageBox->setFocusProxy( m_newMessageEdit );
	newMessageBox->setFocusPolicy( Qt::ClickFocus );
/*	newMessageLabel->setFocusProxy( newMessageEdit );
	newMessageLabel->setBuddy( newMessageEdit );
	newMessageLabel->setFocusPolicy( QWidget::ClickFocus );*/
	newMessagePix->setFocusProxy( m_newMessageEdit );
	newMessagePix->setFocusPolicy( Qt::ClickFocus );
	connect( m_newMessageEdit, SIGNAL( returnPressed() ), SLOT( slotNewStatusMessageEntered() ) );

	m_globalStatusMessageMenu->insertItem( newMessageBox );

	int i = 0;
	
	m_globalStatusMessageMenu->insertItem( SmallIcon( "remove" ), i18n( "No Message" ), i++ );
	m_globalStatusMessageMenu->insertSeparator();
	
	QStringList awayMessages = Kopete::Away::getInstance()->getMessages();
	for( QStringList::iterator it = awayMessages.begin(); it != awayMessages.end(); ++it, ++i )
	{
		m_globalStatusMessageMenu->insertItem( KStringHandler::rsqueeze( *it ), i );
	}
//	connect( m_globalStatusMessageMenu, SIGNAL( activated( int ) ), SLOT( slotStatusMessageSelected( int ) ) );
//	connect( messageMenu, SIGNAL( aboutToHide() ), messageMenu, SLOT( deleteLater() ) );

	m_newMessageEdit->setFocus();

	//messageMenu->popup( e->globalPos(), 1 );
#endif
}

void KopeteWindow::slotStatusMessageSelected( int i )
{
	Kopete::Away *away = Kopete::Away::getInstance();
	if ( 0 == i )
		setStatusMessage( "" );
	else
		setStatusMessage( away->getMessage( i - 1 ) );
}

void KopeteWindow::slotNewStatusMessageEntered()
{
	m_globalStatusMessageMenu->close();
	QString newMessage = m_newMessageEdit->text();
	if ( !newMessage.isEmpty() )
		Kopete::Away::getInstance()->addMessage( newMessage );
	setStatusMessage( m_newMessageEdit->text() );
}

void KopeteWindow::slotGlobalStatusMessageIconClicked( const QPoint &position )
{
	KMenu *statusMessageIconMenu = new KMenu(this);
	connect(statusMessageIconMenu, SIGNAL( aboutToShow() ),
		this, SLOT(slotBuildStatusMessageMenu()));
	connect( statusMessageIconMenu, SIGNAL( activated( int ) ),
				SLOT( slotStatusMessageSelected( int ) ) );

	statusMessageIconMenu->popup(position);
}

void KopeteWindow::slotAddContactDialogInternal( const QString & accountIdentifier )
{
	QString protocolId = accountIdentifier.section( QChar(0xE000), 0, 0 );
	QString accountId = accountIdentifier.section( QChar(0xE000), 1, 1 );
	Kopete::Account *account = Kopete::AccountManager::self()->findAccount( protocolId, accountId );
 	showAddContactDialog( account );
}

void KopeteWindow::showAddContactDialog( Kopete::Account * account )
{
	if ( !account ) {
		kDebug( 14000 ) << k_funcinfo << "no account given" << endl; 
		return;
	}

	KDialog *addDialog = new KDialog( this, i18n( "Add Contact" ), KDialog::Ok | KDialog::Cancel );
	addDialog->setDefaultButton( KDialog::Ok );
	addDialog->enableButtonSeparator( true );

	KVBox * mainWid = new KVBox( addDialog );
	
	AddContactPage *addContactPage =
		account->protocol()->createAddContactWidget( mainWid, account );

	QWidget* groupKABC = new QWidget( mainWid );
	groupKABC->setObjectName( "groupkabcwidget" );
	Ui::GroupKABCSelectorWidget ui_groupKABC;
	ui_groupKABC.setupUi( groupKABC );

	// Populate the groups list
	Kopete::GroupList groups=Kopete::ContactList::self()->groups();
	QHash<QString, Kopete::Group*> groupItems;
	foreach( Kopete::Group *group, groups )
    {
		QString groupname = group->displayName();
		if ( !groupname.isEmpty() )
		{
			groupItems.insert( groupname, group );
			ui_groupKABC.groupCombo->insertItem( groupname );
		}
	}

	if (!addContactPage)
	{
		kDebug(14000) << k_funcinfo <<
			"Error while creating addcontactpage" << endl;
	}
	else
	{
		addDialog->setMainWidget( mainWid );
		if( addDialog->exec() == QDialog::Accepted )
		{
			if( addContactPage->validateData() )
			{
				Kopete::MetaContact * metacontact = new Kopete::MetaContact();
				metacontact->addToGroup( groupItems[ ui_groupKABC.groupCombo->currentText() ] );
				metacontact->setMetaContactId( ui_groupKABC.widAddresseeLink->uid() );
				if (addContactPage->apply( account, metacontact ))
				{
					Kopete::ContactList::self()->addMetaContact( metacontact );
				}
				else
				{
					delete metacontact;
				}
			}
		}
	}
	addDialog->deleteLater();
}

#include "kopetewindow.moc"
// vim: set noet ts=4 sts=4 sw=4:
