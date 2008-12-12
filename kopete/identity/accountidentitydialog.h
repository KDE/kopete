/*
    accountidentitydialog.h - Kopete Account Identity Dialog

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

#ifndef ACCOUNTIDENTITYDIALOG_H
#define ACCOUNTIDENTITYDIALOG_H

#include <KDialog>
#include <kopete_export.h>

namespace Kopete
{
class Identity;
class Account;
}

/**
 * This dialog is used to set the identity of an account 
 *
 * @author Gustavo Pichorim Boiko <gustavo.boiko@kemail.net>
 */
class KOPETE_IDENTITY_EXPORT AccountIdentityDialog : public KDialog
{
	Q_OBJECT

public:
	/**
	 * @brief Default constructor for the account identity dialog
	 *
	 * You can create a dialog for setting the identity of an account using this 
	 * constructor, but the easiest (and the recommended) way of calling it is by 
	 * using the static methods @ref changeAccountIdentity()
	 */
	explicit AccountIdentityDialog( QWidget *parent = 0 );
	~AccountIdentityDialog();

	/**
	 * Sets the account this dialog will change the identity of
	 */
	void setAccount(Kopete::Account *account);

	/**
	 * Sets the accounts this dialog will change the identity of
	 */
	void setAccounts(QList<Kopete::Account*> accountList);

	/**
	 * Sets the text message to be displayed in the dialog
	 */
	void setMessage(const QString &text);

	/**
	 * Sets the identity to be hidden
	 */
	void setHiddenIdentity(Kopete::Identity *ident);

	/**
	 * This is the easiest (and the recommended) way of changing the account identity.
	 * 
	 * Example:
	 * @code
	   if (AccountIdentityDialog::changeAccountIdentity(parent_widget, account))
		 // it succeeded on changing the account identity
	 * @endcode
	 *
	 * @param parent the parent widget to be used by the dialog
	 * @param account the @ref Kopete::Account that needs to change the identity
	 * @param hidden_ident the @ref Kopete::Identity that should be hidden in the 
	 *                     dialog (in case it is being removed we don't want any 
	 *                     account to be assigned to it). The default behavior is
	 *                     to show all identities
	 * @param message the text message to be displayed in the dialog
	 */
	static bool changeAccountIdentity( QWidget *parent, Kopete::Account *account, 
										Kopete::Identity *hidden_ident = 0,
										const QString &message = QString() );
	/**
	 * This behaves essentially like the above function, but can be used to change
	 * the identity of more than one @ref Kopete::Account at a time
	 *
	 * @param parent the parent widget to be used by the dialog
	 * @param accountList a list of accounts to change the identity of
	 * @param hidden_ident the @ref Kopete::Identity that should be hidden in the 
	 *                     dialog (in case it is being removed we don't want any 
	 *                     account to be assigned to it). The default behavior is
	 *                     to show all identities
	 * @param message the text message to be displayed in the dialog
	 */
	static bool changeAccountIdentity( QWidget *parent, QList<Kopete::Account*> accountList, 
										Kopete::Identity *hidden_ident = 0,
										const QString &message = QString() );

private slots:
	void slotValidate();
	void slotIdentityListDoubleClicked();
	void slotLoadIdentities();
	void slotLoadAccounts();

protected slots:
	virtual void accept();
	virtual void reject();

private:
	class Private;
	Private * const d;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

