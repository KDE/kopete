/*
    preferencesdialog.cpp  -  Kopete Setup Dialog

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

#include "preferencesdialog.h"

#include <qapplication.h>

#include <klocale.h>

#include "configmodule.h"

PreferencesDialog* PreferencesDialog::s_preferencesDialog = 0L;

PreferencesDialog* PreferencesDialog::preferencesDialog()
{
	if( !s_preferencesDialog )
		s_preferencesDialog = new PreferencesDialog;

	return s_preferencesDialog;
}

PreferencesDialog::PreferencesDialog()
: KDialogBase( IconList, i18n( "Preferences" ), Ok | Apply | Cancel,
	Ok, qApp->mainWidget(), 0, false )
{
//	resize(640, 480); // KDE is required to support 800x600 min.
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

#include "preferencesdialog.moc"

// vim: set noet ts=4 sts=4 sw=4:

