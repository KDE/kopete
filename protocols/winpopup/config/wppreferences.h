/***************************************************************************
                          wppreferences.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WPPREFERENCES_H
#define __WPPREFERENCES_H

// QT Includes

// KDE Includes
#include <kgenericfactory.h>
#include "kcautoconfigmodule.h"

// Kopete Includes

// Local Includes
#include "wppreferencesbase.h"

class WPPreferences : public KCAutoConfigModule
{
	Q_OBJECT
private:
	WPPreferencesBase *preferencesDialog;	// Preferences Dialog

public:
	WPPreferences(QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList());
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

