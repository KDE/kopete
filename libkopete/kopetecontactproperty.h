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

#ifndef _KOPETECONTACTPROPERTY_H_
#define _KOPETECONTACTPROPERTY_H_

#include <qvariant.h>

/**
 * @author Stefan Gehn <metz AT gehn.net>
 *
 * This class is a data container for whatever information
 * kopete or any of its plugins want to store for a KopeteContact
 */
class KopeteContactProperty
{
	// TODO: Add d-pointer !
	public:
		/**
		 * Constructor only used for empty KopeteContactProperty objects
		 *
		 * Note: you cannot set label or value or label later on!
		 **/
		KopeteContactProperty();

		/**
		 * @param label The i18ned label for this Property, used for display
		 * @param value The value this Property holds
		 **/
		KopeteContactProperty(const QString &label, const QVariant &value);
		~KopeteContactProperty();

		/**
		 * Getter for i18ned label
		 **/
		const QString &label() const;

		/**
		 * Getter for this properties value
		 **/
		const QVariant &value() const;

		static KopeteContactProperty null;

		/**
		 * Returns true if this object is an empty Property (i.e. it holds no value)
		 **/
		bool isNull() const;

	private:
		QString mLabel;
		QVariant mValue;
};

#endif //_KOPETECONTACTPROPERTY_H_
