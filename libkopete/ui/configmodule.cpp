/*
    configmodule.cpp - Kopete Plugin Config Module

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett       <duncan@kde.org>

    Portions of this code based in Noatun plugin code:
    Copyright (c) 2000-2002 The Noatun Developers

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "configmodule.h"
#include "kopete.h"
#include "preferencesdialog.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

ConfigModule::ConfigModule(const QString &name, const QString &description, QObject *owner)
	: QWidget(kopeteapp->preferencesBox()->addPage(name, description))
{
	if (owner)
		connect(owner, SIGNAL(destroyed()), SLOT(ownerDeleted()));
	kopeteapp->preferencesBox()->add(this);

	QFrame *page=static_cast<QFrame*>(parent());
	(new QHBoxLayout(page))->addWidget(this);
}

ConfigModule::ConfigModule(const QString &name, const QString &description, const QString &pixmap, QObject *owner)
	: QWidget(kopeteapp->preferencesBox()->addPage(name, description, KGlobal::iconLoader()->loadIcon(pixmap,KIcon::NoGroup, KIcon::SizeMedium)  ))
{
	if (owner)
		connect(owner, SIGNAL(destroyed()), SLOT(ownerDeleted()));
	kopeteapp->preferencesBox()->add(this);

	QFrame *page=static_cast<QFrame*>(parent());
	(new QHBoxLayout(page))->addWidget(this);
}


ConfigModule::~ConfigModule()
{
	kopeteapp->preferencesBox()->remove(this);

}

void ConfigModule::ownerDeleted()
{
	QObject *p=parent();
	delete this;
	delete p;
}

void ConfigModule::activate()
{
	QFrame *page=static_cast<QFrame*>(parent());
	kopeteapp->preferencesBox()->showPage(kopeteapp->preferencesBox()->pageIndex(page));
	kopeteapp->preferencesBox()->show();
	kopeteapp->preferencesBox()->raise();
}

#include "configmodule.moc"

// vim: set noet ts=4 sts=4 sw=4:

