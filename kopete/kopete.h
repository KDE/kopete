/***************************************************************************
    kopete.cpp - Kopete Instant Messenger Main Class

    (C) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>

 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETE_H
#define KOPETE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kuniqueapplication.h>
#include <kmainwindow.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <kiconloader.h>

#include "preferencesdialog.h"
#include "kopetewindow.h"
#include "kopeteevent.h"
#include "kopetenotifier.h"

#include "plugin.h"
#include "configmodule.h"
#include "pluginmodule.h"
#include "pluginloader.h"

/* included so every plugon can access global appeareance-prefs */
#include "appearanceconfig.h"

class LibraryLoader;
class KopeteLibraryInfo;
class KopeteMessageManagerFactory;

struct KopeteEmoticons
{
	/* Happy */
	QString smile;
	QString wink;
	QString tongue;
	QString biggrin;

	/* Sad */
	QString unhappy;
	QString cry;
	/* Surprise */
	QString oh;
	/* Other */
	QString sleep;
};


/**
* Kopete is the main class.
* Almost all the common tasks you will need
* for plugin development are here.
*
* You can should Kopete API like this:
* 1- #include <kopete.h>
* 2- kopeteapp->someFunction();
**/

class Kopete : public KUniqueApplication
{
	Q_OBJECT
	
public:
	Kopete();
	~Kopete();

	/**
	* This is the preferences dialog, where Kopete's general
	* preferences are, and where plugins embedd its preferences.
	**/
	PreferencesDialog *preferencesBox() const { return mPref; }
	/**
	* Use it to access the appearance preferences module
	* preferences are, and where plugins embedd its preferences.
	**/
//	AppearanceConfig *appearance() const { return mAppearance; }
	/**
	* Use it to access Kopete's plugin loader.
	* You wouldnt need to use it from a plugin.
	**/
	KopeteMessageManagerFactory *sessionFactory() const { return mMessageManagerFactory; }
	/**
	* Use it to access Kopete's plugin loader.
	* You wouldnt need to use it from a plugin.
	**/
	LibraryLoader *libraryLoader() const { return mLibraryLoader; }
	/**
	* Use it to access Kopete's icon loader.
	* You wouldnt need to use it from a plugin.
	**/
	KIconLoader *iconLoader() const { return mIconLoader; }
	/**
	* Use it to access Kopete's Main App window.
	**/
	KopeteWindow *mainWindow() const { return m_mainWindow; }
	/**
	* Use it to access Kopete's Contact List.
	**/
	ContactList *contactList() const;
	/**
	* Use it to access the status bar
	* The plugins can dock status bar icons to
	* show informatin about the plugin itself
	**/
	KStatusBar *statusBar() const;
	/**
	* Use it to access Kopete's system tray
	**/
	KopeteSystemTray *systemTray() const;
	/**
	* Use it to parse emoticons in a text.
	* You dont need to use this for chat windows,
	* There is a special class that abstract a chat view
	* and uses emoticons parser.
	* This function will use the selected emoticon theme.
	**/
	QString parseEmoticons(QString);
	/**
	* Use it to parse HTML in text.
	* You dont need to use this for chat windows,
	* There is a special class that abstract a chat view
	* and uses HTML parser.
	**/
	QString parseHTML( QString message, bool parseURLs = true );

	void initEmoticons();

private:
	PreferencesDialog *mPref;
	Plugins *mPluginsModule;

	KopeteWindow *m_mainWindow;
	LibraryLoader *mLibraryLoader;
	KIconLoader *mIconLoader;
	KopeteEmoticons mEmoticons;
	AppearanceConfig *mAppearance;
	QString mEmoticonTheme;
	KopeteNotifier *mNotifier;
	KopeteMessageManagerFactory *mMessageManagerFactory;
	void loadPlugins();

public slots:
	/**
	* Only use notify event for system-wide messages
	* and things like online notification
	* Messages from specific contacts should use KopeteContact's
	* incomingEvent signal
	*/
	void notifyEvent( KopeteEvent *);
	/**
	* Cancel an event.
	**/
	void cancelEvent( KopeteEvent *);

	void slotPreferences();
//	void slotExit();
	void slotConnectAll();
	void slotDisconnectAll();
	void slotAddContact();
	void slotSetAwayAll(void);
	void slotSetAvailableAll(void);

signals:
	void signalSettingsChanged();

private slots:
	void slotMainWindowDestroyed();
	void initialize();
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif

// vim: set noet ts=4 sts=4 sw=4:

