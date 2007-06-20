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
#include "kopeteinfopage.h"
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
	InfoPage::List pageList;
};

InfoDialog::InfoDialog(	const Kopete::PropertyContainer *properties, 
			const QString &title, const QString &icon)
{
	InfoDialog( properties, title, KIcon(icon) );
}

InfoDialog::InfoDialog(	const Kopete::PropertyContainer *properties,
			const QString &title, const KIcon &icon)
: KDialog()
{
	resize(500,500);
	d = new Private();
	d->layout = new QVBoxLayout(mainWidget());
	
	d->title = new KTitleWidget();
	if (!title.isEmpty())
		setTitle( title );
	else
		setTitle( i18n( "Information" ) );

	setIcon( icon );
	d->layout->addWidget( d->title );

	d->container = new SettingsContainer();
	d->layout->addWidget( d->container );
	
	d->pageList = properties->infoPages();
	InfoPage::List::const_iterator it;
	for (it = d->pageList.begin(); it != d->pageList.end(); ++it)
	{
		(*it)->load();
		CollapsibleWidget *c = d->container->insertWidget( *it, (*it)->pageName() );
		c->setExpanded(true);
	}

}

InfoDialog::~InfoDialog()
{
}

void InfoDialog::slotSave()
{
	InfoPage::List::const_iterator it;
	for (it = d->pageList.begin(); it != d->pageList.end(); ++it)
		(*it)->save();
}

void InfoDialog::setTitle(const QString &title)
{
	d->title->setText( title );
}

void InfoDialog::setIcon(const QString &icon)
{
	d->title->setPixmap( icon );
}

	void InfoDialog::setIcon(const KIcon &icon)
{
	d->title->setPixmap( icon );
}

} // namespace UI
} // namespace Kopete

#include "kopeteinfodialog.moc"
