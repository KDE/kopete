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
#include "smsservice.h"
#include "serviceloader.h"
#include "smsprefs.h"

#include <qlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpoint.h>
#include <qsizepolicy.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

SMSPreferences::SMSPreferences( const QString &pixmap, QObject *parent )
: ConfigModule( i18n( "SMS Plugin" ), i18n( "Sending messages to cellphones" ),
	pixmap, parent )
{
	(new QVBoxLayout(this, QBoxLayout::Down))->setAutoAdd(true);

	preferencesDialog = new smsPrefsUI(this);

	service = 0L;
	configWidget = 0L;

	preferencesDialog->serviceName->insertItem("SMSSend");

	connect (preferencesDialog->serviceName, SIGNAL(activated(const QString &)), this, SLOT(setServicePreferences(const QString &)));

	reopen();
}

SMSPreferences::~SMSPreferences()
{
	if (service != 0L)
		delete service;
}

void SMSPreferences::reopen()
{
	KGlobal::config()->setGroup("SMS");
	QString sName = KGlobal::config()->readEntry( "ServiceName", QString::null );
	
	for (int i=0; i < preferencesDialog->serviceName->count(); i++)
	{
		if (preferencesDialog->serviceName->text(i) == sName)
		{
			preferencesDialog->serviceName->setCurrentItem(i);
			break;
		}
	}
	setServicePreferences(preferencesDialog->serviceName->currentText());
}


void SMSPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("SMS");
	config->writeEntry("ServiceName", preferencesDialog->serviceName->currentText());
	config->sync();

	if ( service != 0L && configWidget != 0L )
		service->savePreferences();
	
	emit saved();

}

void SMSPreferences::setServicePreferences(const QString& name)
{
	if (service != 0L)
	{
		service->savePreferences();
		delete service;
	}

	if (configWidget != 0L)
		delete configWidget;
	
	service = ServiceLoader::loadService(name);

	if ( service == 0L)
		return;

	configWidget = service->configureWidget(this);
	configWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	configWidget->show();
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

