/*
    kopetecontactproperty.cpp

    KopeteContact Property class

    Copyright (c) 2004      by Stefan Gehn            <metz AT gehn.net>

    Kopete    (c) 2004      by the Kopete developers  <kopete-devel@kde.org>

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

// -----------------------------------------------------------------------------
KopeteContactProperty KopeteContactProperty::null;

KopeteContactProperty::KopeteContactProperty()
{
	/*kdDebug(14000) << k_funcinfo <<
		"EMPTY, this = " << (void*) this << endl;*/
}

KopeteContactProperty::KopeteContactProperty( const QString &lbl,
	const QVariant &val )
{
	mLabel = lbl;
	mValue = val;
	/*kdDebug(14000) << k_funcinfo <<
		"label=" << mLabel << ", this = " << (void*) this << endl;*/
}

KopeteContactProperty::~KopeteContactProperty()
{
	//kdDebug(14000) << k_funcinfo << "this = " << (void *)this << endl;
}

const QString &KopeteContactProperty::label() const
{
	return mLabel;
}

const QVariant &KopeteContactProperty::value() const
{
	return mValue;
}

bool KopeteContactProperty::isNull() const
{
	return (mValue.isNull() && mLabel.isNull());
}
