/*
    kopetewindow.h  -  Kopete Main Window

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <qptrdict.h>

#include <kmainwindow.h>

class QHBox;

class KAction;
class KActionMenu;

class KGlobalAccel;
class KSelectAction;
class KToggleAction;

class KopeteAccount;
class KopeteAccountStatusBarIcon;
class KopeteContact;
class KopeteContactListView;
class KopeteAwayAction;
class KopetePlugin;
class KopetePluginConfig;
class KopeteProtocol;
class KopeteSystemTray;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteWindow : public KMainWindow
{
	Q_OBJECT

public:
	KopeteWindow ( QWidget *parent = 0, const char *name = 0 );
	~KopeteWindow();

protected:
	virtual void closeEvent( QCloseEvent *ev );

private slots:
	void showMenubar();
	void showStatusbar();
	void slotToggleShowOffliners();
	void slotToggleShowEmptyGroups();
	void slotConfigChanged();
	void slotConfKeys();
	void slotConfNotifications();
	void slotConfToolbar();
	void slotUpdateToolbar();
	void slotConfigurePlugins();
	void slotConfGlobalKeys();
	void slotShowHide();

	/**
	 * This slot will show an away dialog and then
	 * set all the protocols to away
	 */
	void slotGlobalAwayMessageSelect( const QString & );
	void slotQuit();

	/**
	 * Get a notification when a plugin is loaded, so we can merge
	 * XMLGUI cruft
	 */
	void slotPluginLoaded( KopetePlugin *p );

	/**
	 * Get a notification when an account is created, so we can add a status bar
	 * icon
	 */
	void slotAccountRegistered( KopeteAccount *a );

	/**
	 * Cleanup the status bar icon when the account is destroyed
	 */
	void slotAccountUnregistered( KopeteAccount *a);

	/**
	 * The status icon got changed, update it.
	 * @param contact The account's contact that changed.
	 */
	void slotAccountStatusIconChanged( KopeteContact * contact);

	/**
	 * The status icon of some account changed. Must be sent by the account in question.
	 */
	void slotAccountStatusIconChanged();

	/**
	 * Show a context menu for a protocol
	 */
	void slotProtocolStatusIconRightClicked( KopeteProtocol *proto,
		const QPoint &p );

	/**
	 * Show a context menu for an account
	 */
	void slotAccountStatusIconRightClicked( KopeteAccount *a,
		const QPoint &p );

	void slotTrayAboutToShowMenu(KPopupMenu *);

	/**
	 * Show the Add Contact wizard
	 */
	void showAddContactDialog();

	/**
	 * Enable the Connect All and Disconnect All buttons here
	 * along with connecting the accountRegistered and accountUnregistered
	 * signals.
	 */
	void slotAllPluginsLoaded();

public:
	KopeteContactListView *contactlist;

	// Some Actions
	KAction* actionAddContact;

	KActionMenu* actionConnectionMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;

	KActionMenu* actionAwayMenu;
	KActionMenu* actionDockMenu;
	KopeteAwayAction* selectAway;
	KAction* actionSetAvailable;

	KAction* actionPrefs;
	KAction* actionQuit;
	KAction* actionSave;
	KToggleAction *menubarAction;
	KToggleAction *statusbarAction;
	KToggleAction *actionShowOffliners;
	KToggleAction *actionShowEmptyGroups;
	KGlobalAccel *globalAccel;

private:
	void initView();
	void initActions();
	void initSystray();
	void loadOptions();
	void saveOptions();
	
	void makeTrayToolTip();

private:
	int docked;
	bool hidden;
	int deskRight;
	QPoint position;
	QHBox *m_statusBarWidget;
	KopeteSystemTray *m_tray;

	KopetePluginConfig *m_pluginConfig;

	/**
	 * This is really a dictionary of KopeteAccountStatusBarIcon objects, but
	 * QPtrDict requires a full class definition to be known to make
	 * that work. And since I don't want to include that whole file here,
	 * use QObject instead.
	 */
	QPtrDict<QObject> m_accountStatusBarIcons;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
