/*
    kopeteinfodialog.cpp - A dialog to configure information for contacts, 
                           metacontacts, groups, identities, etc

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteinfodialog.h"
#include "collapsiblewidget.h"
#include <kopetepropertycontainer.h>
#include <KLocale>
#include <KTitleWidget>
#include <KIconLoader>
#include <QVBoxLayout>

namespace Kopete
{

namespace UI
{

class InfoDialog::Private
{
public:
	QVBoxLayout *layout;
	KTitleWidget *title;
	SettingsContainer *container;
};

InfoDialog::InfoDialog(QWidget *parent, const QString &title, const QString &icon)
: KDialog(parent), d(new Private())
{
	initialize();

	if (!title.isEmpty())
		setTitle( title );
	else
		setTitle( i18n( "Information" ) );
	setIcon( icon );

	connect(this, SIGNAL(okClicked()), this, SLOT(slotSave()));
}

InfoDialog::InfoDialog(QWidget *parent, const QString &title, const KIcon &icon)
: KDialog(parent), d(new Private())
{
	initialize();

	if (!title.isEmpty())
		setTitle( title );
	else
		setTitle( i18n( "Information" ) );
	setIcon( icon );

}

void InfoDialog::initialize()
{
	//FIXME: this should be changed
	resize(500,500);

	d->layout = new QVBoxLayout(mainWidget());
	
	d->title = new KTitleWidget();
	d->layout->addWidget( d->title );

	d->container = new SettingsContainer();
	d->layout->addWidget( d->container );
}

InfoDialog::~InfoDialog()
{
    delete d;
}

void InfoDialog::slotSave()
{
}

void InfoDialog::setTitle(const QString &title)
{
	d->title->setText( title, Qt::AlignLeft );
}

void InfoDialog::setIcon(const QString &icon)
{
	d->title->setPixmap( icon );
}

void InfoDialog::setIcon(const KIcon &icon)
{
	d->title->setPixmap( icon );
}

void InfoDialog::addWidget(QWidget *w, const QString &caption)
{
	CollapsibleWidget *c = d->container->insertWidget(w, caption);
	// FIXME: maybe we could check for pages that were collapsed by the user the 
	// last time the dialog was shown
	c->setExpanded(true);
}

} // namespace UI
} // namespace Kopete

#include "kopeteinfodialog.moc"
