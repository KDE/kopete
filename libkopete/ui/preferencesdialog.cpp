/***************************************************************************
                          preferencesdialog.cpp  -  description
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

#include <klocale.h>
#include "preferencesdialog.h"
#include "configmodule.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
								: KDialogBase(IconList, i18n("Preferences"),
                  Ok|Apply|Close, Ok, parent, 0, false)
{
		resize(640, 480); // KDE is required to support 800x600 min.
}

PreferencesDialog::~PreferencesDialog()
{

}
void PreferencesDialog::show()
{
	for (ConfigModule *i=mModules.first(); i != 0; i=mModules.next())
		i->reopen();
	KDialogBase::show();
}

void PreferencesDialog::slotApply()
{
	for (ConfigModule *i=mModules.first(); i != 0; i=mModules.next())
		i->save();
}

void PreferencesDialog::slotOk()
{
		slotApply();
		hide();
}
void PreferencesDialog::add(ConfigModule *page)
{
		mModules.append(page);
}
void PreferencesDialog::remove(ConfigModule *page)
{
		mModules.removeRef(page);
}

