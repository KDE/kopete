/*
    contactinfodialog.cpp - A dialog for configuring user info

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

#include "userinfodialog.h"
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

class UserInfoDialog::Private
{
public:
	QVBoxLayout *layout;
	KTitleWidget *title;
	SettingsContainer *container;
	InfoPage::List pageList;
};

UserInfoDialog::UserInfoDialog(const Kopete::PropertyContainer *properties)
: KDialog()
{
	resize(500,500);
	d = new Private();
	d->layout = new QVBoxLayout(mainWidget());
	
	d->title = new KTitleWidget();
	d->title->setText( i18n("User Information") );
	d->title->setPixmap( "identity" ); 
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

UserInfoDialog::~UserInfoDialog()
{
}

void UserInfoDialog::slotSave()
{
	InfoPage::List::const_iterator it;
	for (it = d->pageList.begin(); it != d->pageList.end(); ++it)
		(*it)->save();
}

} // namespace UI
} // namespace Kopete

#include "userinfodialog.moc"
