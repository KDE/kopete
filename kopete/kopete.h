/***************************************************************************
                          Kopete Instant Messenger
					             kopete.h
                             -------------------
				(C) 2001-2002 by Duncan Mac-Vicar P. <duncan@kde.org>
 ***************************************************************************/

/***************************************************************************
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

#include "plugin.h"
#include "configmodule.h"
#include "pluginmodule.h"
#include "pluginloader.h"

class AppearanceConfig;
class LibraryLoader;
class KopeteLibraryInfo;

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


/** Kopete is the base class of the project */
class Kopete : public KUniqueApplication
{
  Q_OBJECT
  public:
	Kopete();
	~Kopete();

	PreferencesDialog *preferencesBox() const { return mPref; }
	LibraryLoader *libraryLoader() const { return mLibraryLoader; }
	KIconLoader *iconLoader() const { return mIconLoader; }
	KopeteWindow *mainWindow() const { return mainwindow; };
	ContactList *contactList() const { return mainwindow->contactlist; };
	KStatusBar *statusBar() const { return mainwindow->statusBar(); };
	KopeteSystemTray *systemTray() const {return mainwindow->tray; };

	QString parseEmoticons(QString);
	QString parseHTML( QString message, bool parseURLs = true );
	void initEmoticons();

private:

	PreferencesDialog *mPref;
	Plugins *mPluginsModule;

	KopeteWindow *mainwindow;
	LibraryLoader *mLibraryLoader;
	KIconLoader *mIconLoader;
	KopeteEmoticons mEmoticons;
	AppearanceConfig *mAppearance;
	QString mEmoticonTheme;

	void loadPlugins();

public slots:
	void slotPreferences();
	void slotExit();
	void slotConnectAll();
	void slotDisconnectAll();
	void slotAddContact();
	void slotSetAway();
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif
