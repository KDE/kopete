/*
    addidentitydialog.h - Kopete Add Identity Dialog

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDIDENTITYDIALOG_H
#define ADDIDENTITYDIALOG_H

#include <KDialog>
#include <kopete_export.h>
#include "ui_addidentitybase.h"

namespace Kopete
{
class Identity;
}

/**
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>
 *
 * This dialog is used to add a new identity to 
 */
class KOPETE_IDENTITY_EXPORT AddIdentityDialog : public KDialog
{
	Q_OBJECT

public:
	/**
	 * @brief Default constructor for the add identity dialog
	 *
	 * You can create a dialog for adding identity using this constructor, but
	 * the easiest (and the recommended) way of calling it is by using the static
	 * method @ref getIdentity();
	 */
	explicit AddIdentityDialog( QWidget *parent = 0 );
	~AddIdentityDialog();

	/**
	 * This method should be called after the dialog was accepted to get the identity 
	 * that was just created. The identity will already have the properties cloned 
	 * (if the duplicate identity option was chosen), but it is not automatically
	 * registered in the @ref Kopete::IdentityManager, because the caller may want
	 * to do some operations before registering.
	 */
	Kopete::Identity *identity();

	/**
	 * This is the easiest (and the recommended) way of adding a new identity.
	 * 
	 * Example:
	 * @code
	   Kopete::Identity *id = AddIdentityDialog::getIdentity(parent_widget);
	   //the id will point to the newly added identity, or it will be 0 if the
	   //user has not accepted the dialog
	 * @endcode
	 */
	static Kopete::Identity *getIdentity( QWidget *parent = 0 );

private slots:
	void slotValidate();
	void slotIdentityListDoubleClicked();

protected slots:
	virtual void accept();
	virtual void reject();

private:
	class Private;
	Private *d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

