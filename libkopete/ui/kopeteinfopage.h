/*
    kopeteinfopage.h - A base class for info pages

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

#ifndef KOPETEINFOPAGE_H
#define KOPETEINFOPAGE_H

#include <qwidget.h>
#include "kopete_export.h"

namespace Kopete
{
class PropertyContainer;

namespace UI
{

/**
 * \brief base widget for info pages
 * @author Gustavo Pichorim Boiko <gustavo.boiko AT kdemail.net>
 */
class KOPETE_EXPORT InfoPage : public QWidget
{
Q_OBJECT

public:
	typedef QList<InfoPage*> List;
	/**
	 * Constructor.
	 *
	 * The parameter @p properties is the property container this widget will display info
	 */
	InfoPage(const Kopete::PropertyContainer *properties);

	/**
	 * Destructor.
	 */
	virtual ~InfoPage();

	/**
	 * @brief Load info from the contact
	 *
	 * Should be reimplemented in the derived class
	 */
	virtual void load();

	/**
	 * @brief Save info to the contact
	 *
	 * Should be reimplemented in the derived class
	 */
	virtual void save();

	/**
	 * @brief The name of this page
	 *
	 * Should be reimplemented in the derived class
	 */
	virtual QString pageName() const;

/*protected slots:
	virtual void slotPropertyChanged( Kopete::Contact *contact, const QString &key,
									  const QVariant &oldValue, const QVariant &newValue );*/
	
protected:
	Kopete::PropertyContainer *m_properties;

};

} // namespace UI
} // namespace Kopete
#endif
// vim: set noet ts=4 sts=4 sw=4:
