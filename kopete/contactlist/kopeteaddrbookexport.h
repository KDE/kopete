/*
    kopeteaddrbookexport.h - Kopete Online Status

    Logic for exporting data acquired from messaging systems to the 
    KDE address book

    Copyright (c) 2004 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEADDRBOOKEXPORT_H
#define KOPETEADDRBOOKEXPORT_H

#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>

#include "kopeteproperty.h"
#include "ui_kopeteaddrbookexportui.h"
#include <QPixmap>

class KDialog;
class K3ListBox;

namespace Kopete
{
class Contact;
class MetaContact;
}

class KopeteAddressBookExport : public QObject, private Ui::AddressBookExportUI
{
Q_OBJECT

public:
	KopeteAddressBookExport( QWidget *parent, Kopete::MetaContact *mc );
	~KopeteAddressBookExport();
	
	/** 
	 * Display the dialog
	 * @return a QDialog return code
	 */
	int showDialog();
	/**
	 * Export the data to KABC if changed, omitting any duplicates
	 */
	void exportData();
	
protected:
	/**
	 * Initialise the GUI labels with labels from KABC
	 */
	void initLabels();
	/**
	 * Populate the GUI with data from KABC
	 */
	void fetchKABCData();
	/**
	 * Populate a listbox with a given type of phone number
	 */
	void fetchPhoneNumbers( K3ListBox * listBox, KABC::PhoneNumber::Type type, uint& counter );
	/**
	 * Populate the GUI with data from IM systems
	 */
	void fetchIMData();
	/**
	 * Populate a combobox with a contact's IM data
	 */
	void populateIM( const Kopete::Contact *contact, const QPixmap &icon, 
			QComboBox *combo, const Kopete::PropertyTmpl &property );
	/**
	 * Populate a listbox with a contact's IM data
	 */
	void populateIM( const Kopete::Contact *contact, const QPixmap &icon, 
			K3ListBox *combo, const Kopete::PropertyTmpl &property );
	
	/** Check the selected item is not the first (existing KABC) item, or the same as it */
	bool newValue( QComboBox *combo );
	QStringList newValues( K3ListBox *listBox, uint counter );
	
	// the GUI
	QWidget *mParent;
	KDialog * mDialog;
	QPixmap mAddrBookIcon;
	AddressBookExportUI *mUI;
	Kopete::MetaContact *mMetaContact;
	KABC::AddressBook *mAddressBook;
	KABC::Addressee mAddressee;
	
	// counters tracking the number of KABC values where multiple values are possible in a single key
	uint numEmails, numHomePhones, numWorkPhones, numMobilePhones;

};

#endif
