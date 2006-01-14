/*
    nowlisteningpreferences.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2005           by Michaël Larouche <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005      by the Kopete developers  <kopete-devel@kde.org>

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
#include <qradiobutton.h>

#include <klistbox.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "config.h" // for HAVE_XMMS
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

	addConfig( NowListeningConfig::self(), preferencesDialog );

	// Fill the media player listbox.
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("Kscd"));
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("Noatun"));
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("Juk"));
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("amaroK"));
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("Kaffeine"));
#if defined Q_WS_X11 && !defined K_WS_QTONLY && defined HAVE_XMMS
	preferencesDialog->kcfg_SelectedMediaPlayer->insertItem(QString::fromUtf8("XMMS"));
#endif
	load();
}

NowListeningPreferences::~NowListeningPreferences( )
{
	delete preferencesDialog;
}

void NowListeningPreferences::save()
{
	KCModule::save();
}

void NowListeningPreferences::load()
{
	KCModule::load();
}

void NowListeningPreferences::slotSettingsChanged()
{
	emit changed( true );
}

void NowListeningPreferences::defaults()
{
	/*preferencesDialog->m_header->setText( i18n("Now Listening To: "));
	preferencesDialog->m_perTrack->setText(i18n("%track( by %artist)( on %album)"));
	preferencesDialog->m_conjunction->setText( i18n(", and "));
	preferencesDialog->m_autoAdvertising->setChecked( false );*/
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
