/*
    AddressBookSelectorWidget
    Copyright (c) 2005 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Based on LinkAddressBookUI whose code was shamelessly stolen from 
    kopete's add new contact wizard, used in Konversation, and then 
    reappropriated by Kopete.

    LinkAddressBookUI:
    Copyright (c) 2004 by John Tapsell           <john@geola.co.uk>
    Copyright (c) 2003-2005 by Will Stephenson   <will@stevello.free-online.co.uk>
    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef AddressBookSelectorWidget_H
#define AddressBookSelectorWidget_H

#include <kabc/addressbook.h>

#include <kdemacros.h>
#include "kopete_export.h"

#include "ui_addressbookselectorwidget_base.h"

namespace KABC {
		class AddressBook;
		class Addressee;
}

namespace Kopete
{
namespace UI
{

class KOPETE_EXPORT AddressBookSelectorWidget : public QWidget, private Ui::AddressBookSelectorWidget_Base
{
	Q_OBJECT
public:
	explicit AddressBookSelectorWidget( QWidget *parent = 0, const char *name  = 0 );
	~AddressBookSelectorWidget();
	KABC::Addressee addressee();
	/**
	 * sets the widget label message
	 * example: Please select a contact
	 * or, Choose a contact to delete
	 */
	void setLabelMessage( const QString &msg );
	/**
	 * pre-selects a contact
	 */
	void selectAddressee( const QString &uid );
	/**
	 * @return true if there is a contact selected
	 */
	bool addresseeSelected();
	
private:
	KABC::AddressBook * m_addressBook;
	KABC::Addressee m_addressee;
	
protected slots:
	void slotAddAddresseeClicked();
	/**
	 * Utility function, populates the addressee list
	 */
	void slotLoadAddressees();
signals:
	void addresseeListClicked( Q3ListViewItem *addressee );
	void addAddresseeClicked();
};

} // namespace UI
} // namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
