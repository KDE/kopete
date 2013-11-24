/*
    nowlisteningpreferences.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2005           by Michaël Larouche <larouche@kde.org>

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

#include "nowlisteningpreferences.h"

#include <qspinbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <QVBoxLayout>

#include <klistwidget.h>
#include <klocale.h>
#include <kpluginfactory.h>

#include "ui_nowlisteningprefs.h"
#include "nowlisteningconfig.h"
#include "nowlisteningpreferences.moc"

K_PLUGIN_FACTORY( NowListeningPreferencesFactory, registerPlugin<NowListeningPreferences>(); )
K_EXPORT_PLUGIN( NowListeningPreferencesFactory( "kcm_kopete_nowlistening" ) )


NowListeningPreferences::NowListeningPreferences(QWidget *parent, const QVariantList &args)
	: KCModule( NowListeningPreferencesFactory::componentData(), parent, args )
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	preferencesDialog = new Ui::NowListeningPrefsUI;
	preferencesDialog->setupUi( w );
	l->addWidget( w );

	//Is this correct?
	addConfig( NowListeningConfig::self(), w );

	// Fill the media player listbox.
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("Kscd"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("Juk"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("Amarok"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("Kaffeine"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("Quod Libet"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("MPRIS compatible player"));
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("MPRIS2 compatible player"));
#if defined Q_WS_X11 && !defined K_WS_QTONLY && defined HAVE_XMMS
	preferencesDialog->kcfg_SelectedMediaPlayer->addItem(QString::fromUtf8("XMMS"));
#endif
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
