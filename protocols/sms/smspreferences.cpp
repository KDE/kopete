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

#include <qlayout.h>
#include <qcombobox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

SMSPreferences::SMSPreferences( const QString &pixmap, QObject *parent )
: ConfigModule( i18n( "SMS Plugin" ), i18n( "Sending messages to cellphones" ),
	pixmap, parent )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new smsPrefsUI(this);
	reopen();
}

SMSPreferences::~SMSPreferences()
{
}

void SMSPreferences::reopen()
{
	KGlobal::config()->setGroup("SMS");
	preferencesDialog->serviceName->setCurrentText(
		KGlobal::config()->readEntry( "ServiceName", QString::null ) );
                              
}


void SMSPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("SMS");
	config->writeEntry("ServiceName", preferencesDialog->serviceName->currentText());
	config->sync();
	emit saved();

}

#include "smspreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
