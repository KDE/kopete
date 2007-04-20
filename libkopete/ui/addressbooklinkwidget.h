/*
    AddressBookLinkWidget

    A compact widget for showing and changing which address book item a
    particular Kopete::MetaContact is related to.

    Comprises a label showing the contact's name, a Clear button, and a Change
    button that usually invokes the AddressBookSelectorWidget.

    Copyright (c) 2006 by Will Stephenson <wstephenson@kde.org>

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

#ifndef ADDRESSBOOKLINKWIDGET_H
#define ADDRESSBOOKLINKWIDGET_H

#include <kabc/addressee.h>
#include <kopete_export.h>

#include "ui_addressbooklinkwidget_base.h"

namespace Kopete {
class MetaContact;

namespace UI {

/** 
 * A compact widget for showing and changing which address book item a
 * particular Kopete::MetaContact is related to.
 *
 * Comprises a label showing the contact's name, a Clear button, and a Change
 * button that usually invokes the AddressBookSelectorWidget.
 */
class KOPETE_EXPORT AddressBookLinkWidget : public QWidget, private Ui::AddressBookLinkWidgetBase
{
Q_OBJECT
public:
	explicit AddressBookLinkWidget( QWidget * parent = 0, const char * name = 0 );
	~AddressBookLinkWidget() {}
	/**
	 * Set the currently selected addressee
	 */
	void setAddressee( const KABC::Addressee& addr );
	/**
	 * Set the current metacontact so that the selector dialog may be preselected
	 */
	void setMetaContact( const Kopete::MetaContact * );
	/**
	 * Return the selected addressbook UID.
	 */
	QString uid() const;
signals:
	/**
	 * Emitted when the selected addressee changed.  addr is the KABC::Addressee that was selected. If addr.isEmpty() is empty, the clear button was clicked.
	 */
	void addresseeChanged( const KABC::Addressee& addr );
	
	/**
	 * Provided so you can perform your own actions instead of opening the AddressBookSelectorWidget.
	 * To do so, QObject::disconnect() btnSelectAddressee and connect your own slot to this signal
	 */
	void selectAddresseeClicked();
protected slots:
	void slotClearAddressee();
	void slotSelectAddressee();
private:
	const Kopete::MetaContact * mMetaContact;
	QString mSelectedUid;
};
} // end namespace UI
} // end namespace Kopete
#endif
