/***************************************************************************
                          ircpreferences.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "ircpreferences.h"

#include "ircprefs.h"

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>

IRCPreferences::IRCPreferences(const QString &pixmap,QObject *parent)
	: ConfigModule(i18n("IRC Plugin"),i18n("Internet Relay Chat Protocol"),pixmap,parent)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	preferencesDialog = new ircPrefsUI(this);
	KGlobal::config()->setGroup("IRC");
	preferencesDialog->mID->setText(KGlobal::config()->readEntry("Nickname", "KopeteUser"));
	preferencesDialog->mServer->setText(KGlobal::config()->readEntry("Server", "irc.freenode.net"));
	preferencesDialog->mPort->setText(KGlobal::config()->readEntry("Port", "6667"));
	preferencesDialog->mAutoConnect->setChecked(KGlobal::config()->readBoolEntry("AutoConnect", false));
	QColor color(175, 8, 8);
}
IRCPreferences::~IRCPreferences()
{
}

void IRCPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("IRC");
	config->writeEntry("Nickname", preferencesDialog->mID->text());
	config->writeEntry("Server", preferencesDialog->mServer->text());
	config->writeEntry("Port", preferencesDialog->mPort->text());
	config->writeEntry("AutoConnect", preferencesDialog->mAutoConnect->isChecked());

	config->sync();
	emit saved();
}

#include "ircpreferences.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

