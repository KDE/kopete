/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <qcheckbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kcolorbutton.h>

#include "historyprefsui.h"
#include "historypreferences.h"

HistoryPreferences::HistoryPreferences(QObject *parent)
							: ConfigModule(i18n("History"),i18n("History Plugin"),"history",parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	m_widget = new HistoryPrefsUI(this);

	reopen();
}

HistoryPreferences::~HistoryPreferences()
{
}


void HistoryPreferences::reopen()
{
	KGlobal::config()->setGroup("History Plugin");

	m_widget->newView->setChecked(KGlobal::config()->readBoolEntry("Auto chatwindow" , false ));
	m_widget->nbNewView->setValue(KGlobal::config()->readNumEntry( "Number Auto chatwindow" , 7) );
	m_widget->nbChatWindow->setValue(KGlobal::config()->readNumEntry( "Number ChatWindow", 20) );
	QColor defaultcolor("dimgrey");
	m_widget->m_color->setColor(KGlobal::config()->readColorEntry( "History Color", &defaultcolor));
}

void HistoryPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("History Plugin");
	config->writeEntry("Auto chatwindow", m_widget->newView->isChecked() );
	config->writeEntry("Number Auto chatwindow",  m_widget->nbNewView->value() );
	config->writeEntry("Number ChatWindow",  m_widget->nbChatWindow->value() );
	config->writeEntry("History Color",  m_widget->m_color->color() );

	config->writeEntry("Version",  "0.7" );

	config->sync();
}


int HistoryPreferences::nbAutoChatwindow() const
{
	if(!m_widget->newView->isChecked())
		return 0;
	return m_widget->nbNewView->value();
}

int HistoryPreferences::nbChatwindow() const
{
	return m_widget->nbChatWindow->value();
}

QColor  HistoryPreferences::historyColor() const
{
	return m_widget->m_color->color();
}

#include "historypreferences.moc"
