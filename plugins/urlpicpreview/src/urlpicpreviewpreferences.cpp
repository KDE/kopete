/*
    urlpicpreviewpreferences.cpp
 
    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

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
#include "urlpicpreviewprefsui.h"
#include "urlpicpreviewpreferences.h"

typedef KGenericFactory<URLPicPreviewPreferences> URLPicPreviewPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kopete_urlpicpreview, URLPicPreviewPreferencesFactory("kcm_kopete_urlpicpreview"));

URLPicPreviewPreferences::URLPicPreviewPreferences(QWidget* parent, const char* name, const QStringList& /* args */ )
        : KCModule(URLPicPreviewPreferencesFactory::instance(), parent, name), m_layout(NULL), m_ui(NULL) {

    m_ui = new URLPicPreviewPrefsBase(this);
    m_layout = new QHBoxLayout(this);
    m_layout->add(m_ui);

    // don't display link preview features yet
    m_ui->linkPreviewGroup->hide();

    connect(m_ui->enableScaling, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->previewScaleWidth, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(m_ui->restrictPreviews, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->previewAmount, SIGNAL(valueChanged(int)), this, SLOT(changed()));

    load();
}

URLPicPreviewPreferences::~URLPicPreviewPreferences() {
    delete m_ui;
    delete m_layout;
}

void URLPicPreviewPreferences::load() {

    kdDebug(0) << k_funcinfo << endl;

	m_ui->enableScaling->setChecked(URLPicPreviewConfig::self()->scaling());
	m_ui->restrictPreviews->setChecked(URLPicPreviewConfig::self()->previewRestriction());
	m_ui->previewScaleWidth->setValue(URLPicPreviewConfig::self()->previewScaleWidth());
	m_ui->previewAmount->setValue(URLPicPreviewConfig::self()->previewAmount());
}

void URLPicPreviewPreferences::save() {

    kdDebug(0) << k_funcinfo << endl;

	URLPicPreviewConfig::self()->setScaling(m_ui->enableScaling->isChecked());
	URLPicPreviewConfig::self()->setPreviewRestriction(m_ui->restrictPreviews->isChecked());
	URLPicPreviewConfig::self()->setPreviewScaleWidth(m_ui->previewScaleWidth->value());
	URLPicPreviewConfig::self()->setPreviewAmount(m_ui->previewAmount->value());

	URLPicPreviewConfig::self()->writeConfig();
}

#include "urlpicpreviewpreferences.moc"
