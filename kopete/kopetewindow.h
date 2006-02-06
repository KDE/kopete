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
#include <qlabel.h>

class QHBox;
class QTimer;
class QSignalMapper;

class QMouseEvent;
class QPoint;

class KAction;
class KActionMenu;

class KGlobalAccel;
class KSelectAction;
class KSqueezedTextLabel;
class KToggleAction;

class KopeteAccountStatusBarIcon;
class KopeteContactListView;
class KopetePluginConfig;
class KopeteSystemTray;
class KopeteEditGlobalIdentityWidget;

namespace Kopete
{
class AwayAction;
class Account;
class Contact;
class Plugin;
class Protocol;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteWindow : public KMainWindow
{
	Q_OBJECT

public:
	KopeteWindow ( QWidget *parent = 0, const char *name = 0 );
	~KopeteWindow();

	virtual bool eventFilter( QObject* o, QEvent* e );

protected:
	virtual void closeEvent( QCloseEvent *ev );
	virtual void leaveEvent( QEvent* ev );
	virtual void showEvent( QShowEvent* ev );

private slots:
	void showMenubar();
	void showStatusbar();
	void slotToggleShowOffliners();
	void slotToggleShowEmptyGroups();
	void slotConfigChanged();
	void slotConfNotifications();
	void slotConfToolbar();
	void slotUpdateToolbar();
	void slotConfigurePlugins();
	void slotConfGlobalKeys();
	void slotShowHide();
	void slotToggleAway();

	/* show the global status message selector menu
	 */
	void setStatusMessage( const QString & );
	
	/**
	 * Checks if the mousecursor is in the contact list.
	 * If not, the window will be hidden.
	 */
	void slotAutoHide();

	/**
	 * This slot will apply settings that change the
	 * contactlist's appearance. Only autohiding is
	 * handled here at the moment
	 */
	void slotContactListAppearanceChanged();

	/**
	 * This slot will set all the protocols to away
	 */
	void slotGlobalAway();
	void slotGlobalBusy();
	void slotGlobalAvailable();
	void slotSetInvisibleAll();
	void slotDisconnectAll();

	void slotQuit();

	/**
	 * Get a notification when a plugin is loaded, so we can merge
	 * XMLGUI cruft
	 */
	void slotPluginLoaded( Kopete::Plugin *p );

	/**
	 * Get a notification when an account is created, so we can add a status bar
	 * icon
	 */
	void slotAccountRegistered( Kopete::Account *a );

	/**
	 * Cleanup the status bar icon when the account is destroyed
	 */
	void slotAccountUnregistered( const Kopete::Account *a);

	/**
	 * The status icon got changed, update it.
	 * @param contact The account's contact that changed.
	 */
	void slotAccountStatusIconChanged( Kopete::Contact * contact);

	/**
	 * The status icon of some account changed. Must be sent by the account in question.
	 */
	void slotAccountStatusIconChanged();

	/**
	 * Show a context menu for a protocol
	 */
//	void slotProtocolStatusIconRightClicked( Kopete::Protocol *proto, const QPoint &p );

	/**
	 * Show a context menu for an account
	 */
	void slotAccountStatusIconRightClicked( Kopete::Account *a,
		const QPoint &p );

	void slotTrayAboutToShowMenu(KPopupMenu *);

	/**
	 * Show the Add Contact wizard
	 */
	void showAddContactDialog( Kopete::Account * );

	/**
	 * Show the Export Contacts wizards
	 */
	void showExportDialog();

	/**
	 * Enable the Connect All and Disconnect All buttons here
	 * along with connecting the accountRegistered and accountUnregistered
	 * signals.
	 */
	void slotAllPluginsLoaded();
	
	/**
	 * Protected slot to setup the Set Global Status Message menu.
	 */
	void slotBuildStatusMessageMenu();
	void slotStatusMessageSelected( int i );
	void slotNewStatusMessageEntered();

        /**
         * Show the set global status message menu when clicking on the icon in the status bar.
         */
        void slotGlobalStatusMessageIconClicked( const QPoint &position );

	/**
	 * Extracts protocolId and accountId from the single QString argument signalled by a QSignalMapper,
	 * get the account, and call showAddContactDialog.
	 * @param accountIdentifer QString of protocolId and accountId, concatenated with QChar( 0xE000 )
	 * We need both to uniquely identify an account, but QSignalMapper only emits one QString.
	 */
	void slotAddContactDialogInternal( const QString & accountIdentifier );
	
public:
	KopeteContactListView *contactlist;

	// Some Actions
	KActionMenu* actionAddContact;

	//KActionMenu* actionConnectionMenu;
	//KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionExportContacts;

	KActionMenu* actionAwayMenu;
	KActionMenu* actionDockMenu;
	KAction* selectAway;
	KAction* selectBusy;
	KAction* actionSetAvailable;
	KAction* actionSetInvisible;


	KAction* actionPrefs;
	KAction* actionQuit;
	KAction* actionSave;
	KToggleAction *menubarAction;
	KToggleAction *statusbarAction;
	KToggleAction *actionShowOffliners;
	KToggleAction *actionShowEmptyGroups;
	KGlobalAccel *globalAccel;

	KopeteEditGlobalIdentityWidget *editGlobalIdentityWidget;
private:
	void initView();
	void initActions();
	void initSystray();
	void loadOptions();
	void saveOptions();

	void makeTrayToolTip();
	void startAutoHideTimer();

	virtual bool queryClose();
	virtual bool queryExit();
private:
	int docked;
	bool hidden;
	int deskRight;
	QPoint position;
	QHBox *m_statusBarWidget;
	KopeteSystemTray *m_tray;
	bool m_autoHide;
	unsigned int m_autoHideTimeout;
	QTimer* m_autoHideTimer;
	QSignalMapper* addContactMapper;

	KopetePluginConfig *m_pluginConfig;

	/**
	 * This is really a dictionary of KopeteAccountStatusBarIcon objects, but
	 * QPtrDict requires a full class definition to be known to make
	 * that work. And since I don't want to include that whole file here,
	 * use QObject instead.
	 */
	QPtrDict<QObject> m_accountStatusBarIcons;
	KSqueezedTextLabel * m_globalStatusMessage;
	KPopupMenu * m_globalStatusMessageMenu;
	QLineEdit * m_newMessageEdit;
	QString m_globalStatusMessageStored;
};


class GlobalStatusMessageIconLabel : public QLabel
{
      Q_OBJECT
public:
      GlobalStatusMessageIconLabel(QWidget *parent = 0, const char *name = 0);

protected:
      void mouseReleaseEvent(QMouseEvent *event);

signals:
      void iconClicked(const QPoint &position);

};

#endif
// vim: set noet ts=4 sts=4 sw=4:
