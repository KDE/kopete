/*
    msnpreferences.cpp - MSN Preferences Widget

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
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

#ifndef MSNPREFERENCES_H
#define MSNPREFERENCES_H

#include <kcmodule.h>

class msnPrefsUI;

/**
 * @author Duncan Mac-Vicar Prett
 * @author Martijn Klingens
 */
class MSNPreferences : public KCModule
{
	Q_OBJECT

public:
	MSNPreferences( QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList() );

	virtual void save();
	virtual void load();

private slots:
	void slotSettingsDirty();

private:
	msnPrefsUI *m_preferencesDialog;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

