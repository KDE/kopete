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

/**
 * @author Stefan Gehn <metz AT gehn.net>
 *
 * This class is a data container for whatever information
 * kopete or any of its plugins want to store for a KopeteContact
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
		 * @param label The i18ned label for this Property, used for display
		 * @param icon  The icon to show instead or aside of the i18ned label
		 **/
		ContactProperty(const QVariant &value, const QString &label, const QString &icon = QString::null);
		~ContactProperty();

		/**
		 * Getter for i18ned label
		 **/
		const QString &label() const;

		/**
		 * Getter for icon to show aside or instead of @p label()
		 **/
		const QString &icon() const;

		/**
		 * Getter for this properties value
		 **/
		const QVariant &value() const;

		static ContactProperty null;

		/**
		 * Returns true if this object is an empty Property (i.e. it holds no value)
		 **/
		bool isNull() const;

		typedef QMap<QString, ContactProperty>  Map;

	private:
		QString mLabel;
		QVariant mValue;
		QString mIcon;
};

} // END namespace Kopete

#endif //_KOPETECONTACTPROPERTY_H_
