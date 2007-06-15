/*
    contactinfopage.cpp - A base class for contact info pages

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

#include "kopeteinfopage.h"
#include <kopetepropertycontainer.h>
#include <KLocale>

namespace Kopete
{

namespace UI
{

InfoPage::InfoPage(const Kopete::PropertyContainer *properties)
: QWidget()
{
	m_properties = const_cast<Kopete::PropertyContainer*>(properties);

	//connect(m_contact, SIGNAL(propertyChanged(Kopete::Contact*, const QString&, const QVariant&, const QVariant&)),
	//	this, SLOT(slotPropertyChanged(Kopete::Contact*, const QString&, const QVariant&, const QVariant&)));
}

InfoPage::~InfoPage()
{
}

void InfoPage::load()
{
	// I guess there is nothing to do here
}

void InfoPage::save()
{
	// nothing to do here either
}

QString InfoPage::pageName() const
{
	return i18n("Contact Info Page");
}

} // namespace UI
} // namespace Kopete

#include "kopeteinfopage.moc"
