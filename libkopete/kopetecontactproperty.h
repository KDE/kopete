/*
    kopetecontactproperty.h

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

#ifndef _KOPETECONTACTPROPERTY_H_
#define _KOPETECONTACTPROPERTY_H_

#include <qvariant.h>

namespace Kopete
{

struct ContactPropertyTmplPrivate;

/**
 * @author Stefan Gehn <metz AT gehn.net>
 *
 * The template class for registering properties in Kopete
 * You need to use this if you want to set properties for a
 * KopeteContact
 **/
class ContactPropertyTmpl
{
	public:
		/**
		 * Constructor only used for empty ContactPropertyTmpl objects
		 *
		 * Note: Only useful for the Null object
		 **/
		ContactPropertyTmpl();

		ContactPropertyTmpl(const QString &key,
			const QString &label,
			const QString &icon = QString::null,
			bool persistent = false);

		ContactPropertyTmpl(const ContactPropertyTmpl &other);

		~ContactPropertyTmpl();

		ContactPropertyTmpl &operator=(const ContactPropertyTmpl &other);

		bool operator==(const ContactPropertyTmpl &other) const;
		bool operator!=(const ContactPropertyTmpl &other) const;

		/**
		 * Getter for the key properties based on this template will be
		 * stored with
		 **/
		const QString &key() const;

		/**
		 * Getter for i18ned label
		 **/
		const QString &label() const;

		/**
		 * Getter for icon to show aside or instead of @p label()
		 **/
		const QString &icon() const;

		/**
		 * Returns true if this property should be saved across Kopete
		 * sessions, false otherwise.
		 **/
		bool persistent() const;

		static ContactPropertyTmpl null;

		/**
		 * Returns true if this object is an empty template
		 **/
		bool isNull() const;

		typedef QMap<QString, ContactPropertyTmpl>  Map;

	private:
		ContactPropertyTmplPrivate *d;
};


/**
 * @author Stefan Gehn <metz AT gehn.net>
 *
 * A data container for whatever information kopete or any of its
 * plugins want to store for a KopeteContact
 **/
class ContactProperty
{
	// TODO: Add d-pointer !
	public:
		/**
		 * Constructor only used for empty ContactProperty objects
		 *
		 * Note: you cannot set label or value later on!
		 **/
		ContactProperty();

		/**
		 * @param value The value this Property holds
		 **/
		ContactProperty(const ContactPropertyTmpl &tmpl, const QVariant &value);

		~ContactProperty();

		/**
		 * Getter for this properties template
		 **/
		const ContactPropertyTmpl &tmpl() const;

		/**
		 * Getter for this properties value
		 **/
		const QVariant &value() const;

		/**
		 * The null, i.e. empty, ContactProperty
		 */
		static ContactProperty null;

		/**
		 * Returns true if this object is an empty Property (i.e. it holds no
		 * value), false otherwise.
		 **/
		bool isNull() const;

		/**
		 * A map of key,ContactProperty items
		 **/
		typedef QMap<QString, ContactProperty> Map;

	private:
		QVariant mValue;
		ContactPropertyTmpl mTemplate;
};

} // END namespace Kopete

#endif //_KOPETECONTACTPROPERTY_H_
