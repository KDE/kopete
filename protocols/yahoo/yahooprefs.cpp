/*
    yahooeditaccount.cpp - UI Page to edit a Yahoo account

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
    Copyright (c) 2002 by Bruno Rodrigues <bruno.rodrigues@litux.org>

    Portions based on code by Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


// Local Includes
#include "yahooprefs.h"

// Kopete Includes

// QT Includes
#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>


// Yahoo Preferences
YahooPreferences::YahooPreferences(const QString &pixmap, QObject *parent)
	: ConfigModule(i18n("Yahoo Plugin"), i18n("Yahoo Protocol"), pixmap, parent)
{
	kdDebug(14180) << k_funcinfo << endl;

	(new QVBoxLayout(this))->setAutoAdd(true);
	m_yahooPrefsDialog = new dlgPreferences(this);
	m_yahooPrefsDialog->show();

	m_config = KGlobal::config();

	m_config->setGroup("Yahoo");
	m_yahooPrefsDialog->mServer->setText(m_config->readEntry("Server", "scs.yahoo.com"));
	m_yahooPrefsDialog->mPort->setValue(m_config->readNumEntry("Port", 5050));
	m_yahooPrefsDialog->mImportContacts->setChecked(m_config->readBoolEntry("ImportContacts", true));
	m_yahooPrefsDialog->mUseGroupNames->setChecked(m_config->readBoolEntry("UseGroupNames", false));
}


// Destructor
YahooPreferences::~YahooPreferences()
{
	kdDebug(14180) << k_funcinfo << endl;
}


// Save preferences
void YahooPreferences::save()
{
	kdDebug(14180) << k_funcinfo << endl;

	m_config->setGroup("Yahoo");
	m_config->writeEntry("Server", m_yahooPrefsDialog->mServer->text());
	m_config->writeEntry("Port", m_yahooPrefsDialog->mPort->text());
	m_config->writeEntry("ImportContacts", m_yahooPrefsDialog->mImportContacts->isChecked());
	m_config->writeEntry("UseGroupNames", m_yahooPrefsDialog->mUseGroupNames->isChecked());
	m_config->sync();
	emit saved();
}

#include "yahooprefs.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

