/***************************************************************************
                          wppreferences.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2003 by Gav Wood
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

// QT Includes

// KDE Includes
#include <klocale.h>
#include <kurlrequester.h>
#include <kgenericfactory.h>
#include <kcautoconfigmodule.h>

// Kopete Includes

// Local Includes
#include "wppreferences.h"

typedef KGenericFactory<WPPreferences> WPProtocolConfigFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kopete_wp, WPProtocolConfigFactory("kcm_kopete_wp"))

// WinPopup Preferences
WPPreferences::WPPreferences(QWidget *parent, const char *, const QStringList &args) : KCAutoConfigModule(WPProtocolConfigFactory::instance(), parent, args)
{
	preferencesDialog = new WPPreferencesBase(this);
	preferencesDialog->SMBClientPath->setFilter(i18n("smbclient|Samba Client Binary\n*|All Files"));
	setMainWidget(preferencesDialog, "WinPopup");
}
