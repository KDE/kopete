/***************************************************************************
                          yahooprefs.cpp  -  description
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org

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
#include "yahoodebug.h"
#include "yahooprefs.h"

// Kopete Includes

// QT Includes
#include <qwidget.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qcheckbox.h>

// KDE Includes
#include <kconfig.h>
#include <klineedit.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>


// Yahoo Preferences
YahooPreferences::YahooPreferences(const QString & pixmap,
					QObject * parent) : ConfigModule(i18n("Yahoo Plugin"), 
					i18n("Yahoo Protocol"), pixmap, parent)
{
	DEBUG(YDMETHOD, "YahooPreferences::YahooPreferences(" << pixmap << 
			", <parent>)");

	(new QVBoxLayout(this))->setAutoAdd(true);
	m_preferencesDialog = new dlgPreferences(this);
	m_preferencesDialog->show();

	KGlobal::config()->setGroup("Yahoo");
	m_preferencesDialog->mID->setText(KGlobal::config()->
			readEntry("UserID", "<your Yahoo ID>"));
	m_preferencesDialog->mPass->setText(KGlobal::config()->
			readEntry("Password", "<your Yahoo password>"));
	m_preferencesDialog->mServer->setText(KGlobal::config()->
			readEntry("Server", "cs.yahoo.com"));
	m_preferencesDialog->mPort->setValue(KGlobal::config()->
			readNumEntry("Port", 5050));
	m_preferencesDialog->mAutoConnect->setChecked(KGlobal::config()->
			readBoolEntry ("AutoConnect", false));
	m_preferencesDialog->mLogAll->setChecked(KGlobal::config()->
			readBoolEntry("LogAll", true));
}


// Destructor
YahooPreferences::~YahooPreferences()
{
	DEBUG(YDMETHOD, "YahooPreferences::~YahooPreferences()");
}


// Save preferences
void YahooPreferences::save()
{
	DEBUG(YDMETHOD, "YahooPreferences::save()");
    KConfig *config = KGlobal::config();
    config->setGroup("Yahoo");
    config->writeEntry("UserID", m_preferencesDialog->mID->text());
    config->writeEntry("Password", m_preferencesDialog->mPass->text());
    config->writeEntry("Server", m_preferencesDialog->mServer->text());
    config->writeEntry("Port", m_preferencesDialog->mPort->text());
    config->writeEntry("AutoConnect",
		       m_preferencesDialog->mAutoConnect->isChecked());
    config->writeEntry("LogAll", m_preferencesDialog->mLogAll->isChecked());
    config->sync();
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

