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

#include <QTimer>
#include <QPixmap>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QLabel>
#include <QShowEvent>
#include <QLineEdit>
#include <QSignalMapper>

#include <khbox.h>
#include <kvbox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kmenu.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kwindowsystem.h>
#include <kdeversion.h>
#include <kinputdialog.h>
#include <kplugininfo.h>
#include <ksqueezedtextlabel.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <kdialog.h>
#include <kstandardaction.h>
#include <solid/networking.h>
#include <kstatusbarofflineindicator.h>

#include "addcontactpage.h"
#include "addressbooklinkwidget.h"
#include "ui_groupkabcselectorwidget.h"
#include "kabcexport.h"
#include "kopeteappearancesettings.h"
#include "kopeteapplication.h"
#include "kopeteaccount.h"
#include "kopeteaway.h"
#include "kopeteaccountmanager.h"
#include "kopeteidentitystatusbaricon.h"
#include "kopetebehaviorsettings.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopetegroup.h"
#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"
#include "kopetelistviewsearchline.h"
#include "kopetechatsessionmanager.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteawayaction.h"
#include "kopeteuiglobal.h"
#include "systemtray.h"
#include "kopeteonlinestatusmanager.h"
#include "identitystatuswidget.h"

//BEGIN GlobalStatusMessageIconLabel
GlobalStatusMessageIconLabel::GlobalStatusMessageIconLabel(QWidget *parent)
 : QLabel(parent)
{
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

class KopeteWindow::Private
{
public:
	Private()
	 : contactlist(0), identitywidget(0), actionAddContact(0), actionDisconnect(0), actionExportContacts(0),
	actionStatusMenu(0), actionDockMenu(0), actionSetAway(0), actionSetBusy(0), actionSetAvailable(0),
	actionSetInvisible(0), actionPrefs(0), actionQuit(0), actionSave(0), menubarAction(0),
	statusbarAction(0), actionShowOfflineUsers(0), actionShowEmptyGroups(0), docked(0), 
	deskRight(0), statusBarWidget(0), tray(0), hidden(false), autoHide(false),
	autoHideTimeout(0), autoHideTimer(0), addContactMapper(0), 
	globalStatusMessage(0), globalStatusMessageMenu(0), newMessageEdit(0)
	{}

	~Private()
	{}

	KopeteContactListView *contactlist;

	IdentityStatusWidget *identitywidget;

	// Some Actions
	KActionMenu *actionAddContact;

	KAction *actionDisconnect;
	KAction *actionExportContacts;

	KActionMenu *actionStatusMenu;
	KActionMenu *actionDockMenu;
	KAction *actionSetAway;
	KAction *actionSetBusy;
	KAction *actionSetAvailable;
	KAction *actionSetInvisible;


	KAction *actionPrefs;
	KAction *actionQuit;
	KAction *actionSave;
	KToggleAction *menubarAction;
	KToggleAction *statusbarAction;
	KToggleAction *actionShowAllOfflineEmpty;
	KToggleAction *actionShowOfflineUsers;
	KToggleAction *actionShowEmptyGroups;

	int docked;
	int deskRight;
	QPoint position;
	KHBox *statusBarWidget;
	KopeteSystemTray *tray;
	bool hidden;
	bool autoHide;
	unsigned int autoHideTimeout;
	QTimer *autoHideTimer;
	QSignalMapper *addContactMapper;

	QHash<const Kopete::Identity*, KopeteIdentityStatusBarIcon*> identityStatusBarIcons;
	KSqueezedTextLabel *globalStatusMessage;
	KMenu *globalStatusMessageMenu;
	KLineEdit *newMessageEdit;
	QString globalStatusMessageStored;
};

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
 *                shutdown, the app is finally deref()ed, and the contact list and accountmanager
 *                are saved.
 *                and calling KApplication::quit()
 * 2) session   - KopeteWindow and all chatwindows are closed by KApplication session management.
 *     quit        Then the shutdown proceeds as above.
 *
 * queryClose() is honoured so groupchats and chats receiving recent messages can interrupt
 * (session) quit.
 */

KopeteWindow::KopeteWindow( QWidget *parent )
: KXmlGuiWindow( parent ), d(new Private)
{
	setAttribute (Qt::WA_DeleteOnClose, false);
	setAttribute (Qt::WA_QuitOnClose, false);
	// Applications should ensure that their StatusBar exists before calling createGUI()
	// so that the StatusBar is always correctly positioned when KDE is configured to use
	// a MacOS-style MenuBar.
	// This fixes a "statusbar drawn over the top of the toolbar" bug
	// e.g. it can happen when you switch desktops on Kopete startup
	d->statusBarWidget = new KHBox(statusBar());
	d->statusBarWidget->setMargin( 2 );
	d->statusBarWidget->setSpacing( 1 );
	statusBar()->addPermanentWidget(d->statusBarWidget, 0);
	QWidget *statusBarMessage = new QWidget( statusBar() );
	QHBoxLayout *statusBarMessageLayout = new QHBoxLayout( statusBarMessage );
	statusBarMessageLayout->setMargin( 2 );

	KStatusBarOfflineIndicator * indicator = new KStatusBarOfflineIndicator( this );
	statusBar()->addPermanentWidget( indicator, 0 );

	GlobalStatusMessageIconLabel *label = new GlobalStatusMessageIconLabel( statusBarMessage );
	label->setCursor( QCursor( Qt::PointingHandCursor ) );
	label->setFixedSize( 16, 16 );
	label->setPixmap( SmallIcon( "object-edit-status-message" ) );
	connect(label, SIGNAL(iconClicked( const QPoint& )),
		this, SLOT(slotGlobalStatusMessageIconClicked( const QPoint& )));
	label->setToolTip( i18n( "Global status message" ) );
	statusBarMessageLayout->addWidget( label );
	statusBarMessageLayout->addSpacing( 1 );
	d->globalStatusMessage = new KSqueezedTextLabel( statusBarMessage );
	statusBarMessageLayout->addWidget( d->globalStatusMessage );
	statusBar()->addWidget(statusBarMessage, 1);

	d->autoHideTimer = new QTimer( this );

	// --------------------------------------------------------------------------------
	initView();
	initActions();
	d->contactlist->initActions(actionCollection());
	initSystray();
	// --------------------------------------------------------------------------------

	// Trap all loaded plugins, so we can add their status bar icons accordingly , also used to add XMLGUIClient
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin * ) ),
		this, SLOT( slotPluginLoaded( Kopete::Plugin * ) ) );
	connect( Kopete::PluginManager::self(), SIGNAL( allPluginsLoaded() ),
		this, SLOT( slotAllPluginsLoaded() ));

	// Connect all identity signals
	connect( Kopete::IdentityManager::self(), SIGNAL(identityRegistered(Kopete::Identity*)),
		this, SLOT(slotIdentityRegistered(Kopete::Identity*)));
	connect( Kopete::IdentityManager::self(), SIGNAL(identityUnregistered(const Kopete::Identity*)),
		this, SLOT(slotIdentityUnregistered(const Kopete::Identity*)));
	
	//Connect the appropriate account signals
	/* Please note that I tried to put this in the slotAllPluginsLoaded() function
	 * but it seemed to break the account icons in the statusbar --Matt */
	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
		this, SLOT(slotAccountRegistered(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
		this, SLOT(slotAccountUnregistered(const Kopete::Account*)));

	connect( d->autoHideTimer, SIGNAL( timeout() ), this, SLOT( slotAutoHide() ) );
	connect( Kopete::AppearanceSettings::self(), SIGNAL( contactListAppearanceChanged() ),
		this, SLOT( slotContactListAppearanceChanged() ) );
	createGUI ( QLatin1String("kopeteui.rc") );

	// call this _after_ createGUI(), otherwise menubar is not set up correctly
	loadOptions();

	// If some plugins are already loaded, merge the GUI
	Kopete::PluginList plugins = Kopete::PluginManager::self()->loadedPlugins();
	foreach(Kopete::Plugin *plug, plugins)
		slotPluginLoaded( plug );

	// If some identity already registered, build the status icon
	Kopete::Identity::List identityList = Kopete::IdentityManager::self()->identities();
	foreach(Kopete::Identity *i, identityList)
		slotIdentityRegistered( i );

	// If some account already loaded, build the status icon
	QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
	foreach(Kopete::Account *a, accountList)
		slotAccountRegistered( a );

    //install an event filter for the quick search toolbar so we can
    //catch the hide events
    toolBar( "quickSearchBar" )->installEventFilter( this );
}

void KopeteWindow::initView()
{
	QWidget *w = new QWidget(this);
	QVBoxLayout *l = new QVBoxLayout(w);
	d->contactlist = new KopeteContactListView(w);
	l->addWidget(d->contactlist);
	l->setSpacing( 0 );
	l->setContentsMargins(0,0,0,0);
	d->identitywidget = new IdentityStatusWidget(0, w);
	d->identitywidget->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum) ); 
	d->identitywidget->setVisible( false );
	l->addWidget(d->identitywidget);

	setCentralWidget(w);
	d->contactlist->setFocus();
}

void KopeteWindow::initActions()
{
	// this action menu contains one action per account and is updated when accounts are registered/unregistered
	d->actionAddContact = new KActionMenu( KIcon("list-add-user"), i18n( "&Add Contact" ), this );
	d->actionAddContact->setIconText( i18n( "Add" ) );
	actionCollection()->addAction( "AddContact", d->actionAddContact );
	d->actionAddContact->setDelayed( false );
	// this signal mapper is needed to call slotAddContact with the correct arguments
	d->addContactMapper = new QSignalMapper( this );
	connect( d->addContactMapper, SIGNAL( mapped( const QString & ) ),
		 this, SLOT( slotAddContactDialogInternal( const QString & ) ) );

	d->actionDisconnect = new KAction( KIcon("connect-no"), i18n( "Offline" ), this );
        actionCollection()->addAction( "DisconnectAll", d->actionDisconnect );
	connect( d->actionDisconnect, SIGNAL( triggered(bool) ), this, SLOT( slotDisconnectAll() ) );
	d->actionDisconnect->setEnabled(false);

	d->actionExportContacts = new KAction( i18n( "&Export Contacts..." ), this );
	d->actionExportContacts->setIcon( KIcon( "document-export" ) );
        actionCollection()->addAction( "ExportContacts", d->actionExportContacts );
	connect( d->actionExportContacts, SIGNAL( triggered(bool) ), this, SLOT( showExportDialog() ) );

	d->actionSetAway = new KAction( KIcon("kopeteaway"), i18n("&Away"), this );
        actionCollection()->addAction( "SetAwayAll", d->actionSetAway );
	connect( d->actionSetAway, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalAway() ) );

	d->actionSetBusy = new KAction( KIcon("kopeteaway"), i18n("&Busy"), this );
        actionCollection()->addAction( "SetBusyAll", d->actionSetBusy );
	connect( d->actionSetBusy, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalBusy() ) );


	d->actionSetInvisible = new KAction( KIcon("kopeteavailable"), i18n( "&Invisible" ), this );
        actionCollection()->addAction( "SetInvisibleAll", d->actionSetInvisible );
	connect( d->actionSetInvisible, SIGNAL( triggered(bool) ), this, SLOT( slotSetInvisibleAll() ) );

	d->actionSetAvailable = new KAction( KIcon("kopeteavailable"), i18n("&Online"), this );
        actionCollection()->addAction( "SetAvailableAll", d->actionSetAvailable );
	connect( d->actionSetAvailable, SIGNAL( triggered(bool) ), this, SLOT( slotGlobalAvailable() ) );

	d->actionStatusMenu = new KActionMenu( KIcon("kopeteavailable"), i18n("&Set Status"),
                                             this );
	d->actionStatusMenu->setIconText( i18n( "Status" ) );
	actionCollection()->addAction( "Status", d->actionStatusMenu );
	d->actionStatusMenu->setDelayed( false );
	d->actionStatusMenu->addAction(d->actionSetAvailable);
	d->actionStatusMenu->addAction(d->actionSetAway);
	d->actionStatusMenu->addAction(d->actionSetBusy);
	d->actionStatusMenu->addAction(d->actionSetInvisible);
	d->actionStatusMenu->addAction(d->actionDisconnect);

	d->actionPrefs = KopeteStdAction::preferences( actionCollection(), "settings_prefs" );

	KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());

	setStandardToolBarMenuEnabled(true);
	d->menubarAction = KStandardAction::showMenubar(menuBar(), SLOT(setVisible(bool)), actionCollection() );
        actionCollection()->addAction( "settings_showmenubar", d->menubarAction );
	d->statusbarAction = KStandardAction::showStatusbar(statusBar(), SLOT(setVisible(bool)), actionCollection() );
        actionCollection()->addAction( "settings_showstatusbar", d->statusbarAction );

	KAction* act = KStandardAction::keyBindings( guiFactory(), SLOT( configureShortcuts() ), actionCollection() );
        actionCollection()->addAction( "settings_keys", act );

	KAction *configureGlobalShortcutsAction = new KAction( KIcon("configure-shortcuts"), i18n( "Configure &Global Shortcuts..." ), this );
        actionCollection()->addAction( "settings_global", configureGlobalShortcutsAction );
	connect( configureGlobalShortcutsAction, SIGNAL( triggered(bool) ), this, SLOT( slotConfGlobalKeys() ) );

	KStandardAction::configureToolbars( this, SLOT(slotConfToolbar()), actionCollection() );
	act = KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection() );
	actionCollection()->addAction( "settings_notifications", act );

	d->actionShowAllOfflineEmpty = new KToggleAction( KIcon("show-offliners"), i18n( "Show &All" ), this );
	actionCollection()->addAction( "settings_show_all_offline_empty", d->actionShowAllOfflineEmpty );
	d->actionShowAllOfflineEmpty->setShortcut( KShortcut(Qt::CTRL + Qt::Key_U) );
	connect( d->actionShowAllOfflineEmpty, SIGNAL( triggered(bool) ), this, SLOT( slotToggleShowAllOfflineEmpty(bool) ) );

	d->actionShowOfflineUsers = new KToggleAction( KIcon("show-offliners"), i18n( "Show Offline &Users" ), this );
        actionCollection()->addAction( "settings_show_offliners", d->actionShowOfflineUsers );
	connect( d->actionShowOfflineUsers, SIGNAL( triggered(bool) ), this, SLOT( slotToggleShowOfflineUsers() ) );

	d->actionShowEmptyGroups = new KToggleAction( KIcon("folder-grey"), i18n( "Show Empty &Groups" ), this );
        actionCollection()->addAction( "settings_show_empty_groups", d->actionShowEmptyGroups );
	d->actionShowEmptyGroups->setShortcut( KShortcut(Qt::CTRL + Qt::Key_G) );
	connect( d->actionShowEmptyGroups, SIGNAL( triggered(bool) ), this, SLOT( slotToggleShowEmptyGroups() ) );

	d->actionShowAllOfflineEmpty->setCheckedState( KGuiItem( i18n("Hide O&ffline") ) );
	d->actionShowOfflineUsers->setCheckedState( KGuiItem( i18n("Hide Offline &Users") ) );
	d->actionShowEmptyGroups->setCheckedState( KGuiItem( i18n("Hide Empty &Groups") ) );

	// quick search bar
	QLabel *searchLabel = new QLabel( i18n("Se&arch:"), 0 );
	searchLabel->setObjectName( QLatin1String("kde toolbar widget") );
	QWidget *searchBar = new Kopete::UI::ListView::SearchLine( 0, d->contactlist );
	searchLabel->setBuddy( searchBar );
	KAction *quickSearch = new KAction( i18n("Quick Search Bar"), this );
        actionCollection()->addAction( "quicksearch_bar", quickSearch );
	quickSearch->setDefaultWidget( searchBar );
	KAction *searchLabelAction = new KAction( i18n("Search:"), this );
        actionCollection()->addAction( "quicksearch_label", searchLabelAction );
	searchLabelAction->setDefaultWidget( searchLabel );

	// KActionMenu for selecting the global status message
	KActionMenu * setStatusMessageMenu = new KActionMenu( KIcon("kopeteeditstatusmessage"), i18n( "Set Status Message" ), this );
	setStatusMessageMenu->setIconText( i18n( "&Message" ) );
	actionCollection()->addAction( "SetStatusMessage", setStatusMessageMenu );
	setStatusMessageMenu->setDelayed( false );
	connect( setStatusMessageMenu->menu(), SIGNAL(aboutToShow()), SLOT(slotBuildStatusMessageMenu()) );
	connect( setStatusMessageMenu->menu(), SIGNAL(triggered(QAction*)), this, SLOT(slotStatusMessageSelected(QAction*)) );

	// sync actions, config and prefs-dialog
	connect ( Kopete::AppearanceSettings::self(), SIGNAL(configChanged()), this, SLOT(slotConfigChanged()) );
	slotConfigChanged();

	// Global actions
	KAction *globalReadMessage = new KAction( i18n("Read Message"), this );
        actionCollection()->addAction( "ReadMessage",  globalReadMessage );
	connect( globalReadMessage, SIGNAL( triggered(bool) ), Kopete::ChatSessionManager::self(), SLOT( slotReadMessage() ) );
	globalReadMessage->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I) );
	globalReadMessage->setWhatsThis( i18n("Read the next pending message") );

	KAction *globalShowContactList = new KAction( i18n("Show/Hide Contact List"), this );
        actionCollection()->addAction( "ShowContactList", globalShowContactList );
	connect( globalShowContactList, SIGNAL( triggered(bool) ), this, SLOT( slotShowHide() ) );
	globalShowContactList->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S) );
	globalShowContactList->setWhatsThis( i18n("Show or hide the contact list") );

	KAction *globalSetAway = new KAction( i18n("Set Away/Back"), this );
        actionCollection()->addAction( "Set_Away_Back",  globalSetAway );
	connect( globalSetAway, SIGNAL( triggered(bool) ), this, SLOT( slotToggleAway() ) );
	globalSetAway->setGlobalShortcut( KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_W) );
}

void KopeteWindow::slotShowHide()
{
	if(isActiveWindow())
	{
		d->autoHideTimer->stop(); //no timeouts if active
		hide();
	}
	else
	{
		show();
#ifdef Q_WS_X11
		//raise() and show() should normaly deIconify the window. but it doesn't do here due
		// to a bug in QT or in KDE  (qt3.1.x or KDE 3.1.x) then, i have to call KWin's method
		if(isMinimized())
			KWindowSystem::unminimizeWindow(winId());

		if(!KWindowSystem::windowInfo(winId(),NET::WMDesktop).onAllDesktops())
			KWindowSystem::setOnDesktop(winId(), KWindowSystem::currentDesktop());
#endif
		raise();
		activateWindow();
	}
}

void KopeteWindow::slotToggleAway()
{
	kDebug(14000);
	Kopete::Away *mAway = Kopete::Away::getInstance();
	if ( mAway->globalAway() )
	{
		Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Online );
	}
	else
	{
		slotGlobalAway();
	}
}

void KopeteWindow::initSystray()
{
	d->tray = KopeteSystemTray::systemTray( this );

	QObject::connect( d->tray, SIGNAL( aboutToShowMenu( KMenu * ) ),
	                  this, SLOT( slotTrayAboutToShowMenu( KMenu * ) ) );
	QObject::connect( d->tray, SIGNAL( quitSelected() ), this, SLOT( slotQuit() ) );
}

KopeteWindow::~KopeteWindow()
{
	delete d;
}

bool KopeteWindow::eventFilter( QObject* target, QEvent* event )
{
    KToolBar *toolBar = dynamic_cast<KToolBar*>( target );
    QAction *resetAction = actionCollection()->action( "quicksearch_reset" );

    if ( toolBar && resetAction && resetAction->associatedWidgets().contains( toolBar ) )
    {

        if ( event->type() == QEvent::Hide )
        {
            resetAction->trigger();
            return true;
        }
        return KXmlGuiWindow::eventFilter( target, event );
    }

    return KXmlGuiWindow::eventFilter( target, event );
}

void KopeteWindow::loadOptions()
{
	KSharedConfig::Ptr config = KGlobal::config();

	toolBar("mainToolBar")->applySettings( config->group( "ToolBar Settings" ) );
	toolBar("quickSearchBar")->applySettings( config->group( "QuickSearchBar Settings" ) );

	applyMainWindowSettings( config->group( "General Options" ) );
        KConfigGroup cg( config, "General Options");
	QPoint pos = cg.readEntry("Position", QPoint());
	move(pos);

	QSize size = cg.readEntry("Geometry", QSize() );
	if(size.isEmpty()) // Default size
		resize( QSize(272, 400) );
	else
		resize(size);

	d->autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	d->autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();


	QString tmp = cg.readEntry("State", "Shown");
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

	d->menubarAction->setChecked( !menuBar()->isHidden() );
	d->statusbarAction->setChecked( !statusBar()->isHidden() );
}

void KopeteWindow::saveOptions()
{
    KConfigGroup mainToolbarGroup( KGlobal::config(), "ToolBar Settings" );
    toolBar("mainToolBar")->saveSettings( mainToolbarGroup );
    KConfigGroup qsbGroup(KGlobal::config(), "QuickSearchBar Settings" );
    toolBar("quickSearchBar")->saveSettings( qsbGroup );

    KConfigGroup cg( KGlobal::config(), "General Options" );
	saveMainWindowSettings( cg );

	cg.writeEntry("Position", pos());
	cg.writeEntry("Geometry", size());

	if(isMinimized())
	{
		cg.writeEntry("State", "Minimized");
	}
	else if(isHidden())
	{
		cg.writeEntry("State", "Hidden");
	}
	else
	{
		cg.writeEntry("State", "Shown");
	}

	cg.sync();
}

void KopeteWindow::slotToggleShowAllOfflineEmpty( bool toggled)
{
	d->actionShowOfflineUsers->setChecked( toggled );
	d->actionShowEmptyGroups->setChecked( toggled );
	Kopete::AppearanceSettings::self()->setShowOfflineUsers( toggled );
	Kopete::AppearanceSettings::self()->setShowEmptyGroups( toggled );
	Kopete::AppearanceSettings::self()->writeConfig();
}

void KopeteWindow::slotToggleShowOfflineUsers()
{
	Kopete::AppearanceSettings::self()->setShowOfflineUsers ( d->actionShowOfflineUsers->isChecked() );
	Kopete::AppearanceSettings::self()->writeConfig();
}

void KopeteWindow::slotToggleShowEmptyGroups()
{
	Kopete::AppearanceSettings::self()->setShowEmptyGroups ( d->actionShowEmptyGroups->isChecked() );
	Kopete::AppearanceSettings::self()->writeConfig();
}

void KopeteWindow::slotConfigChanged()
{
	if( isHidden() && !Kopete::BehaviorSettings::self()->showSystemTray()) // user disabled systray while kopete is hidden, show it!
		show();

	d->actionShowAllOfflineEmpty->setChecked( Kopete::AppearanceSettings::self()->showOfflineUsers() && Kopete::AppearanceSettings::self()->showEmptyGroups() );
	d->actionShowOfflineUsers->setChecked( Kopete::AppearanceSettings::self()->showOfflineUsers() );
	d->actionShowEmptyGroups->setChecked( Kopete::AppearanceSettings::self()->showEmptyGroups() );
}

void KopeteWindow::slotContactListAppearanceChanged()
{
	d->autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	d->autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();

	startAutoHideTimer();
}

void KopeteWindow::slotConfNotifications()
{
	KNotifyConfigWidget::configure( this );
}

void KopeteWindow::slotConfGlobalKeys()
{
	KShortcutsDialog::configure( actionCollection() );
}

void KopeteWindow::slotConfToolbar()
{
        KConfigGroup cg( KGlobal::config(), "General Options");
	saveMainWindowSettings( cg );
	KEditToolBar *dlg = new KEditToolBar(factory());
	connect( dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotUpdateToolbar()) );
	connect( dlg, SIGNAL(finished()) , dlg, SLOT(deleteLater()));
	dlg->show();
}

void KopeteWindow::slotUpdateToolbar()
{
	applyMainWindowSettings(KGlobal::config()->group( "General Options") );
}

void KopeteWindow::slotGlobalAway()
{
	Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Away, d->globalStatusMessageStored );
}

void KopeteWindow::slotGlobalBusy()
{
	Kopete::AccountManager::self()->setOnlineStatus(
			Kopete::OnlineStatusManager::Busy, d->globalStatusMessageStored );
}

void KopeteWindow::slotGlobalAvailable()
{
	Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Online, d->globalStatusMessageStored );
}

void KopeteWindow::slotSetInvisibleAll()
{
	Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Invisible  );
}

void KopeteWindow::slotDisconnectAll()
{
	d->globalStatusMessage->setText( "" );
	d->globalStatusMessageStored = QString();
	Kopete::AccountManager::self()->setOnlineStatus( Kopete::OnlineStatusManager::Offline, d->globalStatusMessageStored );
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
		saveOptions();
		kDebug( 14000 ) << " shutting down plugin manager";
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
		kDebug( 14000 ) << "just closing because we have a system tray icon";
	}
	else
	{
		kDebug( 14000 ) << "delegating to KXmlGuiWindow::closeEvent()";
		KXmlGuiWindow::closeEvent( e );
	}
}

void KopeteWindow::slotQuit()
{
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
	d->actionDisconnect->setEnabled(true);
}

void KopeteWindow::slotIdentityRegistered( Kopete::Identity *identity )
{
	if ( !identity )
		return;

	connect( identity, SIGNAL(onlineStatusChanged(Kopete::Identity *)),
			this, SLOT( slotIdentityStatusIconChanged(Kopete::Identity *)));
	connect( identity, SIGNAL(identityChanged(Kopete::Identity *)),
			this, SLOT( slotIdentityStatusIconChanged( Kopete::Identity *)));

	KopeteIdentityStatusBarIcon *sbIcon = new KopeteIdentityStatusBarIcon( identity, d->statusBarWidget );
	connect( sbIcon, SIGNAL( leftClicked( Kopete::Identity *, const QPoint & ) ),
					 SLOT( slotIdentityStatusIconLeftClicked( Kopete::Identity *, const QPoint & ) ) );

	d->identityStatusBarIcons.insert( identity, sbIcon );
	slotIdentityStatusIconChanged( identity );
}

void KopeteWindow::slotIdentityUnregistered( const Kopete::Identity *identity)
{
	kDebug(14000) ;
	
	KopeteIdentityStatusBarIcon *sbIcon = d->identityStatusBarIcons[identity];

	if( !sbIcon )
		return;

	d->identityStatusBarIcons.remove( identity );
	delete sbIcon;

	makeTrayToolTip();

}

void KopeteWindow::slotIdentityStatusIconChanged( Kopete::Identity *identity )
{
	kDebug( 14000 ) << identity->property( Kopete::Global::Properties::self()->statusMessage() ).value();
	// update the global status label if the change doesn't
//	QString newAwayMessage = contact->property( Kopete::Global::Properties::self()->awayMessage() ).value().toString();
// 	if ( status.status() != Kopete::OnlineStatus::Connecting )
// 	{
// 		QString globalMessage = m_globalStatusMessage->text();
// 		if ( newAwayMessage != globalMessage )
// 			m_globalStatusMessage->setText( ""i18n("status message to show when different accounts have different status messages", "(multiple)" );
// 	}
//	kDebug(14000) << "Icons: '" <<
//		status.overlayIcons() << "'" << endl;

	if ( identity->onlineStatus() != Kopete::OnlineStatus::Connecting )
	{
		d->globalStatusMessageStored = identity->property( Kopete::Global::Properties::self()->statusMessage() ).value().toString();
		d->globalStatusMessage->setText( d->globalStatusMessageStored );
	}

	KopeteIdentityStatusBarIcon *i = d->identityStatusBarIcons[ identity ];
	if( !i )
		return;

	// Adds tooltip for each status icon,
	// useful in case you have many accounts
	// over one protocol
	i->setToolTip( identity->toolTip() );

	// FIXME: should add the status to the icon
	QPixmap pm;
	switch ( identity->onlineStatus() ) {
		case Kopete::OnlineStatus::Offline:
		case Kopete::OnlineStatus::Connecting:
			pm = SmallIcon( "user-offline" );
			break;
		case Kopete::OnlineStatus::Invisible:
		case Kopete::OnlineStatus::Away:
			pm = SmallIcon( "user-away" );
			break;
		case Kopete::OnlineStatus::Online:
			pm = SmallIcon( "user-online" );
			break;
		case Kopete::OnlineStatus::Unknown:
			pm = SmallIcon( "user" );
			break;
	}

	// No Pixmap found, fallback to Unknown
	if( pm.isNull() )
		i->setPixmap( SmallIcon( "user" ) );
	else
		i->setPixmap( pm );
	makeTrayToolTip();
}

void KopeteWindow::makeTrayToolTip()
{
	//FIXME: maybe use identities here?
	//the tool-tip of the systemtray.
	if(d->tray)
	{
		QString tt = QLatin1String("<qt>");
		QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
		foreach(Kopete::Account *a, accountList)
		{
			Kopete::Contact *self = a->myself();
			tt += i18nc( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)</nobr><br />",
			             "<nobr><img src=\"kopete-account-icon:%3:%4\" /> <b>%1:</b> %2 (<i>%5</i>)</nobr><br />",
				     a->protocol()->displayName(), a->accountLabel(), QString(QUrl::toPercentEncoding( a->protocol()->pluginId() )),
				     QString(QUrl::toPercentEncoding( a->accountId() )), self->onlineStatus().description() );
		}
		tt += QLatin1String("</qt>");
		d->tray->setToolTip(tt);
	}
}

void KopeteWindow::slotIdentityStatusIconLeftClicked( Kopete::Identity *identity, const QPoint &p )
{
	Q_UNUSED( p )
	if (d->identitywidget->isVisible() && d->identitywidget->identity() == identity)
	{
		d->identitywidget->setIdentity(0);
		d->identitywidget->setVisible(false);
		return;
	}

	d->identitywidget->setIdentity(identity);
	d->identitywidget->setVisible(true);
}


void KopeteWindow::slotAccountRegistered( Kopete::Account *account )
{

	//enable the connect all toolbar button
//	actionConnect->setEnabled(true);
	d->actionDisconnect->setEnabled(true);

	// add an item for this account to the add contact actionmenu
	QString s = QString("actionAdd%1Contact").arg( account->accountId() );
	KAction *action = new KAction( KIcon(account->accountIcon()), account->accountLabel(), this );
        actionCollection()->addAction( s, action );
	connect( action, SIGNAL(triggered(bool)), d->addContactMapper, SLOT(map()) );

	d->addContactMapper->setMapping( action, account->protocol()->pluginId() + QChar(0xE000) + account->accountId() );
	d->actionAddContact->addAction( action );

}

void KopeteWindow::slotAccountUnregistered( const Kopete::Account *account )
{
	QList<Kopete::Account *> accounts = Kopete::AccountManager::self()->accounts();
	if (accounts.isEmpty())
	{
//		actionConnect->setEnabled(false);
		d->actionDisconnect->setEnabled(false);
	}

	// update add contact actionmenu
	QString s = QString("actionAdd%1Contact").arg( account->accountId() );
	QAction *action = actionCollection()->action( s );
	if ( action )
	{
		kDebug(14000) << " found KAction " << action << " with name: " << action->objectName();
		d->addContactMapper->removeMappings( action );
		d->actionAddContact->removeAction( action );
	}
}

void KopeteWindow::slotTrayAboutToShowMenu( KMenu * popup )
{
	KActionCollection *actionCollection = d->tray->actionCollection();

	popup->clear();
	popup->addTitle( qApp->windowIcon(), KGlobal::caption() );

	QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
	foreach(Kopete::Account *a, accountList)
	{
		KActionMenu *menu = a->actionMenu();
		if( menu )
			popup->addAction( menu );

		connect(popup , SIGNAL(aboutToHide()) , menu , SLOT(deleteLater()));
	}

	popup->addSeparator();
	popup->addAction( d->actionStatusMenu );
	popup->addSeparator();
	popup->addAction( d->actionPrefs );
	popup->addAction( d->actionAddContact );
	popup->addSeparator();
	popup->addAction( actionCollection->action( "minimizeRestore" ) );
	popup->addAction( actionCollection->action( KStandardAction::name( KStandardAction::Quit ) ) );
}

void KopeteWindow::showExportDialog()
{
	KabcExportWizard* wizard = new KabcExportWizard( this );
	wizard->setObjectName( QLatin1String("export_contact_dialog") );
	wizard->show();
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
		d->autoHideTimer->stop();
		hide();
	}
}

void KopeteWindow::startAutoHideTimer()
{
	if ( d->autoHideTimeout > 0 && d->autoHide == true && isVisible() && Kopete::BehaviorSettings::self()->showSystemTray())
		d->autoHideTimer->start( d->autoHideTimeout * 1000 );
}

// Iterate each connected account, updating its status message bug keeping the
// same onlinestatus.  Then update Kopete::Away and the UI.
void KopeteWindow::setStatusMessage( const QString & message )
{
	bool changed = false;
	QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
	foreach(Kopete::Account *account, accountList)
	{
		Kopete::Contact *self = account->myself();
		bool isInvisible = self && self->onlineStatus().status() == Kopete::OnlineStatus::Invisible;
		if ( account->isConnected() && !isInvisible )
		{
			changed = true;
			account->setOnlineStatus( self->onlineStatus(), message );
		}
	}
	Kopete::Away::getInstance()->setGlobalAwayMessage( message );
	d->globalStatusMessageStored = message;
	d->globalStatusMessage->setText( message );
}

void KopeteWindow::slotBuildStatusMessageMenu()
{
	//There are two ways for the menu to be avtivated:
	//Via the menu or the 'Global Status Message' button
	//Find out which menu asked us to be built
	QObject * senderObj = const_cast<QObject *>( sender() );
	d->globalStatusMessageMenu = static_cast<KMenu *>( senderObj );
	d->globalStatusMessageMenu->clear();

	d->globalStatusMessageMenu->addTitle( i18n("Status Message") );
	//BEGIN: Add new message widget to the Set Status Message Menu.
	QWidget * newMessageBox = new QWidget( this );
	QHBoxLayout * newMessageBoxLayout = new QHBoxLayout( newMessageBox );
	newMessageBoxLayout->setMargin( 1 );
	newMessageBoxLayout->addSpacing( 2 );
	QLabel * newMessagePix = new QLabel( newMessageBox );
	newMessagePix->setPixmap( SmallIcon( "object-edit" ) );
	newMessageBoxLayout->addWidget( newMessagePix );
	newMessageBoxLayout->addSpacing( 3 );
	QLabel * newMessageLabel = new QLabel( i18n( "Add" ), newMessageBox );
	newMessageBoxLayout->addWidget( newMessageLabel );
	d->newMessageEdit = new KLineEdit( newMessageBox );
	d->newMessageEdit->setClearButtonShown( true );
	newMessageBoxLayout->addWidget( d->newMessageEdit );
	newMessageBox->setFocusProxy( d->newMessageEdit );
	newMessageBox->setFocusPolicy( Qt::ClickFocus );
	newMessageLabel->setFocusProxy( d->newMessageEdit );
	newMessageLabel->setBuddy( d->newMessageEdit );
	newMessageLabel->setFocusPolicy( Qt::ClickFocus );
	newMessagePix->setFocusProxy( d->newMessageEdit );
	newMessagePix->setFocusPolicy( Qt::ClickFocus );
	connect( d->newMessageEdit, SIGNAL( returnPressed() ), SLOT( slotNewStatusMessageEntered() ) );

	KAction *newMessageAction = new KAction( KIcon("object-edit"), i18n("New Message..."), d->globalStatusMessageMenu );
	newMessageAction->setDefaultWidget( newMessageBox );

	d->globalStatusMessageMenu->addAction( newMessageAction );
	//END

	d->globalStatusMessageMenu->addAction( SmallIcon( "list-remove" ), i18n( "No Message" ) );
	d->globalStatusMessageMenu->addSeparator();

	QStringList awayMessages = Kopete::Away::getInstance()->getMessages();
	for ( int i = 0; i < awayMessages.count(); ++i )
	{
		QAction *action = new QAction( d->globalStatusMessageMenu );
		action->setText( KStringHandler::rsqueeze( awayMessages[i] ) );
		action->setData( i );
		d->globalStatusMessageMenu->addAction( action );
	}

	d->newMessageEdit->setFocus(Qt::OtherFocusReason);
	d->globalStatusMessageMenu->setActiveAction(newMessageAction);
}

void KopeteWindow::slotStatusMessageSelected( QAction *action )
{
	if ( !action->data().isValid() )
	{
		setStatusMessage( "" );
		return;
	}

	int i = action->data().toInt();
	setStatusMessage(  Kopete::Away::getInstance()->getMessage( i ) );
}

void KopeteWindow::slotNewStatusMessageEntered()
{
	d->globalStatusMessageMenu->close();
	QString newMessage = d->newMessageEdit->text();
	if ( !newMessage.isEmpty() )
		Kopete::Away::getInstance()->addMessage( newMessage );
	setStatusMessage( d->newMessageEdit->text() );
}

void KopeteWindow::slotGlobalStatusMessageIconClicked( const QPoint &position )
{
	KMenu *statusMessageIconMenu = new KMenu(this);

	connect( statusMessageIconMenu, SIGNAL(aboutToShow()),
	         this, SLOT(slotBuildStatusMessageMenu()) );
	connect( statusMessageIconMenu, SIGNAL(triggered(QAction*)),
	         this, SLOT(slotStatusMessageSelected(QAction*)) );
	connect( statusMessageIconMenu, SIGNAL(aboutToHide()),
	         statusMessageIconMenu, SLOT(deleteLater()) );

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
		kDebug( 14000 ) << "no account given";
		return;
	}

	KDialog *addDialog = new KDialog( this );
	addDialog->setCaption( i18n( "Add Contact" ) );
	addDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	addDialog->setDefaultButton( KDialog::Ok );
	addDialog->showButtonSeparator( true );

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
			ui_groupKABC.groupCombo->addItem( groupname );
		}
	}

	if (!addContactPage)
	{
		kDebug(14000) <<
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
