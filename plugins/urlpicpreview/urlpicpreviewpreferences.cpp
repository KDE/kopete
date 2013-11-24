/*
    urlpicpreviewpreferences.cpp

    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ************************************************************************ *
    *                                                                        *
    * This program is free software; you can redistribute it and/or modify   *
    * it under the terms of the GNU General Public License as published by   *
    * the Free Software Foundation; version 2, or (at your option) version 3 *
    * of the License.                                                        *
    *                                                                        *
    **************************************************************************
*/

#include "urlpicpreviewpreferences.h"

// Qt
#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>

// KDE
#include <kdebug.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kgenericfactory.h>
// Kopete
#include "urlpicpreviewconfig.h"
#include "ui_urlpicpreviewprefsbase.h"


K_PLUGIN_FACTORY(URLPicPreviewPreferencesFactory,
        registerPlugin<URLPicPreviewPreferences>();
        )
K_EXPORT_PLUGIN(URLPicPreviewPreferencesFactory ( "kcm_kopete_urlpicpreview" ))


URLPicPreviewPreferences::URLPicPreviewPreferences ( QWidget *parent, const QVariantList &args )
		: KCModule ( URLPicPreviewPreferencesFactory::componentData(), parent, args )
{
	m_ui = new Ui::URLPicPreviewPrefsUI;
	QWidget * w = new QWidget ( this );
	m_ui->setupUi ( w );
	m_layout = new QHBoxLayout ( this );
	m_layout->addWidget ( w );

	// don't display link preview features yet
	m_ui->linkPreviewGroup->hide();

	connect ( m_ui->enableScaling, SIGNAL (toggled(bool)), this, SLOT (changed()) );
	connect ( m_ui->previewScaleWidth, SIGNAL (valueChanged(int)), this, SLOT (changed()) );
	connect ( m_ui->restrictPreviews, SIGNAL (toggled(bool)), this, SLOT (changed()) );
	connect ( m_ui->previewAmount, SIGNAL (valueChanged(int)), this, SLOT (changed()) );
}

URLPicPreviewPreferences::~URLPicPreviewPreferences() {
	delete m_ui;
	delete m_layout;
}

void URLPicPreviewPreferences::load() {

	kDebug ( 14314 );

	m_ui->enableScaling->setChecked ( URLPicPreviewConfig::self()->scaling() );
	m_ui->restrictPreviews->setChecked ( URLPicPreviewConfig::self()->previewRestriction() );
	m_ui->previewScaleWidth->setValue ( URLPicPreviewConfig::self()->previewScaleWidth() );
	m_ui->previewAmount->setValue ( URLPicPreviewConfig::self()->previewAmount() );
}

void URLPicPreviewPreferences::save() {

	kDebug ( 14314 );

	URLPicPreviewConfig::self()->setScaling ( m_ui->enableScaling->isChecked() );
	URLPicPreviewConfig::self()->setPreviewRestriction ( m_ui->restrictPreviews->isChecked() );
	URLPicPreviewConfig::self()->setPreviewScaleWidth ( m_ui->previewScaleWidth->value() );
	URLPicPreviewConfig::self()->setPreviewAmount ( m_ui->previewAmount->value() );

	URLPicPreviewConfig::self()->writeConfig();
}

void URLPicPreviewPreferences::defaults() {
	URLPicPreviewConfig::self()->setDefaults ();
	load();
	changed();
}

#include "urlpicpreviewpreferences.moc"
