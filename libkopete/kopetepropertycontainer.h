/*
    kopetepropertycontainer.h - Kopete Property Container

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>

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

#ifndef KOPETEPROPERTYCONTAINER_H
#define KOPETEPROPERTYCONTAINER_H

#include <QtCore/QObject>

#include <kdemacros.h>
#include "kopeteglobal.h"
#include "kopete_export.h"

namespace Kopete
{

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 * This class abstracts a generic contact
 * Use it for inserting contacts in the contact list for example.
 */
class KOPETE_EXPORT PropertyContainer : public QObject
{
	Q_OBJECT

public:	
	/**
	 * @brief A container for properties. 
	 *
	 * This class provides an interface for reading and writing properties.
	 */
	PropertyContainer( QObject *parent = 0 );

	virtual ~PropertyContainer();

	/**
	 * @brief Serialize the persistent properties for storage in the contact list.
	 *
	 * Does the same as @ref serialize() does but for KopeteContactProperties
	 * set in this contact with their persistency flag turned on.
	 * In contrary to @ref serialize() this does not need to be reimplemented.
	 *
	 */
	void serializeProperties(QMap<QString, QString> &serializedData) const;

	/**
	 * @brief Deserialize the contacts persistent properties
	 */
	void deserializeProperties(const QMap<QString, QString> &serializedData);

	/**
	 * @return A QStringList containing all property keys
	 **/
	QStringList properties() const;

	/**
	 * Check for existence of a certain property stored
	 * using "key".
	 * \param key the property to check for
	 **/
	bool hasProperty(const QString &key) const;

	/**
	 * \brief Get the value of a property with key "key".
	 *
	 * If you don't know the type of the returned QVariant, you will need
	 * to check for it.
	 * \return the value of the property
	 **/
	const Kopete::Property &property(const QString &key) const;
	const Kopete::Property &property(const Kopete::PropertyTmpl &tmpl) const;

	/**
	 * \brief Add or Set a property for this contact.
	 *
	 * @param tmpl The template this property is based on, key, label etc. are
	 * taken from this one
	 * @param value The value to store
	 *
	 * \note Setting a NULL value or an empty QString castable value
	 * removes the property if it already existed.
	 * <b>Don't</b> abuse this for property-removal, instead use
	 * @ref removeProperty() if you want to remove on purpose.
	 * The Removal is done to clean up the list of properties and to purge them
	 * from UI.
	 **/
	void setProperty(const Kopete::PropertyTmpl &tmpl, const QVariant &value);

	/**
	 * \brief Remove a property if it exists
	 *
	 * @param tmpl the template this property is based on
	 **/
	void removeProperty(const Kopete::PropertyTmpl &tmpl);

signals:
	void propertyChanged( Kopete::PropertyContainer *container, const QString &key,
		const QVariant &oldValue, const QVariant &newValue );

private:
	class Private;
	Private * const d;

};


} //END namespace Kopete

#endif


