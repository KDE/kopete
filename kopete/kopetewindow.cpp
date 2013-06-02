/*
    kopetewindow.cpp  -  Kopete Main Window

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2009 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005-2006 by Will Stephenson        <wstephenson@kde.org>
    Copyright (c) 2008      by Roman Jarosz           <kedgedev@centrum.cz>

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

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
#include <QTextEdit>

#include <khbox.h>
#include <kvbox.h>
#include <kaction.h>
#include <kactionmenu.h>
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
#include <kfilterproxysearchline.h>

#include "addcontactpage.h"
#include "addressbooklinkwidget.h"
#include "ui_groupkabcselectorwidget.h"
#include "kabcexport.h"
#include "kopeteappearancesettings.h"
#include "kopeteapplication.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccountstatusbaricon.h"
#include "kopeteidentitystatusbaricon.h"
#include "kopetebehaviorsettings.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteidentity.h"
#include "kopeteidentitymanager.h"
#include "kopetelistviewsearchline.h"
#include "kopetechatsessionmanager.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"
#include "systemtray.h"
#include "kopeteonlinestatusmanager.h"
#include "identitystatuswidget.h"
#include "kopetestatusmanager.h"
#include "kopetestatusrootaction.h"
#include "kopetestatuseditaction.h"
#include "kopeteemoticons.h"
#include "kopeteinfoeventmanager.h"
#include "infoeventwidget.h"
#include "contactlisttreemodel.h"
#include "contactlistplainmodel.h"
#include "contactlistproxymodel.h"
#include "kopeteitemdelegate.h"
#include "kopetemetacontact.h"
#include "kopetecontactlistview.h"
#include "kopetestatusitems.h"


//BEGIN GlobalStatusMessageIconLabel
GlobalStatusMessageIconLabel::GlobalStatusMessageIconLabel ( QWidget *parent )
		: QLabel ( parent )
{
	setCursor ( QCursor ( Qt::PointingHandCursor ) );
	setFixedSize ( 16, 16 );
	setPixmap ( SmallIcon ( "im-status-message-edit" ) );
	setToolTip ( i18n ( "Global status message" ) );
}

void GlobalStatusMessageIconLabel::mouseReleaseEvent ( QMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton || event->button() == Qt::RightButton )
	{
		emit iconClicked ( event->globalPos() );
		event->accept();
	}
}
//END GlobalStatusMessageIconLabel

//BEGIN InfoEventIconLabel
InfoEventIconLabel::InfoEventIconLabel( QWidget *parent )
: QLabel( parent )
{
	setCursor( QCursor( Qt::PointingHandCursor ) );
	setFixedSize( 16, 16 );
	setPixmap( SmallIcon( "flag-black" ) );
	setToolTip( i18n( "Service messages" ) );

	connect( Kopete::InfoEventManager::self(), SIGNAL(changed()), this, SLOT(updateIcon()) );
}

void InfoEventIconLabel::mouseReleaseEvent( QMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton || event->button() == Qt::RightButton )
	{
		emit clicked();
		event->accept();
	}
}

void InfoEventIconLabel::updateIcon()
{
	if ( Kopete::InfoEventManager::self()->eventCount() > 0 )
		setPixmap( SmallIcon( "flag-green" ) );
	else {
		setPixmap( SmallIcon( "flag-black" ) );
		emit clicked();
	}
}
//END InfoEventIconLabel

class KopeteWindow::Private
{
	public:
		Private()
				: contactlist ( 0 ), model(0), proxyModel(0), identitywidget ( 0 ), infoEventWidget ( 0 ), actionAddContact ( 0 ), actionDisconnect ( 0 ),
				actionExportContacts ( 0 ), actionStatusMenu ( 0 ), actionDockMenu ( 0 ), actionSetAway ( 0 ),
				actionSetBusy ( 0 ), actionSetAvailable ( 0 ), actionSetInvisible ( 0 ), actionPrefs ( 0 ),
				actionQuit ( 0 ), actionSave ( 0 ), menubarAction ( 0 ), statusbarAction ( 0 ),
				actionShowOfflineUsers ( 0 ), actionShowEmptyGroups ( 0 ), docked ( 0 ), deskRight ( 0 ),
				statusBarWidget ( 0 ), tray ( 0 ), hidden ( false ), autoHide ( false ),
				autoHideTimeout ( 0 ), autoHideTimer ( 0 ), addContactMapper ( 0 ),
				showIdentityIcons( Kopete::AppearanceSettings::self()->showIdentityIcons() ),
				globalStatusMessage ( 0 )
		{}

		~Private()
		{}

		KopeteContactListView *contactlist;
		Kopete::UI::ContactListModel* model;
		Kopete::UI::ContactListProxyModel* proxyModel;

		IdentityStatusWidget *identitywidget;
		InfoEventWidget *infoEventWidget;

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
		bool appDestroyed;
		bool hidden;
		bool autoHide;
		unsigned int autoHideTimeout;
		QTimer *autoHideTimer;
		QTimer *autoResizeTimer;
		QSignalMapper *addContactMapper;

		bool showIdentityIcons;
		QHash<const Kopete::Identity*, KopeteIdentityStatusBarIcon*> identityStatusBarIcons;
		QHash<const Kopete::Account*, KopeteAccountStatusBarIcon*> accountStatusBarIcons;
		KSqueezedTextLabel *globalStatusMessage;
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

KopeteWindow::KopeteWindow ( QWidget *parent )
		: KXmlGuiWindow ( parent ), d ( new Private )
{
	d->appDestroyed = false;
	connect ( kapp, SIGNAL (destroyed()), this, SLOT (slotAppDestroyed()) );
	setAttribute ( Qt::WA_DeleteOnClose, false );
	setAttribute ( Qt::WA_QuitOnClose, false );
	// Applications should ensure that their StatusBar exists before calling createGUI()
	// so that the StatusBar is always correctly positioned when KDE is configured to use
	// a MacOS-style MenuBar.
	// This fixes a "statusbar drawn over the top of the toolbar" bug
	// e.g. it can happen when you switch desktops on Kopete startup
	d->statusBarWidget = new KHBox ( statusBar() );
	d->statusBarWidget->setMargin ( 2 );
	d->statusBarWidget->setSpacing ( 1 );
	window()->setAttribute( Qt::WA_AlwaysShowToolTips );
	statusBar()->addPermanentWidget ( d->statusBarWidget, 0 );
	QWidget *statusBarMessage = new QWidget ( statusBar() );
	QHBoxLayout *statusBarMessageLayout = new QHBoxLayout ( statusBarMessage );
	statusBarMessageLayout->setMargin ( 2 );

	KStatusBarOfflineIndicator * indicator = new KStatusBarOfflineIndicator ( this );
	statusBar()->addPermanentWidget ( indicator, 0 );

	GlobalStatusMessageIconLabel *label = new GlobalStatusMessageIconLabel ( statusBarMessage );
	connect ( label, SIGNAL (iconClicked(QPoint)),
	          this, SLOT (slotGlobalStatusMessageIconClicked(QPoint)) );
	statusBarMessageLayout->addWidget ( label );
	statusBarMessageLayout->addSpacing ( 1 );

	InfoEventIconLabel *infoLabel = new InfoEventIconLabel ( statusBarMessage );
	connect ( infoLabel, SIGNAL(clicked()), this, SLOT(slotInfoIconClicked()) );
	statusBarMessageLayout->addWidget ( infoLabel );
	statusBarMessageLayout->addSpacing ( 1 );
	connect( Kopete::InfoEventManager::self(), SIGNAL(eventAdded(Kopete::InfoEvent*)), this, SLOT(slotNewInfoEvent()) );

	d->globalStatusMessage = new KSqueezedTextLabel ( statusBarMessage );
	connect ( Kopete::StatusManager::self(), SIGNAL (globalStatusChanged()),
	          this, SLOT (globalStatusChanged()) );
	statusBarMessageLayout->addWidget ( d->globalStatusMessage );
	statusBar()->addWidget ( statusBarMessage, 1 );

	d->autoHideTimer = new QTimer ( this );
	d->autoResizeTimer = new QTimer ( this );
	d->autoResizeTimer->setSingleShot ( true );

	// --------------------------------------------------------------------------------
	initView();
	initActions();
	d->contactlist->initActions ( actionCollection() );
	initSystray();
	// --------------------------------------------------------------------------------

	// Trap all loaded plugins, so we can add their status bar icons accordingly , also used to add XMLGUIClient
	connect ( Kopete::PluginManager::self(), SIGNAL (pluginLoaded(Kopete::Plugin*)),
	          this, SLOT (slotPluginLoaded(Kopete::Plugin*)) );
	connect ( Kopete::PluginManager::self(), SIGNAL (allPluginsLoaded()),
	          this, SLOT (slotAllPluginsLoaded()) );

	// Connect all identity signals
	connect ( Kopete::IdentityManager::self(), SIGNAL (identityRegistered(Kopete::Identity*)),
	          this, SLOT (slotIdentityRegistered(Kopete::Identity*)) );
	connect ( Kopete::IdentityManager::self(), SIGNAL (identityUnregistered(const Kopete::Identity*)),
	          this, SLOT (slotIdentityUnregistered(const Kopete::Identity*)) );

	connect ( d->autoHideTimer, SIGNAL (timeout()), this, SLOT (slotAutoHide()) );
	connect ( d->contactlist, SIGNAL(visibleContentHeightChanged()), this, SLOT (slotStartAutoResizeTimer()) );
	connect ( d->autoResizeTimer, SIGNAL (timeout()), this, SLOT (slotUpdateSize()) );
	connect ( Kopete::AppearanceSettings::self(), SIGNAL (contactListAppearanceChanged()),
	          this, SLOT (slotContactListAppearanceChanged()) );
	createGUI ( QLatin1String ( "kopeteui.rc" ) );

	// call this _after_ createGUI(), otherwise menubar is not set up correctly
	loadOptions();

	// If some plugins are already loaded, merge the GUI
	Kopete::PluginList plugins = Kopete::PluginManager::self()->loadedPlugins();
	foreach ( Kopete::Plugin *plug, plugins )
	slotPluginLoaded ( plug );

	// If some identity already registered, build the status icon
	Kopete::Identity::List identityList = Kopete::IdentityManager::self()->identities();
	foreach ( Kopete::Identity *i, identityList )
	slotIdentityRegistered ( i );

	//install an event filter for the quick search toolbar so we can
	//catch the hide events
	toolBar ( "quickSearchBar" )->installEventFilter ( this );
}

void KopeteWindow::slotAppDestroyed()
{
	d->appDestroyed = true;
}

void KopeteWindow::initView()
{
	QWidget *w = new QWidget ( this );
	QVBoxLayout *l = new QVBoxLayout ( w );
 	d->contactlist = new KopeteContactListView ( w );

	if ( Kopete::AppearanceSettings::self()->groupContactByGroup() )
		d->model = new Kopete::UI::ContactListTreeModel( this );
	else
		d->model = new Kopete::UI::ContactListPlainModel( this );

	d->model->init();
	d->proxyModel = new Kopete::UI::ContactListProxyModel( this );
	d->proxyModel->setSourceModel( d->model );
	d->contactlist->setModel( d->proxyModel );
	l->addWidget ( d->contactlist );
	l->setSpacing ( 0 );
	l->setContentsMargins ( 0,0,0,0 );
	d->identitywidget = new IdentityStatusWidget ( 0, w );
	d->identitywidget->setSizePolicy ( QSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Minimum ) );
	d->identitywidget->setVisible ( false );
	l->addWidget ( d->identitywidget );
	d->infoEventWidget = new InfoEventWidget ( w );
	d->infoEventWidget->setSizePolicy ( QSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Minimum ) );
	d->infoEventWidget->setVisible ( false );
	connect ( d->infoEventWidget, SIGNAL(showRequest()), this, SLOT(slotShowInfoEventWidget()) );
	l->addWidget ( d->infoEventWidget );

	setCentralWidget ( w );
	d->contactlist->setFocus();
}

void KopeteWindow::initActions()
{
	// this action menu contains one action per account and is updated when accounts are registered/unregistered
	d->actionAddContact = new KActionMenu ( KIcon ( "list-add-user" ), i18n ( "&Add Contact" ), this );
	d->actionAddContact->setIconText ( i18n ( "Add" ) );
	actionCollection()->addAction ( "AddContact", d->actionAddContact );
	d->actionAddContact->setDelayed ( false );
	// this signal mapper is needed to call slotAddContact with the correct arguments
	d->addContactMapper = new QSignalMapper ( this );
	connect ( d->addContactMapper, SIGNAL (mapped(QString)),
	          this, SLOT (slotAddContactDialogInternal(QString)) );

	d->actionDisconnect = new KAction ( KIcon ( "user-offline" ), i18n ( "Offline" ), this );
	actionCollection()->addAction ( "DisconnectAll", d->actionDisconnect );
	connect ( d->actionDisconnect, SIGNAL (triggered(bool)), this, SLOT (slotDisconnectAll()) );
	d->actionDisconnect->setEnabled ( false );

	d->actionExportContacts = new KAction ( i18n ( "&Export Contacts..." ), this );
	d->actionExportContacts->setIcon ( KIcon ( "document-export" ) );
	actionCollection()->addAction ( "ExportContacts", d->actionExportContacts );
	connect ( d->actionExportContacts, SIGNAL (triggered(bool)), this, SLOT (showExportDialog()) );

	d->actionSetAway = new KAction ( KIcon ( "user-identity", 0, QStringList() << QString() << "user-away" ), i18n ( "&Away" ), this );
	actionCollection()->addAction ( "SetAwayAll", d->actionSetAway );
	connect ( d->actionSetAway, SIGNAL (triggered(bool)), this, SLOT (slotGlobalAway()) );

	d->actionSetBusy = new KAction ( KIcon ( "user-identity", 0, QStringList() << QString() << "user-busy" ), i18n ( "&Busy" ), this );
	actionCollection()->addAction ( "SetBusyAll", d->actionSetBusy );
	connect ( d->actionSetBusy, SIGNAL (triggered(bool)), this, SLOT (slotGlobalBusy()) );


	d->actionSetInvisible = new KAction ( KIcon ( "user-identity", 0, QStringList() << QString() << "user-invisible" ), i18n ( "&Invisible" ), this );
	actionCollection()->addAction ( "SetInvisibleAll", d->actionSetInvisible );
	connect ( d->actionSetInvisible, SIGNAL (triggered(bool)), this, SLOT (slotSetInvisibleAll()) );

	d->actionSetAvailable = new KAction ( KIcon ( "user-identity", 0, QStringList() << QString() << "user-online" ), i18n ( "&Online" ), this );
	actionCollection()->addAction ( "SetAvailableAll", d->actionSetAvailable );
	connect ( d->actionSetAvailable, SIGNAL (triggered(bool)), this, SLOT (slotGlobalAvailable()) );

	d->actionStatusMenu = new KActionMenu ( KIcon ( "user-identity", 0, QStringList() << QString() << "user-online" ), i18n ( "&Set Status" ), this );
	d->actionStatusMenu->setIconText ( i18n ( "Status" ) );
	actionCollection()->addAction ( "Status", d->actionStatusMenu );
	d->actionStatusMenu->setDelayed ( false );

	// Will be automatically deleted when the actionStatusMenu is deleted.
	Kopete::StatusRootAction* statusAction = new Kopete::StatusRootAction ( d->actionStatusMenu );

	connect ( statusAction, SIGNAL (changeStatus(uint,Kopete::StatusMessage)),
	          this, SLOT (setOnlineStatus(uint,Kopete::StatusMessage)) );
	connect ( statusAction, SIGNAL (updateMessage(Kopete::StatusRootAction*)),
	          this, SLOT (updateStatusMenuMessage(Kopete::StatusRootAction*)) );
	connect ( statusAction, SIGNAL (changeMessage(Kopete::StatusMessage)),
	          this, SLOT (setStatusMessage(Kopete::StatusMessage)) );

	d->actionPrefs = KopeteStdAction::preferences ( actionCollection(), "settings_prefs" );

	KStandardAction::quit ( this, SLOT (slotQuit()), actionCollection() );

	setStandardToolBarMenuEnabled ( true );
	d->menubarAction = KStandardAction::showMenubar ( menuBar(), SLOT (setVisible(bool)), actionCollection() );
	actionCollection()->addAction ( "settings_showmenubar", d->menubarAction );
	d->statusbarAction = KStandardAction::showStatusbar ( statusBar(), SLOT (setVisible(bool)), actionCollection() );
	actionCollection()->addAction ( "settings_showstatusbar", d->statusbarAction );

	KAction* act = KStandardAction::keyBindings ( guiFactory(), SLOT (configureShortcuts()), actionCollection() );
	actionCollection()->addAction ( "settings_keys", act );

	KAction *configureGlobalShortcutsAction = new KAction ( KIcon ( "configure-shortcuts" ), i18n ( "Configure &Global Shortcuts..." ), this );
	configureGlobalShortcutsAction->setMenuRole( QAction::NoRole ); //OS X: prevent Qt heuristics to move action to app menu->"Preferences"
	actionCollection()->addAction ( "settings_global", configureGlobalShortcutsAction );
	connect ( configureGlobalShortcutsAction, SIGNAL (triggered(bool)), this, SLOT (slotConfGlobalKeys()) );

	KStandardAction::configureToolbars ( this, SLOT (slotConfToolbar()), actionCollection() );
	act = KStandardAction::configureNotifications ( this, SLOT (slotConfNotifications()), actionCollection() );
	actionCollection()->addAction ( "settings_notifications", act );

	d->actionShowAllOfflineEmpty = new KToggleAction ( KIcon ( "view-user-offline-kopete" ), i18n ( "Show &All" ), this );
	actionCollection()->addAction ( "settings_show_all_offline_empty", d->actionShowAllOfflineEmpty );
	d->actionShowAllOfflineEmpty->setShortcut ( KShortcut ( Qt::CTRL + Qt::Key_U ) );
	connect ( d->actionShowAllOfflineEmpty, SIGNAL (triggered(bool)), this, SLOT (slotToggleShowAllOfflineEmpty(bool)) );

	d->actionShowOfflineUsers = new KToggleAction ( KIcon ( "view-user-offline-kopete" ), i18n ( "Show Offline &Users" ), this );
	actionCollection()->addAction ( "settings_show_offliners", d->actionShowOfflineUsers );
	connect ( d->actionShowOfflineUsers, SIGNAL (triggered(bool)), this, SLOT (slotToggleShowOfflineUsers()) );

	d->actionShowEmptyGroups = new KToggleAction ( KIcon ( "folder-grey" ), i18n ( "Show Empty &Groups" ), this );
	actionCollection()->addAction ( "settings_show_empty_groups", d->actionShowEmptyGroups );
	d->actionShowEmptyGroups->setShortcut ( KShortcut ( Qt::CTRL + Qt::Key_G ) );
	connect ( d->actionShowEmptyGroups, SIGNAL (triggered(bool)), this, SLOT (slotToggleShowEmptyGroups()) );

    /* The following are highly misleading together with the checkbox, consider removing them - ahartmetz
	d->actionShowAllOfflineEmpty->setCheckedState ( KGuiItem ( i18n ( "Hide O&ffline" ) ) );
	d->actionShowOfflineUsers->setCheckedState ( KGuiItem ( i18n ( "Hide Offline &Users" ) ) );
	d->actionShowEmptyGroups->setCheckedState ( KGuiItem ( i18n ( "Hide Empty &Groups" ) ) );
    */

	KFilterProxySearchLine* searchLine = new KFilterProxySearchLine ( this );
	searchLine->setProxy( d->proxyModel );
	KAction *quickSearch = new KAction ( i18n ( "Quick Search Bar" ), this );
	actionCollection()->addAction ( "quicksearch_bar", quickSearch );
	quickSearch->setDefaultWidget ( searchLine );

	// sync actions, config and prefs-dialog
	connect ( Kopete::AppearanceSettings::self(), SIGNAL (configChanged()), this, SLOT (slotConfigChanged()) );
	slotConfigChanged();

	// Global actions
	KAction *globalReadMessage = new KAction ( i18n ( "Read Message" ), this );
	actionCollection()->addAction ( "ReadMessage",  globalReadMessage );
	connect ( globalReadMessage, SIGNAL (triggered(bool)), Kopete::ChatSessionManager::self(), SLOT (slotReadMessage()) );
	globalReadMessage->setGlobalShortcut ( KShortcut ( Qt::CTRL + Qt::SHIFT + Qt::Key_I ) );
	globalReadMessage->setWhatsThis ( i18n ( "Read the next pending message" ) );

	KAction *globalShowContactList = new KAction ( i18n ( "Show/Hide Contact List" ), this );
	actionCollection()->addAction ( "ShowContactList", globalShowContactList );
	connect ( globalShowContactList, SIGNAL (triggered(bool)), this, SLOT (slotShowHide()) );
	globalShowContactList->setGlobalShortcut ( KShortcut ( Qt::CTRL + Qt::ALT + Qt::Key_T ) );
	globalShowContactList->setWhatsThis ( i18n ( "Show or hide the contact list" ) );

	KAction *globalSetAway = new KAction ( i18n ( "Set Away/Back" ), this );
	actionCollection()->addAction ( "Set_Away_Back",  globalSetAway );
	connect ( globalSetAway, SIGNAL (triggered(bool)), this, SLOT (slotToggleAway()) );
	globalSetAway->setGlobalShortcut ( KShortcut ( Qt::CTRL + Qt::SHIFT + Qt::Key_W ) );
}

void KopeteWindow::slotShowHide()
{
	if ( isActiveWindow() )
	{
		d->autoHideTimer->stop(); //no timeouts if active
		hide();
	}
	else
	{
		show();
#ifdef Q_WS_X11
		if ( !KWindowSystem::windowInfo ( winId(),NET::WMDesktop ).onAllDesktops() )
			KWindowSystem::setOnDesktop ( winId(), KWindowSystem::currentDesktop() );
#endif
		raise();
		KWindowSystem::forceActiveWindow( winId() );
	}
}

void KopeteWindow::slotToggleAway()
{
	kDebug ( 14000 );
	Kopete::StatusManager * statusManager = Kopete::StatusManager::self();
	const Kopete::Status::StatusItem * item = 0;
	bool away = Kopete::StatusManager::self()->globalAway();

	foreach (const Kopete::Status::StatusItem *i, statusManager->getRootGroup()->childList()) {
		if (i->title() == QLatin1String("Online") && away ) {
			item = i;
			break;
		} else if (i->title() == QLatin1String("Away") && !away) {
			item = i;
			break;
		}
	}

	const Kopete::Status::Status * status = qobject_cast<const Kopete::Status::Status*>(item);
	if (status) {
		statusManager->setGlobalStatusMessage(Kopete::StatusMessage(status->title(), status->message()));
	}

	if ( away )
		slotGlobalAvailable();
	else
		slotGlobalAway();
}

void KopeteWindow::initSystray()
{
	if ( Kopete::BehaviorSettings::self()->showSystemTray() ) {
		d->tray = KopeteSystemTray::systemTray ( this );

		QObject::connect ( d->tray, SIGNAL (aboutToShowMenu(KMenu*)),
						   this, SLOT (slotTrayAboutToShowMenu(KMenu*)) );
		// :FIXME: The signal quitSelected does not exist on KopeteSystemTray
		// QObject::connect ( d->tray, SIGNAL (quitSelected()), this, SLOT (slotQuit()) );
	}
}

KopeteWindow::~KopeteWindow()
{
	delete d;
}

bool KopeteWindow::eventFilter ( QObject* target, QEvent* event )
{
	KToolBar *toolBar = dynamic_cast<KToolBar*> ( target );
	QAction *resetAction = actionCollection()->action ( "quicksearch_reset" );

	if ( toolBar && resetAction && resetAction->associatedWidgets().contains ( toolBar ) )
	{

		if ( event->type() == QEvent::Hide )
		{
			resetAction->trigger();
			return true;
		}
		return KXmlGuiWindow::eventFilter ( target, event );
	}

	return KXmlGuiWindow::eventFilter ( target, event );
}

void KopeteWindow::loadOptions()
{
	KSharedConfig::Ptr config = KGlobal::config();

	toolBar ( "mainToolBar" )->applySettings ( config->group ( "ToolBar Settings" ) );
	toolBar ( "quickSearchBar" )->applySettings ( config->group ( "QuickSearchBar Settings" ) );

	applyMainWindowSettings ( config->group ( "General Options" ) );
	KConfigGroup cg ( config, "General Options" );
	QPoint pos = cg.readEntry ( "Position", QPoint(-1, -1) );
	if ( pos.x() != -1 || pos.y() != -1 )
		move ( pos );

	QSize size = cg.readEntry ( "Geometry", QSize() );
	if ( size.isEmpty() ) // Default size
		resize ( QSize ( 272, 400 ) );
	else
		resize ( size );

	d->autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	d->autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();


	QString tmp = cg.readEntry ( "WindowState", "Shown" );
	if ( tmp == "Minimized" && Kopete::BehaviorSettings::self()->showSystemTray() )
	{
		showMinimized();
	}
	else if ( tmp == "Hidden" && Kopete::BehaviorSettings::self()->showSystemTray() )
	{
		hide();
	}
	else if ( !Kopete::BehaviorSettings::self()->startDocked() || !Kopete::BehaviorSettings::self()->showSystemTray() )
		show();

	d->menubarAction->setChecked ( !menuBar()->isHidden() );
	d->statusbarAction->setChecked ( !statusBar()->isHidden() );
}

void KopeteWindow::saveOptions()
{
	KConfigGroup mainToolbarGroup ( KGlobal::config(), "ToolBar Settings" );
	toolBar ( "mainToolBar" )->saveSettings ( mainToolbarGroup );
	KConfigGroup qsbGroup ( KGlobal::config(), "QuickSearchBar Settings" );
	toolBar ( "quickSearchBar" )->saveSettings ( qsbGroup );

	KConfigGroup cg ( KGlobal::config(), "General Options" );
	saveMainWindowSettings ( cg );

	cg.writeEntry ( "Position", pos() );
	cg.writeEntry ( "Geometry", size() );

	if ( isMinimized() )
	{
		cg.writeEntry ( "WindowState", "Minimized" );
	}
	else if ( isHidden() )
	{
		cg.writeEntry ( "WindowState", "Hidden" );
	}
	else
	{
		cg.writeEntry ( "WindowState", "Shown" );
	}

	Kopete::Identity *identity = d->identitywidget->identity();
	if ( identity )
		cg.writeEntry ( "ShownIdentityId", identity->id() );
	else
		cg.writeEntry ( "ShownIdentityId", QString() );

	cg.sync();
}

void KopeteWindow::slotToggleShowAllOfflineEmpty ( bool toggled )
{
	d->actionShowOfflineUsers->setChecked ( toggled );
	d->actionShowEmptyGroups->setChecked ( toggled );
	Kopete::AppearanceSettings::self()->setShowOfflineUsers ( toggled );
	Kopete::AppearanceSettings::self()->setShowEmptyGroups ( toggled );
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

static bool compareOnlineStatus(const Kopete::Account *a, const Kopete::Account *b);
static bool invertedCompareOnlineStatus(const Kopete::Account *a, const Kopete::Account *b);

void KopeteWindow::slotConfigChanged()
{
	bool groupContactByGroupModel = qobject_cast<Kopete::UI::ContactListTreeModel*>( d->model );
	if ( groupContactByGroupModel != Kopete::AppearanceSettings::self()->groupContactByGroup() )
	{
		Kopete::UI::ContactListModel* oldModel = d->model;
		if ( Kopete::AppearanceSettings::self()->groupContactByGroup() )
			d->model = new Kopete::UI::ContactListTreeModel( this );
		else
			d->model = new Kopete::UI::ContactListPlainModel( this );

		d->model->init();
		d->proxyModel->setSourceModel( d->model );
		oldModel->deleteLater();
	}

	if ( isHidden() && !Kopete::BehaviorSettings::self()->showSystemTray() ) // user disabled systray while kopete is hidden, show it!
		show();

	d->actionShowAllOfflineEmpty->setChecked ( Kopete::AppearanceSettings::self()->showOfflineUsers() && Kopete::AppearanceSettings::self()->showEmptyGroups() );
	d->actionShowOfflineUsers->setChecked ( Kopete::AppearanceSettings::self()->showOfflineUsers() );
	d->actionShowEmptyGroups->setChecked ( Kopete::AppearanceSettings::self()->showEmptyGroups() );

	if ( d->showIdentityIcons != Kopete::AppearanceSettings::self()->showIdentityIcons() )
	{
		// Delete status bar icons
		if ( d->showIdentityIcons )
		{
			if ( d->identitywidget->isVisible() )
			{
				d->identitywidget->setIdentity( 0 );
				d->identitywidget->setVisible( false );
			}

			qDeleteAll( d->identityStatusBarIcons );
			d->identityStatusBarIcons.clear();
		}
		else
		{
			qDeleteAll( d->accountStatusBarIcons );
			d->accountStatusBarIcons.clear();
		}

		// Add new status bar icons
		d->showIdentityIcons = Kopete::AppearanceSettings::self()->showIdentityIcons();
		if ( d->showIdentityIcons )
		{
			Kopete::Identity::List identityList = Kopete::IdentityManager::self()->identities();
			foreach ( Kopete::Identity *identity, identityList )
			{
				KopeteIdentityStatusBarIcon *sbIcon = new KopeteIdentityStatusBarIcon ( identity, d->statusBarWidget );
				connect ( sbIcon, SIGNAL(leftClicked(Kopete::Identity*,QPoint)), this,
				          SLOT(slotIdentityStatusIconLeftClicked(Kopete::Identity*,QPoint)) );

				d->identityStatusBarIcons.insert ( identity, sbIcon );
				slotIdentityStatusIconChanged ( identity );
				slotIdentityToolTipChanged ( identity );
			}
		}
		else
		{
			QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
			qSort(accountList.begin(), accountList.end(), invertedCompareOnlineStatus);
			foreach ( Kopete::Account *account, accountList )
			{
				KopeteAccountStatusBarIcon *sbIcon = new KopeteAccountStatusBarIcon ( account, d->statusBarWidget );
				d->accountStatusBarIcons.insert ( account, sbIcon );
			}
		}
	}
}

void KopeteWindow::slotContactListAppearanceChanged()
{
	d->autoHide = Kopete::AppearanceSettings::self()->contactListAutoHide();
	d->autoHideTimeout = Kopete::AppearanceSettings::self()->contactListAutoHideTimeout();

	startAutoHideTimer();
}

void KopeteWindow::slotConfNotifications()
{
	KNotifyConfigWidget::configure ( this );
}

void KopeteWindow::slotConfGlobalKeys()
{
	KShortcutsDialog::configure ( actionCollection() );
}

void KopeteWindow::slotConfToolbar()
{
	KConfigGroup cg ( KGlobal::config(), "General Options" );
	saveMainWindowSettings ( cg );
	KEditToolBar *dlg = new KEditToolBar ( factory() );
	connect ( dlg, SIGNAL (newToolBarConfig()), this, SLOT (slotUpdateToolbar()) );
	connect ( dlg, SIGNAL (finished()) , dlg, SLOT (deleteLater()) );
	dlg->show();
}

void KopeteWindow::slotUpdateToolbar()
{
	applyMainWindowSettings ( KGlobal::config()->group ( "General Options" ) );
}

void KopeteWindow::slotGlobalAway()
{
	Kopete::AccountManager::self()->setOnlineStatus ( Kopete::OnlineStatusManager::Away,
	        Kopete::StatusManager::self()->globalStatusMessage() );
}

void KopeteWindow::slotGlobalBusy()
{
	Kopete::AccountManager::self()->setOnlineStatus ( Kopete::OnlineStatusManager::Busy,
	        Kopete::StatusManager::self()->globalStatusMessage() );
}

void KopeteWindow::slotGlobalAvailable()
{
	Kopete::AccountManager::self()->setOnlineStatus ( Kopete::OnlineStatusManager::Online,
	        Kopete::StatusManager::self()->globalStatusMessage() );
}

void KopeteWindow::slotSetInvisibleAll()
{
	Kopete::AccountManager::self()->setOnlineStatus ( Kopete::OnlineStatusManager::Invisible,
	        Kopete::StatusManager::self()->globalStatusMessage() );
}

void KopeteWindow::slotDisconnectAll()
{
	Kopete::AccountManager::self()->setOnlineStatus ( Kopete::OnlineStatusManager::Offline,
	        Kopete::StatusManager::self()->globalStatusMessage() );
}

bool KopeteWindow::queryClose()
{
	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	if ( app->sessionSaving() || app->isShuttingDown() ) {
		// we are shutting down, don't show any message
		return true;
	}

	Kopete::PluginList list = Kopete::PluginManager::self()->loadedPlugins();
	foreach ( Kopete::Plugin *plugin, list ) {
		bool shown = false;
		QMetaObject::invokeMethod(plugin, "showCloseWindowMessage", Qt::DirectConnection, Q_RETURN_ARG(bool, shown));
		if ( shown ) {
			// A message box has just been shown. Stop now, we do not want
			// to spam the user with multiple message boxes.
			return true;
		}
	}

	if ( Kopete::BehaviorSettings::self()->showSystemTray()
	     && !isHidden() )
		// I would make this a KMessageBox::queuedMessageBox but there doesn't seem to be don'tShowAgain support for those
		KMessageBox::information ( this,
		                           i18n ( "<qt>Closing the main window will keep Kopete running in the "
		                                  "system tray. Use 'Quit' from the 'File' menu to quit the application.</qt>" ),
		                           i18n ( "Docking in System Tray" ), "hideOnCloseInfo" );
//	else	// we are shutting down either user initiated or session management
//	Kopete::PluginManager::self()->shutdown();

	return true;
}

bool KopeteWindow::shouldExitOnClose() const
{
	Kopete::PluginList list = Kopete::PluginManager::self()->loadedPlugins();
	foreach ( Kopete::Plugin *plugin, list ) {
		bool ok = true;
		QMetaObject::invokeMethod(plugin, "shouldExitOnClose", Qt::DirectConnection, Q_RETURN_ARG(bool, ok));
		if ( !ok ) {
			kDebug ( 14000 ) << "plugin" << plugin->displayName() << "does not want to exit";
			return false;
		}
	}
	// If all plugins are OK, consider ourself OK only if there is no tray icon
	return !Kopete::BehaviorSettings::self()->showSystemTray();
}

bool KopeteWindow::queryExit()
{
	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	if ( app->sessionSaving()
	        || app->isShuttingDown() /* only set if KopeteApplication::quitKopete() or
									KopeteApplication::commitData() called */
	        || shouldExitOnClose()
	        || isHidden() )
	{
		saveOptions();
		kDebug ( 14000 ) << " shutting down plugin manager";
		Kopete::PluginList list = Kopete::PluginManager::self()->loadedPlugins();
		foreach ( Kopete::Plugin *plugin, list ) {
			guiFactory()->removeClient(plugin);
		}
		Kopete::PluginManager::self()->shutdown();
		return true;
	}
	else
		return false;
}

void KopeteWindow::closeEvent ( QCloseEvent *e )
{
	// if we are not ok to exit on close and we are not shutting down then just do what needs to be done if a
	// window is closed.
	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	if ( !shouldExitOnClose() && !app->isShuttingDown() && !app->sessionSaving() ) {
		// BEGIN of code borrowed from KMainWindow::closeEvent
		// Save settings if auto-save is enabled, and settings have changed
		if ( settingsDirty() && autoSaveSettings() )
			saveAutoSaveSettings();

		if ( queryClose() ) {
			e->accept();
		}
		// END of code borrowed from KMainWindow::closeEvent
		kDebug ( 14000 ) << "just closing because we have a system tray icon";
	}
	else
	{
		kDebug ( 14000 ) << "delegating to KXmlGuiWindow::closeEvent()";
		KXmlGuiWindow::closeEvent ( e );
	}
}

void KopeteWindow::slotQuit()
{
	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	app->quitKopete();

	if ( d->tray && app->isShuttingDown() )
	{
		d->tray->deleteLater();
		d->tray = 0;
	}
}

void KopeteWindow::slotPluginLoaded ( Kopete::Plugin *  p )
{
	guiFactory()->addClient ( p );
}

void KopeteWindow::slotAllPluginsLoaded()
{
//	actionConnect->setEnabled(true);
	d->actionDisconnect->setEnabled ( true );

	KConfigGroup cg( KGlobal::config(), "General Options" );

	// If some account already loaded, build the status icon
	QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
	qSort(accountList.begin(), accountList.end(), invertedCompareOnlineStatus);
	foreach ( Kopete::Account *a, accountList )
	slotAccountRegistered ( a );

	//Connect the appropriate account signals
	/* Please note that I tried to put this in the slotAllPluginsLoaded() function
	 * but it seemed to break the account icons in the statusbar --Matt */
	connect ( Kopete::AccountManager::self(), SIGNAL (accountRegistered(Kopete::Account*)),
	          this, SLOT (slotAccountRegistered(Kopete::Account*)) );
	connect ( Kopete::AccountManager::self(), SIGNAL (accountUnregistered(const Kopete::Account*)),
	          this, SLOT (slotAccountUnregistered(const Kopete::Account*)) );

	if ( d->showIdentityIcons )
	{
		QString identityId = cg.readEntry( "ShownIdentityId", Kopete::IdentityManager::self()->defaultIdentity()->id() );
		if ( !identityId.isEmpty() )
		{
			Kopete::Identity* identity = Kopete::IdentityManager::self()->findIdentity( identityId );
			if ( identity )
				slotIdentityStatusIconLeftClicked( identity, QPoint() );
		}
	}
}

void KopeteWindow::slotIdentityRegistered ( Kopete::Identity *identity )
{
	if ( !identity )
		return;

	connect ( identity, SIGNAL(onlineStatusChanged(Kopete::Identity*)),
	          this, SLOT(slotIdentityStatusIconChanged(Kopete::Identity*)) );
	connect ( identity, SIGNAL(identityChanged(Kopete::Identity*)),
	          this, SLOT(slotIdentityStatusIconChanged(Kopete::Identity*)) );
	connect ( identity, SIGNAL(toolTipChanged(Kopete::Identity*)),
	          this, SLOT(slotIdentityToolTipChanged(Kopete::Identity*)) );

	if ( d->showIdentityIcons )
	{
		KopeteIdentityStatusBarIcon *sbIcon = new KopeteIdentityStatusBarIcon ( identity, d->statusBarWidget );
		connect ( sbIcon, SIGNAL (leftClicked(Kopete::Identity*,QPoint)),
		          SLOT (slotIdentityStatusIconLeftClicked(Kopete::Identity*,QPoint)) );

		d->identityStatusBarIcons.insert ( identity, sbIcon );
	}

	slotIdentityStatusIconChanged ( identity );
	slotIdentityToolTipChanged( identity );
}

void KopeteWindow::slotIdentityUnregistered ( const Kopete::Identity *identity )
{
	kDebug ( 14000 ) ;

	if ( d->showIdentityIcons )
	{
		KopeteIdentityStatusBarIcon *sbIcon = d->identityStatusBarIcons.value ( identity, 0 );
		if ( sbIcon )
		{
			d->identityStatusBarIcons.remove ( identity );
			delete sbIcon;
		}
	}

	makeTrayToolTip();

}

void KopeteWindow::slotIdentityToolTipChanged ( Kopete::Identity *identity )
{
	if ( d->appDestroyed )
		return;

	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	if ( app->sessionSaving() || app->isShuttingDown() )
		return;

	// Adds tooltip for each status icon, useful in case you have many accounts
	// over one protocol
	KopeteIdentityStatusBarIcon *i = d->identityStatusBarIcons.value ( identity, 0 );
	if ( i )
		i->setToolTip ( identity->toolTip() );

	makeTrayToolTip();
}

void KopeteWindow::slotIdentityStatusIconChanged ( Kopete::Identity *identity )
{
	kDebug ( 14000 ) << identity->property ( Kopete::Global::Properties::self()->statusMessage() ).value();
	// update the global status label if the change doesn't
//	QString newAwayMessage = contact->property( Kopete::Global::Properties::self()->awayMessage() ).value().toString();
//	if ( status.status() != Kopete::OnlineStatus::Connecting )
//	{
//		QString globalMessage = m_globalStatusMessage->text();
//		if ( newAwayMessage != globalMessage )
//			m_globalStatusMessage->setText( ""i18n("status message to show when different accounts have different status messages", "(multiple)" );
//	}
//	kDebug(14000) << "Icons: '" <<
//		status.overlayIcons() << "'" << endl;

	if ( d->appDestroyed )
		return;

	KopeteApplication *app = static_cast<KopeteApplication *> ( kapp );
	if ( app->sessionSaving() || app->isShuttingDown() )
		return;

	if ( identity->onlineStatus() != Kopete::OnlineStatus::Connecting )
	{
		// FIXME: It's not global status so don't save it
		//Kopete::StatusManager::self()->setGlobalStatusMessage( identity->property( Kopete::Global::Properties::self()->statusMessage() ).value().toString() );
	}

	KopeteIdentityStatusBarIcon *i = d->identityStatusBarIcons.value ( identity, 0 );
	if ( !i )
		return;

	QPixmap pm;
	switch ( identity->onlineStatus() ) {
		case Kopete::OnlineStatus::Offline:
		case Kopete::OnlineStatus::Connecting:
			pm = SmallIcon ( "user-identity", 0, KIconLoader::DefaultState,
			                 QStringList() << QString() << "user-offline" );
			break;
		case Kopete::OnlineStatus::Invisible:
			pm = SmallIcon ( "user-identity", 0, KIconLoader::DefaultState,
			                 QStringList() << QString() << "user-invisible" );
			break;
		case Kopete::OnlineStatus::Away:
			pm = SmallIcon ( "user-identity", 0, KIconLoader::DefaultState,
			                 QStringList() << QString() << "user-away" );
			break;
		case Kopete::OnlineStatus::Busy:
			pm = SmallIcon ( "user-identity", 0, KIconLoader::DefaultState,
			                 QStringList() << QString() << "user-busy" );
			break;
		case Kopete::OnlineStatus::Online:
			pm = SmallIcon ( "user-identity", 0, KIconLoader::DefaultState,
			                 QStringList() << QString() << "user-online" );
			break;
		case Kopete::OnlineStatus::Unknown:
			pm = SmallIcon ( "user-identity" );
			break;
	}

	// No Pixmap found, fallback to Unknown
	if ( pm.isNull() )
		i->setPixmap ( SmallIcon ( "user-identity" ) );
	else
		i->setPixmap ( pm );
}

static bool compareOnlineStatus(const Kopete::Account *a, const Kopete::Account *b)
{
	int c = 0;

	if (a->identity() && b->identity()) {
		c = QString::localeAwareCompare(a->identity()->label(), b->identity()->label());
	}

	if (c == 0) {
		c = a->myself()->onlineStatus().status() - b->myself()->onlineStatus().status();

		if (c == 0) {
			return (QString::localeAwareCompare(a->protocol()->displayName(), b->protocol()->displayName()) < 0);
		}
		return (c > 0);
	}
	return (c < 0);
}

static bool invertedCompareOnlineStatus(const Kopete::Account *a, const Kopete::Account *b)
{
	return !compareOnlineStatus(a, b);
}

void KopeteWindow::makeTrayToolTip()
{
	//FIXME: maybe use identities here?
	//the tool-tip of the systemtray.
	if ( d->tray )
	{
		QString tt = QLatin1String ( "<qt>" );
		QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
		qSort(accountList.begin(), accountList.end(), compareOnlineStatus);
		foreach ( Kopete::Account *a, accountList )
		{
			Kopete::Contact *self = a->myself();
			/*tt += i18nc ( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)</nobr><br />",
			              "<nobr><img src=\"kopete-account-icon:%3:%4\" /> <b>%1:</b> %2 (<i>%5</i>)</nobr><br />",
			              a->protocol()->displayName(), a->accountLabel(), QString ( QUrl::toPercentEncoding ( a->protocol()->pluginId() ) ),
			              QString ( QUrl::toPercentEncoding ( a->accountId() ) ), self->onlineStatus().description() );*/
			tt += i18nc ( "Account tooltip information: <nobr>ICON <b>PROTOCOL:</b> NAME (<i>STATUS</i>)</nobr><br />",
			              "<nobr><img src=\"%3\" width=\"16\" height=\"16\" /> <b>%1:</b> %2 (<i>%4</i>)</nobr><br />",
			              a->protocol()->displayName(), a->accountLabel(),
				      a->accountIconPath(KIconLoader::Small), self->onlineStatus().description() );
		}
		tt += QLatin1String ( "</qt>" );
		d->tray->setToolTip ( "kopete", i18n("Kopete"), tt );
	}
}

void KopeteWindow::slotIdentityStatusIconLeftClicked ( Kopete::Identity *identity, const QPoint &p )
{
	Q_UNUSED ( p )
	if ( d->identitywidget->isVisible() && d->identitywidget->identity() == identity )
	{
		d->identitywidget->setIdentity ( 0 );
		d->identitywidget->setVisible ( false );
		return;
	}

	if ( d->infoEventWidget->isVisible() )
		d->infoEventWidget->setVisible ( false );

	d->identitywidget->setIdentity ( identity );
	d->identitywidget->setVisible ( true );
}

void KopeteWindow::slotShowInfoEventWidget()
{
	if ( d->identitywidget->isVisible() )
	{
		d->identitywidget->setIdentity( 0 );
		d->identitywidget->setVisible( false );
	}

	if ( !d->infoEventWidget->isVisible() )
		d->infoEventWidget->setVisible( true );

	if ( !isActiveWindow() )
		slotShowHide();
}

void KopeteWindow::slotInfoIconClicked()
{
	if ( d->infoEventWidget->isVisible() )
	{
		d->infoEventWidget->setVisible( false );
	}
	else
	{
		if ( d->identitywidget->isVisible() )
		{
			d->identitywidget->setIdentity( 0 );
			d->identitywidget->setVisible( false );
		}
		d->infoEventWidget->setVisible( true );
	}
}

void KopeteWindow::slotAccountRegistered ( Kopete::Account *account )
{

	//enable the connect all toolbar button
//	actionConnect->setEnabled(true);
	d->actionDisconnect->setEnabled ( true );

	// add an item for this account to the add contact actionmenu
	QString s = QString ( "actionAdd%1Contact" ).arg ( account->accountId() );
	KAction *action = new KAction ( KIcon ( account->accountIcon() ), account->accountLabel(), this );
	actionCollection()->addAction ( s, action );
	connect ( action, SIGNAL (triggered(bool)), d->addContactMapper, SLOT (map()) );
	connect ( account, SIGNAL(colorChanged(QColor)), this, SLOT(slotAccountColorChanged()) );

	d->addContactMapper->setMapping ( action, account->protocol()->pluginId() + QChar ( 0xE000 ) + account->accountId() );
	d->actionAddContact->addAction ( action );

	if ( !d->showIdentityIcons )
	{
		KopeteAccountStatusBarIcon *sbIcon = new KopeteAccountStatusBarIcon ( account, d->statusBarWidget );
		d->accountStatusBarIcons.insert ( account, sbIcon );
	}
}

void KopeteWindow::slotAccountColorChanged()
{
	Kopete::Account* account = qobject_cast<Kopete::Account*>(sender());
	Q_ASSERT(account);

	// update add contact actionmenu
	QString s = QString( "actionAdd%1Contact" ).arg( account->accountId() );
	QAction *action = actionCollection()->action ( s );
	if ( action )
		action->setIcon( KIcon( account->accountIcon() ) );
}

void KopeteWindow::slotAccountUnregistered ( const Kopete::Account *account )
{
	QList<Kopete::Account *> accounts = Kopete::AccountManager::self()->accounts();
	if ( accounts.isEmpty() )
	{
//		actionConnect->setEnabled(false);
		d->actionDisconnect->setEnabled ( false );
	}

	disconnect ( account, SIGNAL(colorChanged(QColor)), this, SLOT(slotAccountColorChanged()) );

	// update add contact actionmenu
	QString s = QString ( "actionAdd%1Contact" ).arg ( account->accountId() );
	QAction *action = actionCollection()->action ( s );
	if ( action )
	{
		kDebug ( 14000 ) << " found KAction " << action << " with name: " << action->objectName();
		d->addContactMapper->removeMappings ( action );
		d->actionAddContact->removeAction ( action );
	}

	if ( !d->showIdentityIcons )
	{
		KopeteAccountStatusBarIcon *sbIcon = d->accountStatusBarIcons.value ( account, 0 );
		if ( sbIcon )
		{
			d->accountStatusBarIcons.remove ( account );
			delete sbIcon;
		}
	}
}

void KopeteWindow::slotTrayAboutToShowMenu ( KMenu * popup )
{
	KActionCollection *actionCollection = d->tray->actionCollection();

	popup->clear();
	popup->addTitle ( qApp->windowIcon(), KGlobal::caption() );

	QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();
	qSort(accountList.begin(), accountList.end(), invertedCompareOnlineStatus);
	foreach ( Kopete::Account *account, accountList )
	{
		KActionMenu *menu = new KActionMenu ( account->accountId(), account );
		menu->setIcon( account->myself()->onlineStatus().iconFor( account ) );

		if ( !account->hasCustomStatusMenu() )
			Kopete::StatusRootAction::createAccountStatusActions ( account, menu );

		account->fillActionMenu ( menu );
		popup->addAction ( menu );

		connect ( popup , SIGNAL (aboutToHide()) , menu , SLOT (deleteLater()) );
	}

	popup->addSeparator();
	popup->addAction ( d->actionStatusMenu );
	popup->addSeparator();
	popup->addAction ( d->actionPrefs );
	popup->addAction ( d->actionAddContact );
	popup->addSeparator();
	popup->addAction ( actionCollection->action ( "minimizeRestore" ) );
	popup->addAction ( actionCollection->action ( KStandardAction::name ( KStandardAction::Quit ) ) );
}

void KopeteWindow::showExportDialog()
{
	KabcExportWizard* wizard = new KabcExportWizard ( this );
	wizard->setObjectName ( QLatin1String ( "export_contact_dialog" ) );
	wizard->show();
}

void KopeteWindow::leaveEvent ( QEvent * )
{
	startAutoHideTimer();
}

void KopeteWindow::showEvent ( QShowEvent * )
{
	startAutoHideTimer();
	slotStartAutoResizeTimer();
}

void KopeteWindow::hideEvent ( QHideEvent * )
{
	d->autoResizeTimer->stop();
}

void KopeteWindow::slotAutoHide()
{
	if ( this->geometry().contains ( QCursor::pos() ) == false )
	{
		/* The autohide-timer doesn't need to emit
		* timeouts when the window is hidden already. */
		d->autoHideTimer->stop();
		hide();
	}
}

void KopeteWindow::startAutoHideTimer()
{
	if ( d->autoHideTimeout > 0 && d->autoHide == true && isVisible() && Kopete::BehaviorSettings::self()->showSystemTray() )
		d->autoHideTimer->start ( d->autoHideTimeout * 1000 );
}

void KopeteWindow::slotStartAutoResizeTimer()
{
	if ( Kopete::AppearanceSettings::contactListAutoResize() == true )
		if ( ! d->autoResizeTimer->isActive() )
			d->autoResizeTimer->start ( 1000 );
}

void KopeteWindow::setOnlineStatus( uint category, const Kopete::StatusMessage& statusMessage )
{
	Kopete::AccountManager::self()->setOnlineStatus( category, statusMessage, 0, true );
}

void KopeteWindow::setStatusMessage ( const Kopete::StatusMessage& statusMessage )
{
	Kopete::StatusManager::self()->setGlobalStatusMessage ( statusMessage );
}

void KopeteWindow::globalStatusChanged()
{
	QString statusTitle = Kopete::StatusManager::self()->globalStatusMessage().title();
	QString statusMessage = Kopete::StatusManager::self()->globalStatusMessage().message();
	d->globalStatusMessage->setText( statusTitle );

	QString toolTip;
	toolTip += i18nc("@label:textbox formatted status title", "<b>Status&nbsp;Title:</b>&nbsp;%1",
	                 Kopete::Emoticons::parseEmoticons( Kopete::Message::escape(statusTitle) ) );

	toolTip += i18nc("@label:textbox formatted status message", "<br /><b>Status&nbsp;Message:</b>&nbsp;%1",
	                 Kopete::Emoticons::parseEmoticons( Kopete::Message::escape(statusMessage) ) );

	d->globalStatusMessage->setToolTip( toolTip );
}

void KopeteWindow::slotGlobalStatusMessageIconClicked ( const QPoint &position )
{
	KMenu *menu = new KMenu ( this );

	menu->addTitle ( i18n ( "Status Message" ) );

	Kopete::UI::StatusEditAction* statusEditAction = new Kopete::UI::StatusEditAction ( this );
	statusEditAction->setStatusMessage ( Kopete::StatusManager::self()->globalStatusMessage() );
	connect ( statusEditAction, SIGNAL (statusChanged(Kopete::StatusMessage)),
	          this, SLOT (setStatusMessage(Kopete::StatusMessage)) );

	menu->addAction ( statusEditAction );
	menu->exec ( position );

	statusEditAction->deleteLater();
	delete menu;
}

void KopeteWindow::slotAddContactDialogInternal ( const QString & accountIdentifier )
{
	QString protocolId = accountIdentifier.section ( QChar ( 0xE000 ), 0, 0 );
	QString accountId = accountIdentifier.section ( QChar ( 0xE000 ), 1, 1 );
	Kopete::Account *account = Kopete::AccountManager::self()->findAccount ( protocolId, accountId );
	showAddContactDialog ( account );
}

void KopeteWindow::updateStatusMenuMessage ( Kopete::StatusRootAction *statusRootAction )
{
	statusRootAction->setCurrentMessage ( Kopete::StatusManager::self()->globalStatusMessage() );
}

void KopeteWindow::showAddContactDialog ( Kopete::Account * account )
{
	if ( !account ) {
		kDebug ( 14000 ) << "no account given";
		return;
	}

	KDialog *addDialog = new KDialog ( this );
	addDialog->setCaption ( i18n ( "Add Contact" ) );
	addDialog->setButtons ( KDialog::Ok | KDialog::Cancel );
	addDialog->setDefaultButton ( KDialog::Ok );
	addDialog->showButtonSeparator ( true );

	KVBox * mainWid = new KVBox ( addDialog );

	AddContactPage *addContactPage =
	    account->protocol()->createAddContactWidget ( mainWid, account );

	QWidget* groupKABC = new QWidget ( mainWid );
	groupKABC->setObjectName ( "groupkabcwidget" );
	Ui::GroupKABCSelectorWidget ui_groupKABC;
	ui_groupKABC.setupUi ( groupKABC );

	// Populate the groups list
	Kopete::GroupList groups=Kopete::ContactList::self()->groups();
	QHash<QString, Kopete::Group*> groupItems;

	// Add top level group
	groupItems.insert ( Kopete::Group::topLevel()->displayName(), Kopete::Group::topLevel() );
	ui_groupKABC.groupCombo->addItem ( Kopete::Group::topLevel()->displayName() );

	foreach ( Kopete::Group *group, groups )
	{
		if ( group->type() != Kopete::Group::Normal )
			continue;
		QString groupname = group->displayName();
		if ( !groupname.isEmpty() )
		{
			groupItems.insert ( groupname, group );
			ui_groupKABC.groupCombo->addItem ( groupname );
		}
	}

	if ( !addContactPage )
	{
		kDebug ( 14000 ) <<
		"Error while creating addcontactpage" << endl;
	}
	else
	{
		addDialog->setMainWidget ( mainWid );
		if ( addDialog->exec() == QDialog::Accepted )
		{
			if ( addContactPage->validateData() )
			{
				Kopete::MetaContact * metacontact = new Kopete::MetaContact();
				metacontact->addToGroup ( groupItems[ ui_groupKABC.groupCombo->currentText() ] );
				metacontact->setKabcId ( ui_groupKABC.widAddresseeLink->uid() );
				if ( addContactPage->apply ( account, metacontact ) )
				{
					Kopete::ContactList::self()->addMetaContact ( metacontact );
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

void KopeteWindow::slotUpdateSize()
{
	/* resize with rules:
	- never will be taller than maxTall
	- never shorter than minTall
	- never will resize if contactlist is empty
	- never will resize if cursor is in window
	*/

	if ( this->geometry().contains ( QCursor::pos() ) == true )
	{
		return; // don't do anything if cursor is inside window
	}
	const int amountWindowBiggerThanContactList = 200;
	const QRect workArea = KWindowSystem::workArea();
	const int minHeight = 400;
	QRect newGeometry = geometry();
	const QRect oldGeometry = geometry();
	const int topFrameWidth = - ( frameGeometry().top() - oldGeometry.top() );
	const int bottomFrameWidth = frameGeometry().bottom() - oldGeometry.bottom();

	// desired height is height of full contents of contact list tree, as well as
	// some buffer for other elements in the main window
	int height = d->contactlist->visibleContentHeight();
	newGeometry.setHeight ( height + amountWindowBiggerThanContactList );

	if ( height ) {
		// if new size is too big or too small, bring inside limits
		if ( newGeometry.height() > workArea.height() )
			newGeometry.setHeight ( workArea.height() - topFrameWidth - bottomFrameWidth );
		else if ( newGeometry.height() < minHeight )
			newGeometry.setHeight ( minHeight );

		// set position of new geometry rectangle to same position as the old one
		if ( Kopete::AppearanceSettings::contactListResizeAnchor() ==
		        Kopete::AppearanceSettings::EnumContactListResizeAnchor::Top )
			newGeometry.moveTop ( oldGeometry.top() );
		else
			newGeometry.moveBottom ( oldGeometry.bottom() );

		// if the window + its frame is out of the work area, bring it just inside
		if ( ( newGeometry.top() - topFrameWidth ) < workArea.top() )
			newGeometry.moveTop ( workArea.top() + topFrameWidth );

		else if ( ( newGeometry.bottom() + bottomFrameWidth ) > workArea.bottom() )
			newGeometry.moveBottom ( workArea.bottom() - bottomFrameWidth );

		// do it!
		setGeometry ( newGeometry );
	}
}


void KopeteWindow::slotNewInfoEvent()
{
	if ( !d->infoEventWidget->isVisible() )
	{
		if ( d->identitywidget->isVisible() )
		{
			d->identitywidget->setIdentity( 0 );
			d->identitywidget->setVisible( false );
		}
		d->infoEventWidget->setVisible( true );
	}
}

#include "kopetewindow.moc"
// vim: set noet ts=4 sts=4 sw=4:
