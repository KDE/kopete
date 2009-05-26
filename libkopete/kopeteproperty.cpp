/*
    kopeteproperty.cpp

    Kopete::Property class

    Copyright (c) 2007    by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2004    by Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2006    by Michaël Larouche <larouche@kde.org>

    Kopete    (c) 2004-2007    by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteproperty.h"
#include <kdebug.h>
#include "kopeteglobal.h"

namespace Kopete
{

class PropertyTmpl::Private
{
public:
	QString key;
	QString label;
	QString icon;
	PropertyOptions options;
	unsigned int refCount;
};

PropertyTmpl PropertyTmpl::null;


PropertyTmpl::PropertyTmpl()
{
	d = new Private;
	d->refCount = 1;
	d->options = NoProperty;
	// Don't register empty template
}

PropertyTmpl::PropertyTmpl(const QString &key,
	const QString &label, const QString &icon, PropertyOptions options)
{
	PropertyTmpl other = Kopete::Global::Properties::self()->tmpl(key);
	if(other.isNull())
	{
//		kDebug(14000) << "Creating new template for key = '" << key << "'";

		d = new Private;
		d->refCount = 1;
		d->key = key;
		d->label = label;
		d->icon = icon;
		d->options = options;
		Kopete::Global::Properties::self()->registerTemplate(key, (*this));
	}
	else
	{
//		kDebug(14000) << "Using existing template for key = '" << key << "'";
		d = other.d;
		d->refCount++;
	}
}

PropertyTmpl::PropertyTmpl(const PropertyTmpl &other)
{
	d = other.d;
	d->refCount++;
}

PropertyTmpl &PropertyTmpl::operator=(
	const PropertyTmpl &other)
{
	if (this == &other)
	{
//		kDebug(14000) << "trying to assign this to itself!";
		return *this;
	}
	if( d == other.d )
	{
//		kDebug(14000) << "trying to assign d to itself!";
		return *this;
	}
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

PropertyTmpl::~PropertyTmpl()
{
	d->refCount--;
	if(d->refCount == 0)
	{
		if (!d->key.isEmpty()) // null property
			Kopete::Global::Properties::self()->unregisterTemplate(d->key);
		delete d;
	}
}

bool PropertyTmpl::operator==(const PropertyTmpl &other) const
{
	return (d && other.d &&
		d->key == other.d->key &&
		d->label == other.d->label &&
		d->icon == other.d->key &&
		d->options == other.d->options);
}

bool PropertyTmpl::operator!=(const PropertyTmpl &other) const
{
	return (!d || !other.d ||
		d->key != other.d->key ||
		d->label != other.d->label ||
		d->icon != other.d->key ||
		d->options != other.d->options);
}


const QString &PropertyTmpl::key() const
{
	return d->key;
}

const QString &PropertyTmpl::label() const
{
	return d->label;
}

const QString &PropertyTmpl::icon() const
{
	return d->icon;
}

PropertyTmpl::PropertyOptions PropertyTmpl::options() const
{
	return d->options;
}

bool PropertyTmpl::persistent() const
{
	return d->options & PersistentProperty;
}

bool PropertyTmpl::isRichText() const
{
	return d->options & RichTextProperty;
}

bool PropertyTmpl::isPrivate() const
{
	return d->options & PrivateProperty;
}

bool PropertyTmpl::isNull() const
{
	return (!d || d->key.isNull());
}


// -----------------------------------------------------------------------------


Property Property::null;

class Property::Private
{
public:
	QVariant value;
	PropertyTmpl propertyTemplate;
};

Property::Property()
 : d(new Private)
{
}

Property::Property(const PropertyTmpl &tmpl,
	const QVariant &val)
 : d(new Private)
{
	d->propertyTemplate = tmpl;
	d->value = val;
}

Property::Property(const Property& other)
 : d(new Private)
{
	d->propertyTemplate = other.d->propertyTemplate;
	d->value = other.d->value;
}

Property::~Property()
{
	delete d;
}

Property& Property::operator=(const Property& other)
{
	if (this == &other)
	{
//		kDebug(14000) << "trying to assign this to itself!";
		return *this;
	}

	d->propertyTemplate = other.d->propertyTemplate;
	d->value = other.d->value;

	return *this;
}

const QVariant &Property::value() const
{
	return d->value;
}

const PropertyTmpl &Property::tmpl() const
{
	return d->propertyTemplate;
}

bool Property::isNull() const
{
	return d->value.isNull();
}

bool Property::isRichText() const
{
	return d->propertyTemplate.isRichText();
}

} // END namespace Kopete
