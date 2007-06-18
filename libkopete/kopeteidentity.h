/*
    kopeteidentity.h - Kopete Identity

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

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

#ifndef KOPETEIDENTITY_H
#define KOPETEIDENTITY_H

#include <kdemacros.h>
#include "kopeteglobal.h"
#include "kopeteinfopage.h"
#include "kopetepropertycontainer.h"
#include "kopete_export.h"

namespace Kopete
{

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 * An identity that might contain one or more accounts associated to it 
 */
class KOPETE_EXPORT Identity : public QObject, public PropertyContainer
{
	Q_OBJECT
public:	
	/**
	 * @brief A container for properties. 
	 *
	 * This class provides an interface for reading and writing properties.
	 */
	Identity(const QString &id);

	~Identity();

	/**
	 * Sets a property
	 * FIXME: this is just to fix the ambiguation of PropertyContainer::setProperty() 
	 * versus QObject::setProperty(). We should check for a better name
	 */
	void setProperty(const Kopete::ContactPropertyTmpl &tmpl, const QVariant &value);

	/**
	 * @brief Returns a list of user info widgets
	 *
	 * This function should be implemented in derived classes if you want to get custom 
	 * info pages to be displayed in the contact info dialog
	 */
	virtual Kopete::UI::InfoPage::List customInfoPages() const;

protected:
	virtual void notifyPropertyChanged( const QString &key, 
		const QVariant &oldValue, const QVariant &newValue );

private:
	class Private;
	Private *d;

};


} //END namespace Kopete

#endif

