/***************************************************************************
                          preferencesdialog.h  -  description
                             -------------------
    begin                : Wed Dec 26 2001
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <qwidget.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "DlgPrefs.h"
/**
  *@author Duncan Mac-Vicar Prett
  */

class PreferencesDialog : public DlgPrefs  {
   Q_OBJECT
public:
	PreferencesDialog(QWidget *parent=0, const char *name=0);
	~PreferencesDialog();
};

#endif
