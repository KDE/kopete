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

#include <kconfig.h>
#include <klocale.h>
#include <kurlrequester.h>

#include "config.h"
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
	m_prefsDialog->m_url->setURL( theConfig->readEntry( "DestinationURL", QString::null ) );
	m_prefsDialog->m_addresses->setChecked( theConfig->readBoolEntry( "ShowAddresses", false ) );
	m_prefsDialog->m_userName->setText( theConfig->readEntry( "UserName" , QString::null ) );
	QString format = theConfig->readEntry( "Formatting", QString::null );
	if ( format == "NoFormat" )
		m_prefsDialog->m_rbNoFormat->setChecked( true );
	else if ( format == "DefaultStyleSheet" )
			m_prefsDialog->m_rbDefaultStyleSheet->setChecked( true );
		else if ( format == "UserStyleSheet" )
				m_prefsDialog->m_rbUserStyleSheet->setChecked( true );

#ifndef HAVE_XSLT
	m_prefsDialog->m_rbNoFormat->setChecked( true );
	m_prefsDialog->m_rbDefaultStyleSheet->setEnabled( false );
	m_prefsDialog->m_rbUserStyleSheet->setEnabled( false );
#endif

	if ( theConfig->readBoolEntry( "UseIMName" ) )
	{
		m_prefsDialog->m_rbUseImName->setChecked( true );
		m_prefsDialog->m_rbUseUserName->setChecked( false );
		}
	else
	{
		m_prefsDialog->m_rbUseImName->setChecked( false );
		m_prefsDialog->m_rbUseUserName->setChecked( true );
	}
	m_prefsDialog->m_userStyleSheet->setURL( 
		theConfig->readEntry( "UserStyleSheetName" , QString::null ));
}

WebPresencePreferences::~WebPresencePreferences()
{}

void WebPresencePreferences::save()
{
	KConfig *theConfig = KGlobal::config();
	theConfig->setGroup( "Web Presence Plugin" );
	theConfig->writeEntry( "DestinationURL", m_prefsDialog->m_url->url() );
	theConfig->writeEntry( "ShowAddresses", m_prefsDialog->m_addresses->isChecked() );
	theConfig->writeEntry( "UseIMName", m_prefsDialog->m_rbUseImName->isChecked() );
	theConfig->writeEntry( "UserName", m_prefsDialog->m_userName->text() );
	
	if ( m_prefsDialog->m_rbNoFormat->isChecked() )
		theConfig->writeEntry( "Formatting", "NoFormat" );
	if ( m_prefsDialog->m_rbDefaultStyleSheet->isChecked() )
		theConfig->writeEntry( "Formatting", "DefaultStyleSheet" );
	if ( m_prefsDialog->m_rbUserStyleSheet->isChecked() )
		theConfig->writeEntry( "Formatting", "UserStyleSheet" );
	theConfig->writeEntry( "UserStyleSheetName", 
		m_prefsDialog->m_userStyleSheet->url() );	
	
	theConfig->sync();
	emit saved();
}

int WebPresencePreferences::frequency() const
{
	return KGlobal::config()->readNumEntry( "UploadFrequency" , 15 );
}

QString WebPresencePreferences::url() const
{
	return m_prefsDialog->m_url->url();
}

bool WebPresencePreferences::showAddresses() const
{
	return m_prefsDialog->m_addresses->isChecked();
}

bool WebPresencePreferences::useImName() const
{
	return m_prefsDialog->m_rbUseImName->isChecked();
}

QString WebPresencePreferences::userName() const
{
	return m_prefsDialog->m_userName->text();
}

bool WebPresencePreferences::useDefaultStyleSheet() const
{
	return m_prefsDialog->m_rbDefaultStyleSheet->isChecked();
}

bool WebPresencePreferences::justXml() const
{
	return m_prefsDialog->m_rbNoFormat->isChecked();
}

QString WebPresencePreferences::userStyleSheet() const
{
	return m_prefsDialog->m_userStyleSheet->url();
}

// vim: set noet ts=4 sts=4 sw=4:
