/***************************************************************************
                          configmodule.cpp  -  description
                             -------------------
    begin                : Thu Jan 3 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "configmodule.h"
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>

ConfigModule::ConfigModule(const QString &name, const QString &description, QObject *owner)
	: QWidget(kopeteapp->preferencesBox()->addPage(name, description))
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
