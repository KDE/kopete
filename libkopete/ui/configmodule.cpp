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
#include "preferencesdialog.h"

#include <qlabel.h>
#include <qlayout.h>

#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>

ConfigModule::ConfigModule(const QString &name, const QString &description, QObject *owner)
	: QWidget( PreferencesDialog::preferencesDialog()->addPage( name, description ) )
{
	if (owner)
		connect(owner, SIGNAL(destroyed()), SLOT(ownerDeleted()));
	PreferencesDialog::preferencesDialog()->add(this);

	QFrame *page=static_cast<QFrame*>(parent());
	(new QHBoxLayout(page))->addWidget(this);
}

ConfigModule::ConfigModule(const QString &name, const QString &description, const QString &pixmap, QObject *owner)
	: QWidget(PreferencesDialog::preferencesDialog()->addPage(name, description, KGlobal::iconLoader()->loadIcon(pixmap,KIcon::NoGroup, KIcon::SizeMedium)  ))
{
	if (owner)
		connect(owner, SIGNAL(destroyed()), SLOT(ownerDeleted()));
	PreferencesDialog::preferencesDialog()->add(this);

	QFrame *page=static_cast<QFrame*>(parent());
	(new QHBoxLayout(page))->addWidget(this);
}


ConfigModule::~ConfigModule()
{
#if (QT_VERSION>=0x030005 && QT_VERSION<0x030102) && KDE_VERSION<0x0302000
	// Due to a bug in Qt 3.1 and 3.1.1 no close events are sent to hidden
	// widgets, causing the KJanusWidget to crash. This workaround is
	// rather intrusive and should be used only in the affected versions
	// to avoid hard to track bugs in the future. KDE HEAD (to become 3.2)
	// has a workaround for this problem, and additionally it's fixed in
	// Qt 3.1.2.
	kapp->sendPostedEvents();
#endif

	PreferencesDialog::preferencesDialog()->remove(this);
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
	PreferencesDialog::preferencesDialog()->showPage(PreferencesDialog::preferencesDialog()->pageIndex(page));
	PreferencesDialog::preferencesDialog()->show();
	PreferencesDialog::preferencesDialog()->raise();
}

#include "configmodule.moc"

// vim: set noet ts=4 sts=4 sw=4:

