/*
    historypreferences.cpp

    Copyright (c) 2003      by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Stefan Gehn            <metz@gehn.net>

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

#include <QtGui/QLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>

#include <kcolorbutton.h>
#include <knuminput.h>
#include <kgenericfactory.h>

#include "historyconfig.h"
#include "ui_historyprefsui.h"

K_PLUGIN_FACTORY( HistoryPreferencesFactory, registerPlugin<HistoryPreferences>(); )
K_EXPORT_PLUGIN( HistoryPreferencesFactory( "kcm_kopete_history" ) )

HistoryPreferences::HistoryPreferences(QWidget *parent, const QVariantList &args)
	: KCModule(HistoryPreferencesFactory::componentData(), parent, args)
{
	kDebug(14310) << "called.";
	
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	p = new Ui::HistoryPrefsUI;
	p->setupUi( w );
	l->addWidget( w );

	connect(p->chkShowPrevious, SIGNAL(toggled(bool)), this, SLOT(slotShowPreviousChanged(bool)));
	connect(p->Number_Auto_chatwindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->Number_ChatWindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->History_color, SIGNAL(changed(QColor)),
		this, SLOT(slotModified()));
}

HistoryPreferences::~HistoryPreferences()
{
	kDebug(14310) << "called.";
	delete p;
}

void HistoryPreferences::load()
{
	kDebug(14310) << "called.";
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
	kDebug(14310) << "called.";
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
	Q_UNUSED(on);
	emit KCModule::changed(true);
}

#include "historypreferences.moc"
