/*
    editaccountwidget.h - Kopete Account Widget

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

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

namespace Kopete
{
class Account;
}

class KopeteEditAccountWidgetPrivate;

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
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
 * - The password, and the remember password checkboxes. When creating the widget you have to get if
 *   the password is remembered, with @ref Kopete::Account::rememberPassword(). If it returns true
 *   you can use @ref Kopete::Account::password() to get the password field.
 *
 *   WARNING: Do not use password() if the password is not remembered, or Kopete will popup a dialog
 *            to ask the password!
 *
 *   To set the password use @ref Kopete::Account::setPassword(). If the user has not selected the
 *   'remember password' checkbox, set the password to QString::null.
 *
 * - The auto connect checkbox: use @ref Kopete::Account::autoLogin() and
 *   @ref Kopete::Account::setAutoLogin() to get/set this flag.
 *
 * You may add other custom fields, e.g. the nickname. To save or retrieve these settings use
 * @ref Kopete::PluginDataObject::pluginData() with your protocol as plugin.
 */
class KopeteEditAccountWidget
{
public:
	/**
	 * Constructor.
	 *
	 * If 'account' is 0L we are in the 'add account wizard', otherwise
	 * we are editing an existing account.
	 */
	KopeteEditAccountWidget( Kopete::Account *account );

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

