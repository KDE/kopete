/*
    editaccountwidget.h - Kopete Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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

class KopeteAccount;

#include "kopeteaccount.h"

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 *
 * This class is used by protocol to add specifics protocol fields in the add account wizzard, or in the account preferences.
 * if the given accountis 0L, then you will have to create a new acvcount in @ref apply
 *
 * protocol has to subclasses it, and the protocol adet account page MUST inherits from QWidget too.
 *
 * We suggest in this page to put at least some fields:
 *
 * - The User login, or the accountId. you can retrieve it from @ref KopeteAccount::accountId()  But this field has
 *     to be marked as ReadOnly when the account already exists. Remember that accountId should be constant
 *
 * - The password, and the remember password checkboxes.  First, you have to get if the password is remember, with @ref KopeteAccount::rememberPassword
 *    if it return true, you can use @ref KopeteAccount::password() to set the password field. WARNING: do not use password if the password is not
 *    remember, or kopete will popup a dialog to ask the password.
 *    To set the password use @ref KopeteAccount::setPassword()  if the user has unselected the remember password checkboxes, set QString::null as password
 *
 * - The auto connect checkboxe: use @ref KopeteAccount::autoConnect and @ref KopeteAccount::setAutoConnect  get/set this flag
 *
 * You may add some other custom fields, for example, the nickname. to save or retrieve theses settings use @ref KopetePluginDataObject::setPluginData or
 * @ref KopetePluginDataObject::pluginData with your protocol as plugin
 *
 */
class EditAccountWidget
{
	public:
		/**
		 * Constructor. if 'account' is 0L, then, we are in the addAccountWizzard,
		 * if it is an account, we are editing this account
		 */
		EditAccountWidget(KopeteAccount *account);

		/**
		 * This method must be reimplemented.
		 * It has the same action as the @ref AddContactPage::validateData()
		 */
		virtual bool validateData()=0;

		/**
		 * This must create the account if we are in the addAccountWizard,
		 * or update it if we are in the edit page.
		 */
		virtual KopeteAccount *apply()=0;

	protected :
		/**
		 * this is simply a link to tha account given in the constructor. do with it what you want.
		 */
		KopeteAccount *m_account;
};
#endif


