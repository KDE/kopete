/*
    kopete.h

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

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

#ifndef KOPETE_H
#define KOPETE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kuniqueapplication.h>
#include "kopetemessage.h"

class KIconLoader;
class KStatusBar;

class AppearanceConfig;
class KopeteContactListView;
class KopeteEvent;
class KopeteMessageManagerFactory;
class KopeteNotifier;
class KopeteSystemTray;
class KopeteTransferManager;
class KopeteUserPreferencesConfig;
class KopeteWindow;
class LibraryLoader;
class Plugins;
class PreferencesDialog;

/*
class AppearanceConfig;
class KopeteLibraryInfo;
class KopeteMessageManagerFactory;
class KopeteMetaContact;
class KopeteTransferManager;
class KopeteUserPreferencesConfig;
*/

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 *
 * Kopete is the main class.
 * Almost all the common tasks you will need
 * for plugin development are here.
 *
 * You can should Kopete API like this:
 * 1- #include <kopete.h>
 * 2- kopeteapp->someFunction();
 */
class Kopete : public KUniqueApplication
{
	Q_OBJECT

public:
	Kopete();
	~Kopete();

	/**
	 * This is the preferences dialog, where Kopete's general
	 * preferences are, and where plugins embedd its preferences.
	 */
	PreferencesDialog *preferencesBox() const
	{ return mPref; }

	/**
	 * Use it to access Kopete's plugin loader.
	 * You shouldn't need to use it from a plugin.
	 */
	KopeteMessageManagerFactory *sessionFactory() const
	{ return mMessageManagerFactory; }

	/**
	 * Use it to access Kopete's plugin loader.
	 * You shouldn't need to use it from a plugin.
	 */
	LibraryLoader *libraryLoader() const
	{ return mLibraryLoader; }

	/**
	 * Use it to access Kopete's icon loader.
	 * You shouldn't need to use it from a plugin.
	 */
	KIconLoader *iconLoader() const
	{ return mIconLoader; }

	/**
	 * Use it to access Kopete's Contact List.
	 */
	KopeteContactListView *contactList() const;

	/**
	 * Use it to access the status bar
	 * The plugins can dock status bar icons to
	 * show informatin about the plugin itself
	 */
	KStatusBar *statusBar() const;

	/**
	 * Use it to access Kopete's system tray
	 */
	KopeteSystemTray *systemTray() const;

	/**
	 * Like slotSetAwayAll, but don't pops up the dialog
	 * (for the autowayplugin)
	 */
	void setAwayAll( void );

	/**
	 * This returns the active transferview window, if any.
	 */
	KopeteTransferManager *transferManager()
	{ return mTransferManager; };

public slots:
	/**
	 * Only use notify event for system-wide messages
	 * and things like online notification
	 * Messages from specific contacts should use KopeteContact's
	 * incomingEvent signal
	 */
	void notifyEvent( KopeteEvent * );

	/**
	 * Cancel an event.
	 */
	void cancelEvent( KopeteEvent * );

	void slotPreferences();
//	void slotExit();
	void slotConnectAll();
	void slotDisconnectAll();
	void slotAddContact();
	void slotSetAwayAll(void);
	void slotSetAvailableAll(void);
	void slotShowTransfers();

signals:
	/**
	 * This signal is emitted whenever a message
	 * is about to be displayed by the KopeteChatWindow.
	 * Please remember that both messages sent and
	 * messages received will emit this signal!
	 * Plugins may connect to this signal to change
	 * the message contents before it's going to be displayed.
	 */
	void aboutToDisplay( KopeteMessage& );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being sent.
	 */
	void aboutToSend( KopeteMessage& );

	void signalSettingsChanged();

private slots:
	void slotMainWindowDestroyed();
	void initialize();

	/**
	 * Load all plugins
	 */
	void slotLoadPlugins();

private:
	PreferencesDialog *mPref;
	Plugins *mPluginsModule;

	KopeteWindow *m_mainWindow;
	LibraryLoader *mLibraryLoader;
	KIconLoader *mIconLoader;
	AppearanceConfig *mAppearance;

	//User preferences config module
	KopeteUserPreferencesConfig *mUserPreferencesConfig;
	QString mEmoticonTheme;
	KopeteNotifier *mNotifier;
	KopeteMessageManagerFactory *mMessageManagerFactory;
	KopeteTransferManager *mTransferManager;
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif

// vim: set noet ts=4 sts=4 sw=4:

