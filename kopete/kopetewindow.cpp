/***************************************************************************
                          Kopete Instant Messenger
							 kopetewindows.cpp
                            -------------------
				(C) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
				(C) 2001-2002 by Stefan Gehn <sgehn@gmx.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetewindow.h"
#include "kopetewindow.moc"

#include "contactlist.h"
#include "imcontact.h"
#include "kopete.h"
#include "kopeteballoon.h"
#include "systemtray.h"

#include <qlayout.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstdaction.h>

KopeteWindow::KopeteWindow(QWidget *parent, const char *name ): KMainWindow(parent,name)
{
	kdDebug() << "[KopeteWindow] KopeteWindow()" << endl;

	/* -------------------------------------------------------------------------------- */
	initView();
	initActions();
	initSystray();
	/* -------------------------------------------------------------------------------- */

	loadOptions();

	// we always show our statusbar
	// it's important because it shows the protocol-icons
	statusBar()->show();
	
	// for now systemtray is also always shown
	// TODO: make the configurable
	tray->show();
}

void KopeteWindow::initView ( void )
{
	mainwidget = new QWidget(this);
	setCentralWidget(mainwidget);

	QBoxLayout *layout = new QBoxLayout(mainwidget,QBoxLayout::TopToBottom);
	contactlist = new ContactList(mainwidget);

	layout->insertWidget ( -1, contactlist );
	connect( contactlist, SIGNAL(executed(QListViewItem *)), this, SLOT(slotExecuted(QListViewItem *)) );
}

void KopeteWindow::initActions ( void )
{
	actionAddContact = new KAction( i18n("&Add contact"),"bookmark_add", 0 ,
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

	actionSetAway = new KAction( i18n("&Set away globally"), "away", 0 ,
							kopeteapp, SLOT(slotSetAwayAll()),
							actionCollection(), "SetAway" );

	actionSetAvailable = new KAction( i18n("Set availa&ble globally"), "available", 0 ,
							kopeteapp, SLOT(slotSetAvailableAll()),
							actionCollection(), "SetAvailable" );

	actionAwayMenu = new KActionMenu( i18n("Status"),"awaymenu",
							actionCollection(), "Status" );
	actionAwayMenu->insert(actionSetAvailable);
	actionAwayMenu->insert(actionSetAway);

	actionPrefs = KStdAction::preferences(kopeteapp, SLOT(slotPreferences()), actionCollection());

//	actionQuit = KStdAction::quit(kopeteapp, SLOT(slotExit()), actionCollection());
	KStdAction::quit(kopeteapp, SLOT(quit()), actionCollection());

	toolbarAction = KStdAction::showToolbar(this, SLOT(showToolbar()), actionCollection());

	createGUI ( "kopeteui.rc" );
}

void KopeteWindow::initSystray ( void )
{
	tray = new KopeteSystemTray(this, "KopeteSystemTray");
	KPopupMenu *tm = tray->getContextMenu();

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
	kopeteapp->unloadPlugins(); // workaround to hopefully quit gracefully
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
	IMContact *contact = dynamic_cast<IMContact *>(item);
	if ( contact )
		contact->doubleClicked();
}

// vim: set noet sw=4 ts=4 sts=4:

