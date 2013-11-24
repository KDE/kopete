/*
    Kopete LaTeX Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "latexpreferences.h"

#include <qlayout.h>

#include <kparts/componentfactory.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <knuminput.h>

#include "latexplugin.h"
#include "latexconfig.h"
#include "ui_latexprefsbase.h"

K_PLUGIN_FACTORY(LatexPreferencesFactory, registerPlugin<LatexPreferences>();)
K_EXPORT_PLUGIN(LatexPreferencesFactory( "kcm_kopete_latex" ))


LatexPreferences::LatexPreferences(QWidget *parent, const QVariantList &args)
							: KCModule(LatexPreferencesFactory::componentData(), parent, args)
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	m_preferencesDialog = new Ui::LatexPrefsUI;
	m_preferencesDialog->setupUi( w );
	l->addWidget( w );

	// connect widget signals here
	m_preferencesDialog->horizontalDPI->setMinimum(1);
	m_preferencesDialog->verticalDPI->setMinimum(1);
	m_preferencesDialog->horizontalDPI->setMaximum(1000);
	m_preferencesDialog->verticalDPI->setMaximum(1000);
	m_preferencesDialog->includeUrlRequester->setMode ( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
	
	connect(m_preferencesDialog->horizontalDPI, SIGNAL(valueChanged(int)), this, SLOT(slotModified()));
	connect(m_preferencesDialog->verticalDPI, SIGNAL(valueChanged(int)), this, SLOT(slotModified()));
	connect(m_preferencesDialog->includeUrlRequester, SIGNAL(textChanged(QString)), this, SLOT(slotModified()));
}

LatexPreferences::~LatexPreferences()
{
	delete m_preferencesDialog;
}

void LatexPreferences::load()
{
	LatexConfig::self()->readConfig();
	// load widgets here
	m_preferencesDialog->horizontalDPI->setValue(LatexConfig::horizontalDPI());
	m_preferencesDialog->verticalDPI->setValue(LatexConfig::verticalDPI());
	m_preferencesDialog->includeUrlRequester->setUrl( KUrl (LatexConfig::latexIncludeFile()) );
	emit KCModule::changed(false);
}

void LatexPreferences::slotModified()
{
	emit KCModule::changed(true);
}

void LatexPreferences::save()
{
	LatexConfig::setHorizontalDPI(m_preferencesDialog->horizontalDPI->value());
	LatexConfig::setVerticalDPI(m_preferencesDialog->verticalDPI->value());
	LatexConfig::setLatexIncludeFile(m_preferencesDialog->includeUrlRequester->url().path());
	LatexConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

void LatexPreferences::defaults()
{
	LatexConfig::self()->setDefaults();
	m_preferencesDialog->horizontalDPI->setValue(LatexConfig::horizontalDPI());
	m_preferencesDialog->verticalDPI->setValue(LatexConfig::verticalDPI());
	m_preferencesDialog->includeUrlRequester->setUrl( KUrl (LatexConfig::latexIncludeFile()) );
	emit KCModule::changed(true);
}

#include "latexpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
