/*
    kopetewindow.h  -  Kopete Main Window

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

#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <qptrdict.h>

#include <kmainwindow.h>

class QLabel;
class QListViewItem;

class KAction;
class KActionMenu;
class KSelectAction;
class KToggleAction;

class KopeteContactListView;
class KopetePlugin;
class KopeteProtocol;
class KopeteSystemTray;
class KopeteAwayDialog;
class StatusBarIcon;

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
	void showToolbar();
	void showMenubar();
	void showStatusbar();
	void slotToggleShowOffliners();
	void slotConfigChanged();
	void slotConfKeys();
	void slotConfToolbar();
	/**
	 * This slot will show an away dialog and then
	 * set all the protocols to away
	 */
	void slotGlobalAwayMessageSelect();
	void slotQuit();

	/**
	 * Get a notification when a plugin is loaded, so we can add a status bar
	 * icon if it's a protocol
	 */
	void slotPluginLoaded( KopetePlugin *p );

	/**
	 * Cleanup the status bar icon when the plugin is destroyed
	 */
	void slotProtocolDestroyed( QObject *o );

	/**
	 * The status icon got changed, update it.
	 * Note that iconName can also be a .mng movie instead of an icon.
	 */
	void slotProtocolStatusIconChanged( KopeteProtocol *proto,
		const QString &iconName );

	/**
	 * Show a context icon for a protocol
	 */
	void slotProtocolStatusIconRightClicked( KopeteProtocol *proto,
		const QPoint &p );

	/**
	 * Show the Add Contact wizard
	 */
	void showAddContactDialog();

public:
	KopeteContactListView *contactlist;

	// Some Actions
	KAction* actionAddContact;

	KActionMenu* actionConnectionMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;

	KActionMenu* actionAwayMenu;
	KAction* actionSetAway;
	KAction* actionSetAvailable;

	KAction* actionPrefs;
	KAction* actionQuit;
	KAction* actionSave;
	KToggleAction *toolbarAction;
	KToggleAction *menubarAction;
	KToggleAction *statusbarAction;
	KToggleAction *actionShowOffliners;
	KAction *actionShowTransfers;

	KopeteSystemTray *tray;

private:
	void initView ( void );
	void initActions ( void );
	void initSystray ( void );
	void loadOptions(void);
	void saveOptions(void);

	bool isClosing;

	/**
	 * This is really a dictionary of StatusBarIcon objects, but
	 * QPtrDict requires a full class definition to be known to make
	 * that work. And since I don't want to include that whole file here,
	 * use QObject instead.
	 */
	QPtrDict<QObject> m_statusBarIcons;

	/**
	 * This is the away message selection dialog
	 */
	KopeteAwayDialog *m_awayMessageDialog;
	
private slots:
	/**
	 * Show the prefs dialog. See also the source for a description
	 * why this is needed.
	 */
	void slotShowPreferencesDialog();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

