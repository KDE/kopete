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

#include "latexplugin.h"
#include "latexconfig.h"
#include "latexprefsbase.h"
#include "latexpreferences.h"

typedef KGenericFactory<LatexPreferences> LatexPreferencesFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_latex, LatexPreferencesFactory( "kcm_kopete_latex" )  )

LatexPreferences::LatexPreferences(QWidget *parent, const char* /*name*/, const QStringList &args)
							: KCModule(LatexPreferencesFactory::instance(), parent, args)
{
	m_mutex=true;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	m_preferencesDialog = new LatexPrefsUI(this);
	m_config = new LatexConfig;

	// connect widget signals here
	
	load();
	m_mutex=false;
}

LatexPreferences::~LatexPreferences()
{
	delete m_config;
}

void LatexPreferences::load()
{
	m_config->load();
	m_mutex=true;

	// load widgets here
	
	m_mutex=false;
	emit KCModule::changed(false);
}

void LatexPreferences::save()
{
	m_config->save();
	emit KCModule::changed(false);
}

#include "latexpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
