/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "smspreferences.h"

#include <klocale.h>
#include <qlayout.h>

SMSPreferences::SMSPreferences( const QString &pixmap, QObject *parent )
	: ConfigModule( i18n( "SMS Plugin" ), i18n( "Sending messages to cellphones" ), pixmap, parent )
{
	(new QBoxLayout(this, QBoxLayout::Down))->setAutoAdd(true);
	prefBase = new SMSPreferencesBase( 0L, this );

	connect (prefBase, SIGNAL(saved()), this, SIGNAL(saved()));
}

SMSPreferences::~SMSPreferences()
{
	delete prefBase;
}

void SMSPreferences::save()
{
	prefBase->save();
}

void SMSPreferences::reopen()
{
	prefBase->reopen();
}

#include "smspreferences.moc"


/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

