/***************************************************************************
                          kopete.h  -  description
                             -------------------
    begin                : Wed Dec 26 03:12:10 CLST 2001
    copyright            : (C) 2001 by Duncan Mac-Vicar Prett
    email                : duncan@puc.cl
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
#include "aboutplugins.h"
#include "kopetewindow.h"

#include "plugin.h"
#include "configmodule.h"
#include "pluginmodule.h"
#include "pluginloader.h"
#include "pluginmanager.h"

class LibraryLoader;
class KopeteLibraryInfo;

/** Kopete is the base class of the project */
class Kopete : public KUniqueApplication
{
  Q_OBJECT 
  public:
    Kopete();
    ~Kopete();
	
	PreferencesDialog *preferencesBox() const { return mPref; }
	KopeteWindow *popupMenu() const { /*return mainwidget->popupmenu;*/ };
	//PluginManager *Plugins() const { return plugins; }
	LibraryLoader *libraryLoader() const { return mLibraryLoader; }
	KIconLoader *iconLoader() const { return mIconLoader; }
	KopeteWindow *mainWindow() const { return mainwindow; };

	/** No descriptions */
	void saveOptions();
	/** No descriptions */
	void readOptions();
	/** No descriptions */
	void initPlugins();
	/** No descriptions */
	KStatusBar *statusBar();
  /** No descriptions */
  ContactList *contactList();
	/** No descriptions */
	PluginManager *plugins;
	PreferencesDialog *mPref;

	/** Some bool stuff */
	bool allConnected;
	private:
	KopeteWindow *mainwindow;
	LibraryLoader *mLibraryLoader;
	KIconLoader *mIconLoader;
	static void cleverKCrashHack(int);
	void loadPlugins();
	
public slots:
  void slotPreferences();
  void slotExit();
  void slotConnectAll();
  void slotDisconnectAll();	
  void slotAboutPlugins();
  void slotAddContact();
	void slotSetAway();
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif
