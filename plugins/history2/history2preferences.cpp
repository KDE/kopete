/*
    history2preferences.cpp

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

#include "history2preferences.h"

#include <QtCore/QPointer>
#include <QtGui/QLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCheckBox>

#include <kcolorbutton.h>
#include <knuminput.h>
#include <kgenericfactory.h>

#include "history2import.h"
#include "history2config.h"
#include "ui_history2prefsui.h"

K_PLUGIN_FACTORY( History2PreferencesFactory, registerPlugin<History2Preferences>(); )
K_EXPORT_PLUGIN( History2PreferencesFactory( "kcm_kopete_history2" ) )

History2Preferences::History2Preferences(QWidget *parent, const QVariantList &args)
	: KCModule(History2PreferencesFactory::componentData(), parent, args)
{
	kDebug(14310) << "called.";
	
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	p = new Ui::History2PrefsUI;
	p->setupUi( w );
	l->addWidget( w );

	connect(p->chkShowPrevious, SIGNAL(toggled(bool)), this, SLOT(slotShowPreviousChanged(bool)));
	connect(p->Number_Auto_chatwindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->Number_ChatWindow, SIGNAL(valueChanged(int)),
		this, SLOT(slotModified()));
	connect(p->History_color, SIGNAL(changed(QColor)),
		this, SLOT(slotModified()));
	connect(p->import2, SIGNAL(clicked()), this, SLOT(slotShowImport()));
}

History2Preferences::~History2Preferences()
{
	kDebug(14310) << "called.";
	delete p;
}

void History2Preferences::load()
{
	kDebug(14310) << "called.";
	History2Config::self()->readConfig();
	p->chkShowPrevious->setChecked(History2Config::auto_chatwindow());
	slotShowPreviousChanged(p->chkShowPrevious->isChecked());
	p->Number_Auto_chatwindow->setValue(History2Config::number_Auto_chatwindow());
	p->Number_ChatWindow->setValue(History2Config::number_ChatWindow());
	p->History_color->setColor(History2Config::history_color());
	//p-> History2Config::browserStyle();
	emit KCModule::changed(false);
}

void History2Preferences::save()
{
	kDebug(14310) << "called.";
	History2Config::setAuto_chatwindow(p->chkShowPrevious->isChecked());
	History2Config::setNumber_Auto_chatwindow(p->Number_Auto_chatwindow->value());
	History2Config::setNumber_ChatWindow(p->Number_ChatWindow->value());
	History2Config::setHistory_color(p->History_color->color());
	History2Config::self()->writeConfig();
	emit KCModule::changed(false);
}

void History2Preferences::slotShowImport()
{
	QPointer <History2Import> importer = new History2Import(parentWidget());
	importer->exec();
	delete importer;
}

void History2Preferences::slotModified()
{
	emit KCModule::changed(true);
}

void History2Preferences::slotShowPreviousChanged(bool on)
{
	Q_UNUSED(on);
	emit KCModule::changed(true);
}

#include "history2preferences.moc"
