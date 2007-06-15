/*
    generalinfopage.h - Display general info of a property container

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

#ifndef GENERALINFOPAGE_H
#define GENERALINFOPAGE_H

#include "kopeteinfopage.h"
#include "kopete_export.h"

namespace Kopete
{
class PropertyContainer;

namespace UI
{

/**
 * \brief a widget for showing general info of a property container 
 * @author Gustavo Pichorim Boiko <gustavo.boiko AT kdemail.net>
 */
class KOPETE_EXPORT GeneralInfoPage : public InfoPage
{
	Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * The parameter @p contact is the contact this widget will display info
	 */
	GeneralInfoPage(const Kopete::PropertyContainer *properties);

	/**
	 * Destructor.
	 */
	virtual ~GeneralInfoPage();

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

/*
protected slots:
	virtual void slotPropertyChanged( Kopete::Contact *contact, const QString &key,
									  const QVariant &oldValue, const QVariant &newValue );
*/
	
private:
	class Private;
	Private *d;

};

} // namespace UI
} // namespace Kopete
#endif
// vim: set noet ts=4 sts=4 sw=4:
