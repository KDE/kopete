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

#include <qlayout.h>

#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaction.h>

#include "kopete.h"
#include "kopeteballoon.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetecontactlistview.h"
#include "systemtray.h"

KopeteWindow::KopeteWindow( QWidget *parent, const char *name )
: KMainWindow( parent, name )
{
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
}

void KopeteWindow::initView ( void )
{
	contactlist = new KopeteContactListView(this);
	setCentralWidget(contactlist);

	connect( contactlist, SIGNAL(executed(QListViewItem *)), this, SLOT(slotExecuted(QListViewItem *)) );
}

void KopeteWindow::initActions ( void )
{
	actionAddContact = new KAction( i18n("&Add Contact..."),"bookmark_add", 0 ,
							kopeteapp, SLOT(slotAddContact()),
							actionCollection(), "AddContact" );

	actionConnect = new KAction( i18n("&Connect All"),"connect_creating", 0 ,
							kopeteapp, SLOT(slotConnectAll()),
							actionCollection(), "Connect" );

	actionDisconnect = new KAction( i18n("&Disconnect All"),"connect_no", 0 ,
							kopeteapp, SLOT(slotDisconnectAll()),
							actionCollection(), "Disconnect" );

	actionConnectionMenu = new KActionMenu( i18n("Connection"),"connect_established",
							actionCollection(), "Connection" );

	actionConnectionMenu->insert(actionConnect);
	actionConnectionMenu->insert(actionDisconnect);

	actionSetAway = new KAction( i18n("&Set Away Globally"), "kopeteaway", 0 ,
							kopeteapp, SLOT(slotSetAwayAll()),
							actionCollection(), "SetAway" );

	actionSetAvailable = new KAction( i18n("Set Availa&ble Globally"), "kopeteavailable", 0 ,
							kopeteapp, SLOT(slotSetAvailableAll()),
							actionCollection(), "SetAvailable" );

	actionAwayMenu = new KActionMenu( i18n("Status"),"kopetestatus",
							actionCollection(), "Status" );
	actionAwayMenu->setDelayed( false );
	actionAwayMenu->insert(actionSetAvailable);
	actionAwayMenu->insert(actionSetAway);

	actionShowTransfers = new KAction( i18n("Show &File Transfers"),"network", 0 ,
							kopeteapp, SLOT(slotShowTransfers()),
							actionCollection(), "ShowTransfers" );

	actionPrefs = KStdAction::preferences(kopeteapp, SLOT(slotPreferences()), actionCollection());

//	actionQuit = KStdAction::quit(kopeteapp, SLOT(slotExit()), actionCollection());
	KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

	toolbarAction = KStdAction::showToolbar(this, SLOT(showToolbar()), actionCollection());

	createGUI ( "kopeteui.rc" );
}

void KopeteWindow::initSystray ( void )
{
	tray = new KopeteSystemTray(this, "KopeteSystemTray");
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

#if KDE_VERSION >= 305
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

void KopeteWindow::slotExecuted( QListViewItem *item )
{
	KopeteContactViewItem *contactvi = dynamic_cast<KopeteContactViewItem *>(item);
	if ( contactvi )
		contactvi->contact()->execute();
}

void KopeteWindow::closeEvent( QCloseEvent *e )
{
	if(isClosing)
	{
		KMainWindow::closeEvent( e );
		return;
	}
	
#if KDE_VERSION >= 305
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
	kdDebug() << "KopeteWindow::slotQuit()" <<endl;

	//I don't know why, but when this slot is called by the toolbar, that work fine
	// but when this slot is called by the system try, the closeEvent's message is showed
	isClosing=true;
	
	kopeteapp->quit();
}

#include "kopetewindow.moc"

// vim: set noet ts=4 sts=4 sw=4:

