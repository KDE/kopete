/***************************************************************************
                          wppreferences.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Local Includes
#include "wppreferences.h"
#include "wpdebug.h"
#include "wpprotocol.h"

// Kopete Includes

// QT Includes
#include <qlayout.h>
#include <qslider.h>

// KDE Includes
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurlrequester.h>

// WinPopup Preferences
WPPreferences::WPPreferences(const QString & pixmap,
					QObject * parent) : ConfigModule(i18n("WinPopup Plugin"),
					i18n("WinPopup Protocol"), pixmap, parent)
{
	DEBUG(WPDMETHOD, "WPPreferences::WPPreferences(" << pixmap << ", <parent>)");

	theProtocol = dynamic_cast<WPProtocol *>(parent);
	(new QVBoxLayout(this))->setAutoAdd(true);
	preferencesDialog = new WPPreferencesBase(this);
	preferencesDialog->mSMBClientPath->setFilter(i18n("smbclient|Samba Client Binary\n*|All Files"));
	preferencesDialog->show();

	KGlobal::config()->setGroup("WinPopup");
	preferencesDialog->mSMBClientPath->setURL(KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient"));
	preferencesDialog->mInitialSearchHost->setText(KGlobal::config()->readEntry("InitialSearchHost", "127.0.0.1"));
	preferencesDialog->mHostCheckFrequency->setValue(KGlobal::config()->readNumEntry("HostCheckFrequency", 60));
	preferencesDialog->mMessageCheckFrequency->setValue(KGlobal::config()->readNumEntry("MessageCheckFrequency", 5));
}

// Destructor
WPPreferences::~WPPreferences()
{
	DEBUG(WPDMETHOD, "WPPreferences::~WPPreferences()");
}

void WPPreferences::installSamba()
{
	DEBUG(WPDMETHOD, "WPPreferences::installSamba()");
	theProtocol->installSamba();
}

// Save preferences
void WPPreferences::save()
{
	DEBUG(WPDMETHOD, "WinPopupPreferences::save()");

	KGlobal::config()->setGroup("WinPopup");
	KGlobal::config()->writeEntry("InitialSearchHost", preferencesDialog->mInitialSearchHost->text());
	KGlobal::config()->writeEntry("SMBClientPath", preferencesDialog->mSMBClientPath->url());
	KGlobal::config()->writeEntry("HostCheckFrequency", preferencesDialog->mHostCheckFrequency->value());
	KGlobal::config()->writeEntry("MessageCheckFrequency", preferencesDialog->mMessageCheckFrequency->value());
	KGlobal::config()->sync();
	emit saved();
}

#include "wppreferences.moc"

