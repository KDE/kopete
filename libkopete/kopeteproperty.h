/*
    kopeteproperty.h

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

#ifndef _KOPETEPROPERTY_H_
#define _KOPETEPROPERTY_H_

#include <QtCore/QVariant>
#include <QtCore/QFlags>

#include "kopete_export.h"

namespace Kopete
{

/**
 * @author Stefan Gehn <metz AT gehn.net>
 * @author Michaël Larouche <larouche@kde.org>
 *
 * The template class for registering properties in Kopete
 * You need to use this if you want to set properties for a
 * Kopete::Contact
 **/
class KOPETE_EXPORT PropertyTmpl
{
	public:
		enum PropertyOption 
		{ 
			NoProperty = 0x0,
			PersistentProperty = 0x1, 
			RichTextProperty = 0x2, 
			PrivateProperty = 0x4 
		};
		Q_DECLARE_FLAGS(PropertyOptions, PropertyOption)

		/**
		 * Constructor only used for empty PropertyTmpl objects
		 *
		 * Note: Only useful for the null object
		 **/
		PropertyTmpl();

		/**
		 * Constructor
		 * @param key internal unique key for this template
		 * @param label a label to show for properties based on this template
		 * @param icon name of the icon to show for properties based on this template
		 * @param options set the options for that property. See PropertyOption enum.
		 **/
		PropertyTmpl( const QString &key,
			const QString &label,
			const QString &icon = QString(),
			PropertyOptions options = NoProperty);

		/**
		 * Copy constructor
		 **/
		PropertyTmpl(const PropertyTmpl &other);

		/** Destructor */
		~PropertyTmpl();

		PropertyTmpl &operator=(const PropertyTmpl &other);

		bool operator==(const PropertyTmpl &other) const;
		bool operator!=(const PropertyTmpl &other) const;

		/**
		 * Getter for the unique key. Properties based on this template will be
		 * stored with this key
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
	 	 * Return the options for that property.
		 */
		PropertyOptions options() const;

		/**
		 * Returns true if properties based on this template should
		 * be saved across Kopete sessions, false otherwise.
		 **/
		bool persistent() const;

		/**
		 * Returns true if properties based on this template are HTML formatted
		 **/
		bool isRichText() const;

		/**
		 * Returns true if properties based on this template are invisible to the user
		 **/
		bool isPrivate() const;

		/**
		 * An empty template, check for it using isNull()
		 */
		static PropertyTmpl null;

		/**
		 * Returns true if this object is an empty template
		 **/
		bool isNull() const;

		/**
		 * A Map of QString and PropertyTmpl objects, based on QMap
		 **/
		typedef QMap<QString, PropertyTmpl>  Map;

	private:
		class Private;
		Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PropertyTmpl::PropertyOptions)

/**
 * @author Stefan Gehn <metz AT gehn.net>
 *
 * A data container for whatever information Kopete or any of its
 * plugins want to store for a Kopete::Contact
 **/
class KOPETE_EXPORT Property
{
	public:
		/**
		 * Constructor only used for empty Property objects
		 *
		 * Note: you cannot set a label or value later on!
		 **/
		Property();

		/**
		 * @param tmpl The contact property template this property is based on
		 * @param value The value this Property holds
		 **/
		Property(const PropertyTmpl &tmpl, const QVariant &value);

		/**
		 * Copy constructor
		 **/
		Property(const Property &other);

		/** Destructor **/
		~Property();

		Property &operator=(const Property &other);

		/**
		 * Getter for this properties template
		 **/
		const PropertyTmpl &tmpl() const;

		/**
		 * Getter for this properties value
		 **/
		const QVariant &value() const;

		/**
		 * The null, i.e. empty, Property
		 */
		static Property null;

		/**
		 * Returns true if this object is an empty Property (i.e. it holds no
		 * value), false otherwise.
		 **/
		bool isNull() const;

		/**
		 * Returns true if this property is HTML formatted
		 **/
		bool isRichText() const;

		/**
		 * A map of key,Property items
		 **/
		typedef QMap<QString, Property> Map;

	private:
		class Private;
		Private * const d;
};

} // END namespace Kopete

#endif //_KOPETEPROPERTY_H_
