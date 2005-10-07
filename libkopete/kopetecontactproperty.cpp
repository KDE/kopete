/*
    kopetecontactproperty.cpp

    Kopete::Contact Property class

    Copyright (c) 2004    by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2004    by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactproperty.h"
#include <kdebug.h>
#include "kopeteglobal.h"

namespace Kopete
{

struct ContactPropertyTmplPrivate
{
	QString key;
	QString label;
	QString icon;
	bool persistent;
	bool richText;
	bool privateProp;
	unsigned int refCount;
};

ContactPropertyTmpl ContactPropertyTmpl::null;


ContactPropertyTmpl::ContactPropertyTmpl()
{
	d = new ContactPropertyTmplPrivate;
	d->refCount = 1;
	d->persistent = false;
	// Don't register empty template
}

ContactPropertyTmpl::ContactPropertyTmpl(const QString &key,
	const QString &label, const QString &icon, bool persistent, bool richText, bool privateProp)
{
	ContactPropertyTmpl other = Kopete::Global::Properties::self()->tmpl(key);
	if(other.isNull())
	{
//		kdDebug(14000) << k_funcinfo << "Creating new template for key = '" << key << "'" << endl;

		d = new ContactPropertyTmplPrivate;
		d->refCount = 1;
		d->key = key;
		d->label = label;
		d->icon = icon;
		d->persistent = persistent;
		d->richText = richText;
		d->privateProp = privateProp;
		Kopete::Global::Properties::self()->registerTemplate(key, (*this));
	}
	else
	{
//		kdDebug(14000) << k_funcinfo << "Using existing template for key = '" << key << "'" << endl;
		d = other.d;
		d->refCount++;
	}
}

ContactPropertyTmpl::ContactPropertyTmpl(const ContactPropertyTmpl &other)
{
	d = other.d;
	d->refCount++;
}

ContactPropertyTmpl &ContactPropertyTmpl::operator=(
	const ContactPropertyTmpl &other)
{
	d->refCount--;
	if(d->refCount == 0)
	{
		if (!d->key.isEmpty()) // null property
			Kopete::Global::Properties::self()->unregisterTemplate(d->key);
		delete d;
	}

	d = other.d;
	d->refCount++;

	return *this;
}

ContactPropertyTmpl::~ContactPropertyTmpl()
{
	d->refCount--;
	if(d->refCount == 0)
	{
		if (!d->key.isEmpty()) // null property
			Kopete::Global::Properties::self()->unregisterTemplate(d->key);
		delete d;
	}
}

bool ContactPropertyTmpl::operator==(const ContactPropertyTmpl &other) const
{
	return (d && other.d &&
		d->key == other.d->key &&
		d->label == other.d->label &&
		d->icon == other.d->key &&
		d->persistent == other.d->persistent);
}

bool ContactPropertyTmpl::operator!=(const ContactPropertyTmpl &other) const
{
	return (!d || !other.d ||
		d->key != other.d->key ||
		d->label != other.d->label ||
		d->icon != other.d->key ||
		d->persistent != other.d->persistent);
}


const QString &ContactPropertyTmpl::key() const
{
	return d->key;
}

const QString &ContactPropertyTmpl::label() const
{
	return d->label;
}

const QString &ContactPropertyTmpl::icon() const
{
	return d->icon;
}

bool ContactPropertyTmpl::persistent() const
{
	return d->persistent;
}

bool ContactPropertyTmpl::isRichText() const
{
	return d->richText;
}

bool ContactPropertyTmpl::isPrivate() const
{
	return d->privateProp;
}

bool ContactPropertyTmpl::isNull() const
{
	return (!d || d->key.isNull());
}


// -----------------------------------------------------------------------------


ContactProperty ContactProperty::null;

ContactProperty::ContactProperty()
{
}

ContactProperty::ContactProperty(const ContactPropertyTmpl &tmpl,
	const QVariant &val)
{
	mTemplate = tmpl;
	mValue = val;
}

ContactProperty::~ContactProperty()
{
	//kdDebug(14000) << k_funcinfo << "this = " << (void *)this << endl;
}

const QVariant &ContactProperty::value() const
{
	return mValue;
}

const ContactPropertyTmpl &ContactProperty::tmpl() const
{
	return mTemplate;
}

bool ContactProperty::isNull() const
{
	return mValue.isNull();
}

bool ContactProperty::isRichText() const
{
	return mTemplate.isRichText();
}

} // END namespace Kopete
