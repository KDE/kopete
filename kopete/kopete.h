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


#include "contactlist.h"
#include "pluginmanager.h"
#include "ui/preferencesdialog.h"
#include "ui/aboutplugins.h"

/** Kopete is the base class of the project */
class Kopete : public KUniqueApplication
{
  Q_OBJECT 
  public:
    Kopete();
    ~Kopete();
	
	PreferencesDialog *preferencesBox() const { return mPref; }
  KPopupMenu *popupMenu() const { return popupmenu; };
	PluginManager *Plugins() const { return plugins; }
	
	/** No descriptions */
  void saveOptions();
  /** No descriptions */
  void readOptions();

	PluginManager *plugins;
	PreferencesDialog *mPref;
	AboutPlugins *aboutPl;
	private:
	QWidget *mainwidget;
	ContactList *contactlist;
	QPushButton *mainbutton;
	
	QPushButton *addbutton;
	QPushButton *awaybutton;
	QPushButton *globalbutton;
	QPushButton *otherbutton;
	
	KPopupMenu *popupmenu;
	QLabel *statuslabel;
	
	
public slots:
  /** No descriptions */
  void slotPreferences();
  /** No descriptions */
  void slotExit();
  /** No descriptions */
  void slotPrefDialogClosed();
  /** No descriptions */
  void slotConnectAll();
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif
