/*
    kopetepropertycontainer.cpp - Kopete Contact

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
#include "generalinfopage.h"
#include "kopeteinfopage.h"

#include <QStringList>
#include <KDebug>

#include <kdeversion.h>

namespace Kopete {

class PropertyContainer::Private
{
public:
	Kopete::ContactProperty::Map properties;
};

PropertyContainer::PropertyContainer()
{
	d = new Private;

}

PropertyContainer::~PropertyContainer()
{
	delete d;
}

void PropertyContainer::serializeProperties(QMap<QString, QString> &serializedData)
{

	Kopete::ContactProperty::Map::ConstIterator it;// = d->properties.ConstIterator;
	for (it=d->properties.begin(); it != d->properties.end(); ++it)
	{
		if (!it.value().tmpl().persistent())
			continue;

		QVariant val = it.value().value();
		QString key = QString::fromLatin1("prop_%1_%2").arg(QString::fromLatin1(val.typeName()), it.key());

		serializedData[key] = val.toString();

	} // end for()
} // end serializeProperties()

void PropertyContainer::deserializeProperties(
	QMap<QString, QString> &serializedData )
{
	QMap<QString, QString>::ConstIterator it;
	for ( it=serializedData.begin(); it != serializedData.end(); ++it )
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
			kDebug(14010) << k_funcinfo <<
				"Casting QVariant to needed type FAILED" <<
				"key=" << key << ", type=" << type << endl;
			continue;
		}

		Kopete::ContactPropertyTmpl tmpl = Kopete::Global::Properties::self()->tmpl(key);
		if( tmpl.isNull() )
		{
			kDebug( 14010 ) << k_funcinfo << "no ContactPropertyTmpl defined for" \
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

const ContactProperty &PropertyContainer::getProperty(const QString &key) const
{
	if(hasProperty(key))
		return d->properties[key];
	else
		return Kopete::ContactProperty::null;
}

const Kopete::ContactProperty &PropertyContainer::getProperty(
	const Kopete::ContactPropertyTmpl &tmpl) const
{
	if(hasProperty(tmpl.key()))
		return d->properties[tmpl.key()];
	else
		return Kopete::ContactProperty::null;
}


void PropertyContainer::setProperty(const Kopete::ContactPropertyTmpl &tmpl,
	const QVariant &value)
{
	if(tmpl.isNull() || tmpl.key().isEmpty())
	{
		kDebug(14000) << k_funcinfo <<
			"No valid template for property passed!" << endl;
		return;
	}

	if(value.isNull() || value.canConvert(QVariant::String) && value.toString().isEmpty())
	{
		removeProperty(tmpl);
	}
	else
	{
		QVariant oldValue = getProperty(tmpl.key()).value();

		if(oldValue != value)
		{
			Kopete::ContactProperty prop(tmpl, value);
			d->properties.remove(tmpl.key());
			d->properties.insert(tmpl.key(), prop);

			notifyPropertyChanged(tmpl.key(), oldValue, value);
		}
	}
}

void PropertyContainer::removeProperty(const Kopete::ContactPropertyTmpl &tmpl)
{
	if(!tmpl.isNull() && !tmpl.key().isEmpty())
	{

		QVariant oldValue = getProperty(tmpl.key()).value();
		d->properties.remove(tmpl.key());
		notifyPropertyChanged(tmpl.key(), oldValue, QVariant());
	}
}


void PropertyContainer::notifyPropertyChanged( const QString &key, 
		const QVariant &oldValue, const QVariant &newValue )
{
	// do nothing
}

Kopete::UI::InfoPage::List PropertyContainer::infoPages() const
{
	// TODO implement
	Kopete::UI::InfoPage::List list;
	list.append( new Kopete::UI::GeneralInfoPage(this) );
	list.append( new Kopete::UI::GeneralInfoPage(this) );
	return list + customInfoPages();
}

Kopete::UI::InfoPage::List PropertyContainer::customInfoPages() const
{
	// TODO implement
	Kopete::UI::InfoPage::List list;
	return list;
}

} //END namespace Kopete

#include "kopetepropertycontainer.moc"

