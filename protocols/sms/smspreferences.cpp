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

#include <qlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpoint.h>

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

	service = 0L;
	configWidget = 0L;

	preferencesDialog->serviceName->insertItem("Spray");

	configVBox = new QGroupBox(this, "configVBox");
	configLayout = new QHBoxLayout(configVBox);
	configLayout->setAutoAdd(true);
    
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

	configWidget = service->configureWidget(configVBox);
	configWidget->show();
}

#include "smspreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
