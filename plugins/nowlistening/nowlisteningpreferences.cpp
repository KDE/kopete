/*
    nowlisteningpreferences.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
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

#include <klocale.h>
#include <kgenericfactory.h>

#include "nowlisteningprefs.h"
#include "nowlisteningconfig.h"
#include "nowlisteningpreferences.h"
#include "nowlisteningpreferences.moc"

typedef KGenericFactory<NowListeningPreferences> NowListeningPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_nowlistening, NowListeningPreferencesFactory( "kcm_kopete_nowlistening" )  )


NowListeningPreferences::NowListeningPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
	: KCModule( NowListeningPreferencesFactory::instance(), parent, args )
{
	(  new QVBoxLayout(  this ) )->setAutoAdd(  true );
	preferencesDialog = new NowListeningPrefsUI( this );
	config = new NowListeningConfig;

	connect ( preferencesDialog->m_header, SIGNAL( textChanged( const QString & ) ),
		  this, SLOT( slotSettingsChanged() ) );
	connect ( preferencesDialog->m_perTrack, SIGNAL( textChanged( const QString & ) ),
		  this, SLOT( slotSettingsChanged() ) );
	connect ( preferencesDialog->m_conjunction, SIGNAL( textChanged( const QString & ) ),
		  this, SLOT( slotSettingsChanged() ) );
	load();
}

NowListeningPreferences::~NowListeningPreferences( )
{
	delete preferencesDialog;
	delete config;
}

void NowListeningPreferences::save()
{
	config->setHeader( preferencesDialog->m_header->text() );
	config->setPerTrack( preferencesDialog->m_perTrack->text() );
	config->setConjunction( preferencesDialog->m_conjunction->text() );
	config->save();

	KCModule::save();

	emit changed( false );
}

void NowListeningPreferences::load()
{
	config->load();
	preferencesDialog->m_header->setText( config->header() );
	preferencesDialog->m_perTrack->setText( config->perTrack() );
	preferencesDialog->m_conjunction->setText( config->conjunction() );

	KCModule::load();

	emit changed( false );
}

void NowListeningPreferences::slotSettingsChanged()
{
	emit changed( true );
}

void NowListeningPreferences::defaults()
{
	preferencesDialog->m_header->setText( i18n("Now Listening To: "));
	preferencesDialog->m_perTrack->setText(i18n("%track( by %artist)( on %album)"));
	preferencesDialog->m_conjunction->setText( i18n(", and "));
}

/*
* Local variables:
* c-indentation-style: k&r
* c-basic-offset: 8
* indent-tabs-mode: t
* End:
*/
// vim: set noet ts=4 sts=4 sw=4:
//
