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

#include <kapp.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kpopupmenu.h>

#include "contactlist.h"
/** Kopete is the base class of the project */
class Kopete : public QWidget
{
  Q_OBJECT 
  public:
    /** construtor */
    Kopete(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~Kopete();
  /** No descriptions */
  void saveOptions();
  /** No descriptions */
  void readOptions();
	private:
	ContactList *contactlist;
	QPushButton *mainbutton;
	
	QPushButton *addbutton;
	QPushButton *awaybutton;
	QPushButton *globalbutton;
	QPushButton *otherbutton;
	
	KPopupMenu *popupmenu;
	KPopupMenu *icqpopupmenu;
	KPopupMenu *msnpopupmenu;
	QLabel *statuslabel;
	
	
public slots: // Public slots
  /** No descriptions */
  void slotICQConnectByMenu();
  /** No descriptions */
  void slotMSNConnectByMenu();
  /** No descriptions */
  void slotPreferences();
  /** No descriptions */
  void slotExit();
  /** No descriptions */
  void slotPrefDialogClosed();
};

#endif
