/*
    kopeteidentity.cpp - Kopete Identity

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>

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

#include "kopetepropertycontainer.h"
#include "kopeteidentity.h"
#include "kopeteaccount.h"

#include <QStringList>
#include <KDebug>

#include <kdeversion.h>

namespace Kopete 
{

class Identity::Private
{
public:
	QList<Kopete::Account*> accounts;
	QString id;
};

Identity::Identity(const QString &id)
{
	d = new Private;
	d->id = id;

}

Identity::~Identity()
{
	delete d;
}

void Identity::setProperty(const Kopete::ContactPropertyTmpl &tmpl, const QVariant &value)
{
	PropertyContainer::setProperty( tmpl, value );
}

Kopete::UI::InfoPage::List Identity::customInfoPages() const
{
	// TODO implement
	Kopete::UI::InfoPage::List list;
	return list;
}

void Identity::notifyPropertyChanged( const QString &key, 
				      const QVariant &oldValue, const QVariant &newValue )
{
	//TODO implement
}


} //END namespace Kopete

#include "kopeteidentity.moc"

