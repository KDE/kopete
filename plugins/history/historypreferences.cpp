/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart             <ogoffart @ kde.org>
              (c) 2003 by Stefan Gehn                 <metz AT gehn.net>
    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "historypreferences.h"
#include "historyconfig.h"
#include "historyprefsui.h"

#include <kgenericfactory.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <qcheckbox.h>

typedef KGenericFactory<HistoryPreferences> HistoryConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_history, HistoryConfigFactory( "kcm_kopete_history" ) )

HistoryPreferences::HistoryPreferences(QWidget *parent, const char*/*name*/, const QStringList &args)
	: KCModule(HistoryConfigFactory::instance(), parent, args)
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
	(new QVBoxLayout(this))->setAutoAdd(true);
	p = new HistoryPrefsUI(this);

	connect(p->chkShowPrevious, SIGNAL(toggled(bool)), this, SLOT(slotShowPreviousChanged(bool)));
	connect(p->Number_Auto_chatwindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->Number_ChatWindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->History_color, SIGNAL(changed(const QColor&)),
		this, SLOT(slotModified()));
	load();
}

HistoryPreferences::~HistoryPreferences()
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
}

void HistoryPreferences::load()
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
	HistoryConfig::self()->readConfig();
	p->chkShowPrevious->setChecked(HistoryConfig::auto_chatwindow());
	slotShowPreviousChanged(p->chkShowPrevious->isChecked());
	p->Number_Auto_chatwindow->setValue(HistoryConfig::number_Auto_chatwindow());
	p->Number_ChatWindow->setValue(HistoryConfig::number_ChatWindow());
	p->History_color->setColor(HistoryConfig::history_color());
	//p-> HistoryConfig::browserStyle();
	emit KCModule::changed(false);
}

void HistoryPreferences::save()
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
	HistoryConfig::setAuto_chatwindow(p->chkShowPrevious->isChecked());
	HistoryConfig::setNumber_Auto_chatwindow(p->Number_Auto_chatwindow->value());
	HistoryConfig::setNumber_ChatWindow(p->Number_ChatWindow->value());
	HistoryConfig::setHistory_color(p->History_color->color());
	HistoryConfig::self()->writeConfig();
	emit KCModule::changed(false);
}

void HistoryPreferences::slotModified()
{
	emit KCModule::changed(true);
}

void HistoryPreferences::slotShowPreviousChanged(bool on)
{
	emit KCModule::changed(true);
}

#include "historypreferences.moc"
