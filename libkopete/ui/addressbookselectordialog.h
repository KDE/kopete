/*
    AddressBookSelectorDialog
    Nice Dialog to select a KDE AddressBook contact

    Copyright (c) 2005 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDRESSBOOKSELECTORDIALOG_H
#define ADDRESSBOOKSELECTORDIALOG_H

#include <kdemacros.h>
#include "kopete_export.h"
#include <kdialog.h>

#include <Qt3Support/Q3ListView>

namespace KABC
{
	class AddressBook;
	class Addressee;
}

namespace Kopete
{
namespace UI
{

class AddressBookSelectorWidget;

/**
 * A dialog that uses AddressBookSelectorWidget to allow the user
 * to select a KDE addressbook contact. If you want to use special features
 * you can use @see addressBookSelectorWidget() to get the pointer to the
 * AddressBookSelectorWidget object and set the desired options there.
 *
 * @author Duncan Mac-Vicar Prett <duncan@kde.org>
 */
class KOPETE_EXPORT AddressBookSelectorDialog : public KDialog
{
	Q_OBJECT
public:
	/**
	* The constructor of an empty AddressBookSelectorWidget
	*/
	AddressBookSelectorDialog( const QString &title, const QString &message,
	                           const QString &preSelectUid, QWidget *parent = 0L
	                          );
	/**
	* The destructor of the dialog
	*/
	~AddressBookSelectorDialog();

	/**
	* @returns the AddressBookSelectorWidget widget so that additional
	* parameters can be set by using it.
	*/
	AddressBookSelectorWidget *addressBookSelectorWidget() const
	{ return m_addressBookSelectorWidget; }

	/**
	* Creates a modal dialog, lets the user to select a addressbook contact
	* and returns when the dialog is closed.
	*
	* @returns the selected contact, or a null addressee if the user
	* pressed the Cancel button. Optionally
	*/
	static KABC::Addressee getAddressee( const QString &title, const QString &message, const QString &preSelectUid, QWidget *parent = 0L );

protected slots:
	virtual void accept();
	virtual void reject();
	void slotWidgetAddresseeListClicked( Q3ListViewItem *addressee );
protected:
	 AddressBookSelectorWidget *m_addressBookSelectorWidget;
};

} // namespace UI
} // namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:

