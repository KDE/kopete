/*
    ircpreferences.cpp - IRC Preferences

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircpreferences.h"

#include "ircprefs.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <qlayout.h>

IRCPreferences::IRCPreferences(const QString &pixmap,QObject *parent)
	: ConfigModule(i18n("IRC Plugin"),i18n("Internet Relay Chat Protocol"),pixmap,parent)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	preferencesDialog = new ircPrefsUI(this);
	KGlobal::config()->setGroup("IRC");

	QColor color(175, 8, 8);
}
IRCPreferences::~IRCPreferences()
{
}

void IRCPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("IRC");

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

