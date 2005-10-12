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
#include <kconfig.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kgenericfactory.h>

// Kopete
#include "urlpicpreviewglobals.h"
#include "urlpicpreviewprefsui.h"
#include "urlpicpreviewpreferences.h"

typedef KGenericFactory<URLPicPreviewPreferences> URLPicPreviewPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kopete_urlpicpreview, URLPicPreviewPreferencesFactory("kcm_kopete_urlpicpreview"));

URLPicPreviewPreferences::URLPicPreviewPreferences(QWidget* parent, const char* name, const QStringList& /* args */ )
        : KCModule(URLPicPreviewPreferencesFactory::instance(), parent, name), m_layout(NULL), m_ui(NULL), m_config(NULL) {

    m_config = kapp->config();

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

    KCModule::load();

    m_config->setGroup(CONFIG_GROUP);
    m_ui->enableScaling->setChecked(m_config->readBoolEntry("Scaling", true));
    m_ui->restrictPreviews->setChecked(m_config->readBoolEntry("PreviewRestriction", true));
    m_ui->previewScaleWidth->setValue(m_config->readNumEntry("PreviewScaleWidth", 256));
    m_ui->previewAmount->setValue(m_config->readNumEntry("PreviewAmount", 2));
}

void URLPicPreviewPreferences::save() {

    kdDebug(0) << k_funcinfo << endl;

    KCModule::save();

    m_config->setGroup(CONFIG_GROUP);
    m_config->writeEntry("Scaling", m_ui->enableScaling->isChecked());
    m_config->writeEntry("PreviewRestriction", m_ui->restrictPreviews->isChecked());
    m_config->writeEntry("PreviewScaleWidth", m_ui->previewScaleWidth->value());
    m_config->writeEntry("PreviewAmount", m_ui->previewAmount->value());

    m_config->sync();
}

#include "urlpicpreviewpreferences.moc"
