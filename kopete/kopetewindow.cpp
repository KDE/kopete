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

#include <qapplication.h>
#include <qlayout.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kwin.h>

#include "kopeteballoon.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "kopeteevent.h"
#include "kopeteidentitymanager.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "preferencesdialog.h"
#include "systemtray.h"
#include "statusbaricon.h"

KopeteWindow::KopeteWindow( QWidget *parent, const char *name )
: KMainWindow( parent, name )
{
	KWin::setState(winId(), NET::SkipTaskbar);
	kdDebug() << "[KopeteWindow] KopeteWindow()" << endl;

	// Applications should ensure that their StatusBar exists before calling createGUI()
	// so that the StatusBar is always correctly positioned when KDE is configured to use
	// a MacOS-style MenuBar.
	// This fixes a "statusbar drawn over the top of the toolbar" bug
	// e.g. it can happen when you switch desktops on Kopete startup
	statusBar();

	/* -------------------------------------------------------------------------------- */
	initView();
	initActions();
	initSystray();
	/* -------------------------------------------------------------------------------- */

	loadOptions();

	// for now systemtray is also always shown
	// TODO: make the configurable
	isClosing=false;
	tray->show();

	// Trap all loaded plugins, so we can add their status bar icons accordingly
	connect( LibraryLoader::pluginLoader(),
		SIGNAL( pluginLoaded( KopetePlugin * ) ),
		this, SLOT( slotPluginLoaded( KopetePlugin * ) ) );
}

void KopeteWindow::initView ( void )
{
	contactlist = new KopeteContactListView(this);
	setCentralWidget(contactlist);
}

void KopeteWindow::initActions ( void )
{
	actionAddContact = new KAction( i18n( "&Add Contact..." ), "bookmark_add",
		0, KopeteContactList::contactList(), SLOT( showAddContactDialog() ),
		actionCollection(), "AddContact" );

	actionConnect = new KAction( i18n( "&Connect All" ), "connect_creating",
		0, KopeteIdentityManager::manager(), SLOT( connectAll() ),
		actionCollection(), "ConnectAll" );

	actionDisconnect = new KAction( i18n( "&Disconnect All" ), "connect_no",
		0, KopeteIdentityManager::manager(), SLOT( disconnectAll() ),
		actionCollection(), "DisconnectAll" );

	actionConnectionMenu = new KActionMenu( i18n("Connection"),"connect_established",
							actionCollection(), "Connection" );

	actionConnectionMenu->insert(actionConnect);
	actionConnectionMenu->insert(actionDisconnect);

	actionSetAway = new KAction( i18n( "&Set Away Globally" ), "kopeteaway",
		0, KopeteIdentityManager::manager(), SLOT( setAwayAll() ),
		actionCollection(), "SetAwayAll" );

	actionSetAvailable = new KAction( i18n( "Set Availa&ble Globally" ),
		"kopeteavailable", 0 , KopeteIdentityManager::manager(),
		SLOT( setAvailableAll() ), actionCollection(),
		"SetAvailableAll" );

	actionAwayMenu = new KActionMenu( i18n("Status"),"kopetestatus",
							actionCollection(), "Status" );
	actionAwayMenu->setDelayed( false );
	actionAwayMenu->insert(actionSetAvailable);
	actionAwayMenu->insert(actionSetAway);

	actionShowTransfers = new KAction( i18n("Show &File Transfers"),"network", 0 ,
							qApp, SLOT(slotShowTransfers()),
							actionCollection(), "ShowTransfers" );

	actionPrefs = KStdAction::preferences(
		PreferencesDialog::preferencesDialog(), SLOT( show() ),
		actionCollection() );

	actionSave = new KAction( i18n("Save &ContactList"), "filesave", KStdAccel::shortcut(KStdAccel::Save),
							KopeteContactList::contactList(), SLOT(save()),
							actionCollection(), "save_contactlist" );

	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

	toolbarAction = KStdAction::showToolbar(this, SLOT(showToolbar()), actionCollection());
	menubarAction = KStdAction::showMenubar(this, SLOT(showMenubar()), actionCollection());
	statusbarAction = KStdAction::showStatusbar(this, SLOT(showStatusbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(slotConfKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(slotConfToolbar()), actionCollection());

	actionShowOffliners = new KToggleAction( i18n("Show offline &users"), "viewmag", CTRL+Key_V,
			this, SLOT(slotToggleShowOffliners()), actionCollection(), "options_show_offliners" );

	// sync actions, config and prefs-dialog
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	slotConfigChanged();

	createGUI ( "kopeteui.rc" );
}

void KopeteWindow::initSystray ( void )
{
	tray = KopeteSystemTray::systemTray( this, "KopeteSystemTray" );

	KPopupMenu *tm = tray->contextMenu();

	tm->insertSeparator();
	actionAddContact->plug( tm );
	tm->insertSeparator();
//	actionConnect->plug( tm );
//	actionDisconnect->plug( tm );
	actionConnectionMenu->plug ( tm );
	actionAwayMenu->plug( tm );
	tm->insertSeparator();
	actionPrefs->plug( tm );
//	tm->insertSeparator();

#if KDE_VERSION >= 306
	connect(tray,SIGNAL(quitSelected()),this,SLOT(slotQuit()));
#endif
}

KopeteWindow::~KopeteWindow()
{
//	delete tray;
	kdDebug() << "[KopeteWindow] ~KopeteWindow()" << endl;
}

bool KopeteWindow::queryExit()
{
	kdDebug() << "[KopeteWindow] queryExit()" << endl;
	saveOptions();
	//Now in Kopete::~Kopete().
	// (KDE3.1 beta2 did't save contact-list on exit)
	/*	KopeteContactList::contactList()->save();*/
	return true;
}


void KopeteWindow::loadOptions(void)
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->applySettings( config, "ToolBar Settings" );

	applyMainWindowSettings ( config, "General Options" );

	QPoint pos = config->readPointEntry("Position");
	move(pos);

	QSize size = config->readSizeEntry("Geometry");
	if(!size.isEmpty())
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
		KConfig *config = KGlobal::config();
		config->setGroup("Appearance");
		if ( !config->readBoolEntry("StartDocked", false) )
			show();
	}

	toolbarAction->setChecked( !toolBar("mainToolBar")->isHidden() );
	menubarAction->setChecked( !menuBar()->isHidden() );
	statusbarAction->setChecked( !statusBar()->isHidden() );
}

void KopeteWindow::saveOptions(void)
{
	KConfig *config = KGlobal::config();

	toolBar("mainToolBar")->saveSettings ( config, "ToolBar Settings" );

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

void KopeteWindow::showToolbar(void)
{
	if( toolbarAction->isChecked() )
		toolBar("mainToolBar")->show();
	else
		toolBar("mainToolBar")->hide();
}

void KopeteWindow::showMenubar(void)
{
        if( menubarAction->isChecked() )
                menuBar()->show();
        else
                menuBar()->hide();
}

void KopeteWindow::showStatusbar(void)
{
        if( statusbarAction->isChecked() )
                statusBar()->show();
        else
                statusBar()->hide();
}

void KopeteWindow::slotToggleShowOffliners ( void )
{
	kdDebug() << "[KopeteWindow] slotToggleShowOffliners()" << endl;
	kdDebug() << "[KopeteWindow] show offliners KAction is " << actionShowOffliners->isChecked() << endl;

	KopetePrefs *p = KopetePrefs::prefs();
	p->setShowOffline ( actionShowOffliners->isChecked() );

	disconnect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
	p->save();
	connect ( KopetePrefs::prefs(), SIGNAL(saved()), this, SLOT(slotConfigChanged()) );
}

void KopeteWindow::slotConfigChanged()
{
	kdDebug() << "[KopeteWindow] slotConfigChanged()" << endl;
	kdDebug() << "[KopeteWindow] show offliners is " << KopetePrefs::prefs()->showOffline() << endl;

	actionShowOffliners->setChecked( KopetePrefs::prefs()->showOffline() );
}


void KopeteWindow::slotConfKeys()
{
	KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
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


void KopeteWindow::closeEvent( QCloseEvent *e )
{
	if(isClosing)
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
#else
	KMainWindow::closeEvent( e );
#endif
}

void KopeteWindow::slotQuit()
{
	kdDebug() << "KopeteWindow::slotQuit()" << endl;

	//I don't know why, but when this slot is called by the toolbar, that work fine
	// but when this slot is called by the system try, the closeEvent's message is showed
	isClosing=true;
	
	qApp->quit();
}

void KopeteWindow::slotPluginLoaded( KopetePlugin *p )
{
	kdDebug() << "KopeteWindow::slotPluginLoaded()" << endl;

	KopeteProtocol *proto = dynamic_cast<KopeteProtocol *>( p );
	if( !proto )
		return;

	connect( proto,
		SIGNAL( statusIconChanged( KopeteProtocol *, const QString & ) ),
		SLOT( slotProtocolStatusIconChanged( KopeteProtocol *,
		const QString & ) ) );
	connect( proto, SIGNAL( destroyed( QObject * ) ),
		SLOT( slotProtocolDestroyed( QObject * ) ) );

	StatusBarIcon *i = new StatusBarIcon( proto, statusBar() );
	statusBar()->addWidget( i, 0, true );
	connect( i, SIGNAL( rightClicked( KopeteProtocol *, const QPoint & ) ),
		SLOT( slotProtocolStatusIconRightClicked( KopeteProtocol *,
		const QPoint & ) ) );

	m_statusBarIcons.insert( proto, i );

	slotProtocolStatusIconChanged( proto, proto->statusIcon() );

	KActionMenu *menu = proto->protocolActions();
	if( menu )
		menu->plug( tray->contextMenu(), 1 );
}

void KopeteWindow::slotProtocolDestroyed( QObject *o )
{
	kdDebug() << "KopeteWindow::slotProtocolDestroyed()" << endl;

	StatusBarIcon *i = static_cast<StatusBarIcon *>( m_statusBarIcons[ o ] );
	if( !i )
		return;

	delete i;
	m_statusBarIcons.remove( o );
}

void KopeteWindow::slotProtocolStatusIconChanged( KopeteProtocol * p,
	const QString &icon )
{
	kdDebug() << "KopeteWindow::slotProtocolStatusIconChanged()" << endl;

	StatusBarIcon *i = static_cast<StatusBarIcon *>( m_statusBarIcons[ p ] );
	if( !i )
		return;

	// Because we want null pixmaps to detect the need for a loadMovie
	// we can't use the SmallIcon() method directly
	KIconLoader *loader = KGlobal::instance()->iconLoader();
	QPixmap pm = loader->loadIcon( icon, KIcon::User, 0, KIcon::DefaultState, 0L,
		true );
	if( pm.isNull() )
	{
		QString path = loader->moviePath( icon, KIcon::User, 0 );
		if( path.isEmpty() )
		{
			kdDebug() << "KopeteWindow::slotProtocolStatusIconChanged(): "
				<< "Using unknown pixmap for status icon '" << icon << "'."
				<< endl;
			i->setPixmap( KIconLoader::unknown() );
		}
		else
		{
			kdDebug() << "KopeteWindow::slotProtocolStatusIconChanged(): "
				<< "Using movie: " << path << endl;
			i->setMovie( QMovie( path ) );
		}
	}
	else
	{
		i->setPixmap( pm );
	}
}

void KopeteWindow::slotProtocolStatusIconRightClicked( KopeteProtocol *proto,
	const QPoint &p )
{
	kdDebug() << "KopeteWindow::slotProtocolStatusIconRightClicked()" << endl;
	KActionMenu *menu = proto->protocolActions();
	if( menu )
		menu->popup( p );
}

#include "kopetewindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

