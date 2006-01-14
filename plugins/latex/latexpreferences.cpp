/*
    Kopete Latex Plugin

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

#include <qlayout.h>
#include <kparts/componentfactory.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <knuminput.h>

#include "latexplugin.h"
#include "latexconfig.h"
#include "latexprefsbase.h"
#include "latexpreferences.h"

typedef KGenericFactory<LatexPreferences> LatexPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_latex, LatexPreferencesFactory( "kcm_kopete_latex" )  )

LatexPreferences::LatexPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCModule(LatexPreferencesFactory::instance(), parent, args)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_preferencesDialog = new LatexPrefsUI(this);
	// connect widget signals here
	m_preferencesDialog->horizontalDPI->setMinValue(1);
	m_preferencesDialog->verticalDPI->setMinValue(1);
	
	connect(m_preferencesDialog->horizontalDPI, SIGNAL(valueChanged(int)), this, SLOT(slotModified()));
	connect(m_preferencesDialog->verticalDPI, SIGNAL(valueChanged(int)), this, SLOT(slotModified()));
	
	load();
}

LatexPreferences::~LatexPreferences()
{
}

void LatexPreferences::load()
{
	LatexConfig::self()->readConfig();
	// load widgets here
	m_preferencesDialog->horizontalDPI->setValue(LatexConfig::self()->horizontalDPI());
	m_preferencesDialog->verticalDPI->setValue(LatexConfig::self()->verticalDPI());
	emit KCModule::changed(false);
}

void LatexPreferences::slotModified()
{
	emit KCModule::changed(true);
}

void LatexPreferences::save()
{
	LatexConfig::self()->setHorizontalDPI(m_preferencesDialog->horizontalDPI->value());
	LatexConfig::self()->setVerticalDPI(m_preferencesDialog->verticalDPI->value());
	LatexConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

#include "latexpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
