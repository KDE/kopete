/*
    webpresencepreferences.cpp

    Kopete Web Presence plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                    	*
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qspinbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

#include "webpresenceprefs.h"
#include "webpresencepreferences.h"
#include "webpresencepreferences.moc"

WebPresencePreferences::WebPresencePreferences( const QString &pixmap, QObject *parent )
	: ConfigModule( i18n( "Web Presence" ), i18n( "Web Presence Plugin" ), pixmap, parent )
{
	(  new QVBoxLayout( this ) )->setAutoAdd( true );
	m_prefsDialog = new WebPresencePrefsUI( this );
	KConfig *theConfig = KGlobal::config();
	theConfig->setGroup( "Web Presence Plugin" );
	m_prefsDialog->m_freq->setValue( theConfig->readNumEntry( "UploadFrequency" , 600 ) );
	m_prefsDialog->m_url->setText( theConfig->readEntry( "DestinationURL", QString::null) );
	m_prefsDialog->m_status->setChecked( theConfig->readBoolEntry( "ShowMyStatus", true ) );
	m_prefsDialog->m_contacts->setChecked( theConfig->readBoolEntry( "ShowMyContacts"
				, false ) );
	m_prefsDialog->m_addresses->setChecked( theConfig->readBoolEntry( "ShowAddresses", false ) );
	m_prefsDialog->m_rbHtml->setChecked( theConfig->readBoolEntry( "UploadHTML", true ) );
}

WebPresencePreferences::~WebPresencePreferences()
{}

void WebPresencePreferences::save()
{
	KConfig *theConfig = KGlobal::config();
	theConfig->setGroup(  "Web Presence Plugin" );
	theConfig->writeEntry( "UploadFrequency", m_prefsDialog->m_freq->value() );
	theConfig->writeEntry( "DestinationURL", m_prefsDialog->m_url->text() );
	theConfig->writeEntry( "ShowMyStatus", m_prefsDialog->m_status->isChecked() );
	theConfig->writeEntry( "ShowMyContacts", m_prefsDialog->m_contacts->isChecked() );
	theConfig->writeEntry( "ShowAddresses", m_prefsDialog->m_addresses->isChecked() );
	theConfig->writeEntry( "UploadHTML", m_prefsDialog->m_rbHtml->isChecked() );
	theConfig->sync();
	emit saved();
}

int WebPresencePreferences::frequency() const
{
	return m_prefsDialog->m_freq->value();
}

QString WebPresencePreferences::url() const
{
	return m_prefsDialog->m_url->text();
}

bool WebPresencePreferences::showMyself() const
{
	return m_prefsDialog->m_status->isChecked();
}

bool WebPresencePreferences::showMyContacts() const
{
	return m_prefsDialog->m_contacts->isChecked();
}

bool WebPresencePreferences::showAddresses() const
{
	return m_prefsDialog->m_addresses->isChecked();
}

bool WebPresencePreferences::uploadHtml() const
{
	return m_prefsDialog->m_rbHtml->isChecked();
}

// vim: set noet ts=4 sts=4 sw=4:
