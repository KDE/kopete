/*
    kopetewindow.h  -  Kopete Main Window

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2001-2002 by Stefan Gehn <metz AT gehn.net>

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

#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <qptrdict.h>

#include <kmainwindow.h>

class QLabel;
class QHBox;
class QListViewItem;
class KGlobalAccel;
class KAction;
class KActionMenu;
class KSelectAction;
class KToggleAction;

class KopeteContactListView;
class KopetePlugin;
class KopeteProtocol;
class KopeteAccount;
class KopeteContact;
class KopeteSystemTray;
class KopeteGlobalAwayDialog;
class KopeteAccountStatusBarIcon;
class KopeteProtocolStatusBarIcon;
class KopeteOnlineStatus;

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
	virtual bool queryExit();

private slots:
//	void showToolbar();
	void showMenubar();
	void showStatusbar();
	void slotToggleShowOffliners();
	void slotConfigChanged();
	void slotConfKeys();
	void slotConfToolbar();
    void slotUpdateToolbar();
	/**
	 * This slot will show an away dialog and then
	 * set all the protocols to away
	 */
	void slotGlobalAwayMessageSelect();
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
	 * Note that iconName can also be a .mng movie instead of an icon.
	 */
	// TODO: Is this obsolete at 20030710? I can't see a call to it anywhere - Will
	void slotProtocolStatusIconChanged( const KopeteOnlineStatus& status );

	/**
	 * The status icon got changed, update it.
	 * @param contact The account's contact that changed.
	 */
	void slotAccountStatusIconChanged( KopeteContact * contact);

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
	 * Carry out any changes resulting from a change to preferences
	 */
	void slotSettingsChanged();

public:
	KopeteContactListView *contactlist;

	// Some Actions
	KAction* actionAddContact;

	KActionMenu* actionConnectionMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;

	KActionMenu* actionAwayMenu;
	KActionMenu* actionDockMenu;
	KAction* actionSetAway;
	KAction* actionSetAvailable;

	KAction* actionPrefs;
	KAction* actionQuit;
	KAction* actionSave;
//	KToggleAction *toolbarAction;
	KToggleAction *menubarAction;
	KToggleAction *statusbarAction;
	KToggleAction *actionShowOffliners;
//	KAction *actionShowTransfers;
	KGlobalAccel *globalAccel;

	KopeteSystemTray *tray;

private:
	void initView();
	void initActions();
	void initSystray();
	void loadOptions();
	void saveOptions();
	int docked;
	bool hidden;
	int deskRight;
	QPoint position;
	QHBox *m_statusBarWidget;

	/**
	 * This is really a dictionary of KopeteAccountStatusBarIcon objects, but
	 * QPtrDict requires a full class definition to be known to make
	 * that work. And since I don't want to include that whole file here,
	 * use QObject instead.
	 */
	QPtrDict<QObject> m_accountStatusBarIcons;
	/**
	 * This is really a dictionary of KopeteProtocolStatusBarIcon objects, but
	 * QPtrDict requires a full class definition to be known to make
	 * that work. And since I don't want to include that whole file here,
	 * use QObject instead.
	 */
	QPtrDict<QObject> m_protocolStatusBarIcons;

	/**
	 * This is the away message selection dialog
	 */
	KopeteGlobalAwayDialog *m_awayMessageDialog;

private slots:
	/**
	 * Show the prefs dialog. See also the source for a description
	 * why this is needed.
	 */
	void slotShowPreferencesDialog();
//	void slotAddAccount();
	void slotSaveContactList();
	void slotConfGlobalKeys();
	void slotShowHide();
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
