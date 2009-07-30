/*
    editaccountwidget.h - Kopete Account Widget

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef EDITACCOUNTWIDGET_H
#define EDITACCOUNTWIDGET_H

#include "kopete_export.h"

namespace Kopete
{
class Account;
}

class KopeteEditAccountWidgetPrivate;

/**
 * @author Olivier Goffart <ogoffart@kde.org>
 *
 * This class is used by the protocol plugins to add specific protocol fields in the add account wizard,
 * or in the account preferences. If the given account is 0L, then you will have to create a new account
 * in @ref apply().
 *
 * Each protocol has to subclass this class, and the protocol's edit account page MUST inherits from
 * QWidget too.
 *
 * We suggest to put at least these fields in the page:
 *
 * - The User login, or the accountId. you can retrieve it from @ref Kopete::Account::accountId(). This
 *   field has to be marked as ReadOnly or shown as a label if the account already exists. Remember
 *   that accountId should be constant after account creation!
 *
 * - The password, and the remember password checkboxes.
 *
 * - The auto connect checkbox: use @ref Kopete::Account::excludeConnect() and
 *   @ref Kopete::Account::setExcludeConnect() to get/set this flag.
 *
 * You may add other custom fields, e.g. the nickname. To save or retrieve these settings use
 * @ref Kopete::ContactListElement::pluginData() with your protocol as plugin.
 */
class KOPETE_EXPORT KopeteEditAccountWidget
{
public:
	/**
	 * Constructor.
	 *
	 * If 'account' is 0L we are in the 'add account wizard', otherwise
	 * we are editing an existing account.
	 */
	explicit KopeteEditAccountWidget( Kopete::Account *account );

	/**
	 * Destructor
	 */
	virtual ~KopeteEditAccountWidget();

	/**
	 * This method must be reimplemented.
	 * It does the same as @ref AddContactPage::validateData()
	 */
	virtual bool validateData() = 0;

	/**
	 * Create a new account if we are in the 'add account wizard',
	 * otherwise update the existing account.
	 */
	virtual Kopete::Account *apply() = 0;

protected:
	/**
	 * Get a pointer to the Kopete::Account passed to the constructor.
	 * You can modify it any way you like, just don't delete the object.
	 */
	Kopete::Account * account() const;

	/**
	 * Set the account
	 */
	// FIXME: Is it possible to make the API not require this? A const account
	//        in this widget seems a lot cleaner to me - Martijn
	void setAccount( Kopete::Account *account );

private:
	KopeteEditAccountWidgetPrivate *d;
};

// vim: set noet ts=4 sts=4 sw=4:

#endif

