/*
    kopetecontactproperty.cpp

    KopeteContact Property class

    Copyright (c) 2004    by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2004    by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactproperty.h"
#include <kdebug.h>

namespace Kopete
{

ContactProperty ContactProperty::null;

ContactProperty::ContactProperty()
{
}

ContactProperty::ContactProperty(const QVariant &value,
	const QString &label, const QString &icon)
{
	mValue = value;
	mLabel = label;
	mIcon = icon;
}

ContactProperty::~ContactProperty()
{
	//kdDebug(14000) << k_funcinfo << "this = " << (void *)this << endl;
}

const QString &ContactProperty::label() const
{
	return mLabel;
}

const QString &ContactProperty::icon() const
{
	return mIcon;
}

const QVariant &ContactProperty::value() const
{
	return mValue;
}

bool ContactProperty::isNull() const
{
	return (mValue.isNull() && mLabel.isNull());
}

} // END namespace Kopete