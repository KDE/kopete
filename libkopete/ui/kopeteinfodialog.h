/*
    kopeteinfodialog.h - A dialog to configure information for contacts, 
                         metacontacts, groups, identities, etc

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

#ifndef KOPETEINFODIALOG_H
#define KOPETEINFODIALOG_H

#include <KDialog>
#include "kopete_export.h"

namespace Kopete
{
class PropertyContainer;

namespace UI
{

/**
 * \brief a dialog for configuring user information
 *
 * A user can be a contact from the contact list, a metacontact or an identity.
 * @author Gustavo Pichorim Boiko <gustavo.boiko AT kdemail.net>
 */
class KOPETE_EXPORT InfoDialog : public KDialog
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param properties is the property container this widget will display info
	 * @param title the title to be shown in the dialog
	 * @param icon the name of the icon to be used
	 */
	InfoDialog(const Kopete::PropertyContainer *properties, 
			   const QString &title = QString(), const QString &icon = QString());
	/**
	 * Constructor.
	 *
	 * @param properties is the property container this widget will display info
	 * @param title the title to be shown in the dialog
	 * @param icon the icon to be used
	 */
	InfoDialog(const Kopete::PropertyContainer *properties, 
			   const QString &title, const KIcon &icon);

	/**
	 * Destructor.
	 */
	~InfoDialog();

	void setTitle(const QString &title);
	void setIcon(const QString &icon);
	void setIcon(const KIcon &icon);

private slots:
	void slotSave();

private:
	class Private;
	Private *d;

};

} // namespace UI
} // namespace Kopete
#endif
// vim: set noet ts=4 sts=4 sw=4:
