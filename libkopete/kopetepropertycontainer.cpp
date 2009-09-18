/*
    kopetepropertycontainer.cpp - Kopete Property Container

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @tiscalinet.be>

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

#include <QStringList>
#include <KDebug>

#include <kdeversion.h>

namespace Kopete {

class PropertyContainer::Private
{
public:
	Kopete::Property::Map properties;
};

PropertyContainer::PropertyContainer(QObject *parent)
: QObject(parent), d(new Private())
{
}

PropertyContainer::~PropertyContainer()
{
	delete d;
}

void PropertyContainer::serializeProperties(QMap<QString, QString> &serializedData) const
{

	Kopete::Property::Map::ConstIterator it;// = d->properties.ConstIterator;
	for (it=d->properties.constBegin(); it != d->properties.constEnd(); ++it)
	{
		if (!it.value().tmpl().persistent())
			continue;

		QVariant val = it.value().value();
		QString key = QString::fromLatin1("prop_%1_%2").arg(QString::fromLatin1(val.typeName()), it.key());

		serializedData[key] = val.toString();

	} // end for()
} // end serializeProperties()

void PropertyContainer::deserializeProperties( const QMap<QString, QString> &serializedData )
{
	QMap<QString, QString>::ConstIterator it;
	for ( it=serializedData.constBegin(); it != serializedData.constEnd(); ++it )
	{
		QString key = it.key();

		if ( !key.startsWith( QString::fromLatin1("prop_") ) ) // avoid parsing other serialized data
			continue;

		QStringList keyList = key.split( QChar('_'), QString::SkipEmptyParts );
		if( keyList.count() < 3 ) // invalid key, not enough parts in string "prop_X_Y"
			continue;

		key = keyList[2]; // overwrite key var with the real key name this property has
		QString type( keyList[1] ); // needed for QVariant casting

		QVariant variant( it.value() );
		if( !variant.convert(QVariant::nameToType(type.toLatin1())) )
		{
			kDebug(14010) <<
				"Casting QVariant to needed type FAILED" <<
				"key=" << key << ", type=" << type << endl;
			continue;
		}

		Kopete::PropertyTmpl tmpl = Kopete::Global::Properties::self()->tmpl(key);
		if( tmpl.isNull() )
		{
			kDebug( 14010 ) << "no PropertyTmpl defined for" \
				" key " << key << ", cannot restore persistent property" << endl;
			continue;
		}

		setProperty(tmpl, variant);
	} // end for()
}

QStringList PropertyContainer::properties() const
{
	return d->properties.keys();
}

bool PropertyContainer::hasProperty(const QString &key) const
{
	return d->properties.contains(key);
}

const Property &PropertyContainer::property(const QString &key) const
{
	if(hasProperty(key))
		return d->properties[key];
	else
		return Kopete::Property::null;
}

const Kopete::Property &PropertyContainer::property(
	const Kopete::PropertyTmpl &tmpl) const
{
	if(hasProperty(tmpl.key()))
		return d->properties[tmpl.key()];
	else
		return Kopete::Property::null;
}


void PropertyContainer::setProperty(const Kopete::PropertyTmpl &tmpl,
	const QVariant &value)
{
	if(tmpl.isNull() || tmpl.key().isEmpty())
	{
		kDebug(14000) <<
			"No valid template for property passed!" << endl;
		return;
	}

	if(value.isNull() || (value.canConvert(QVariant::String) && value.toString().isEmpty()))
	{
		removeProperty(tmpl);
	}
	else
	{
		QVariant oldValue = property(tmpl.key()).value();

		if(oldValue != value)
		{
			Kopete::Property prop(tmpl, value);
			d->properties.remove(tmpl.key());
			d->properties.insert(tmpl.key(), prop);

			emit propertyChanged(this, tmpl.key(), oldValue, value);
		}
	}
}

void PropertyContainer::removeProperty(const Kopete::PropertyTmpl &tmpl)
{
	if(!tmpl.isNull() && !tmpl.key().isEmpty() && hasProperty(tmpl.key()))
	{
		QVariant oldValue = property(tmpl.key()).value();
		d->properties.remove(tmpl.key());
		emit propertyChanged(this, tmpl.key(), oldValue, QVariant());
	}
}

} //END namespace Kopete

#include "kopetepropertycontainer.moc"

