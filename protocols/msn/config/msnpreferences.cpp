/*
    msnpreferences.cpp - MSN Preferences Widget

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnpreferences.h"

#include <qcheckbox.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kcmodule.h>

#include "msnprefs.h"

typedef KGenericFactory<MSNPreferences> MSNProtocolConfigFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_msn, MSNProtocolConfigFactory( "kcm_kopete_msn" ) );

MSNPreferences::MSNPreferences( QWidget *parent, const char * /* name */, const QStringList &args )
: KCModule( MSNProtocolConfigFactory::instance(), parent, args )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_preferencesDialog = new msnPrefsUI( this );
	load();

	connect( m_preferencesDialog->mSendAwayMessages, SIGNAL( toggled( bool ) ),
		this, SLOT( slotSettingsDirty() ) );
	connect( m_preferencesDialog->mNotifyNewChat, SIGNAL( toggled( bool ) ),
		this, SLOT( slotSettingsDirty() ) );
	connect( m_preferencesDialog->mAwayMessageSeconds, SIGNAL( valueChanged( int ) ),
		this, SLOT( slotSettingsDirty() ) );
}

void MSNPreferences::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "MSN" );

	m_preferencesDialog->mNotifyNewChat->setChecked( config->readBoolEntry( "NotifyNewChat", false ) );
	m_preferencesDialog->mSendAwayMessages->setChecked( config->readBoolEntry( "SendAwayMessages", true ) );
	m_preferencesDialog->mAwayMessageSeconds->setValue( config->readNumEntry( "AwayMessagesSeconds", 90 ) );
	m_preferencesDialog->mAwayMessageSeconds->setEnabled( m_preferencesDialog->mSendAwayMessages->isChecked() );

	setChanged( false );
}

void MSNPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup( "MSN" );

	config->writeEntry( "NotifyNewChat", m_preferencesDialog->mNotifyNewChat->isChecked() );
	config->writeEntry( "SendAwayMessages", m_preferencesDialog->mSendAwayMessages->isChecked() );
	config->writeEntry( "AwayMessagesSeconds", m_preferencesDialog->mAwayMessageSeconds->value() );

	setChanged( false );
}

void MSNPreferences::slotSettingsDirty()
{
	m_preferencesDialog->mAwayMessageSeconds->setEnabled( m_preferencesDialog->mSendAwayMessages->isChecked() );

	// Just mark settings dirty, even if the user undoes his changes,
	// because KPrefs will handle it in the near future.
	setChanged( true );
}

#include "msnpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:

