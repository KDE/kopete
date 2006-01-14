/*
    kopeteaddrbookexport.cpp - Kopete Online Status

    Logic for exporting data acquired from messaging systems to the 
    KDE address book

    Copyright (c) 2004 by Will Stephenson <lists@stevello.free-online.co.uk>

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

#include <kabc/phonenumber.h>
#include <qcombobox.h>
#include <qlabel.h>

#include <kdialogbase.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

#include "kopeteaddrbookexport.h"
#include "kopeteaddrbookexportui.h"

KopeteAddressBookExport::KopeteAddressBookExport( QWidget *parent, Kopete::MetaContact *mc ) : QObject( parent )
{
	// instantiate dialog and populate widgets
	mParent = parent;
	mAddressBook = KABC::StdAddressBook::self();
	mMetaContact = mc;
}

KopeteAddressBookExport::~KopeteAddressBookExport()
{

}

void KopeteAddressBookExport::initLabels()
{
	if ( !mAddressee.isEmpty() )
	{
		mUI->mLblFirstName->setText( mAddressee.givenNameLabel() );
		mUI->mLblLastName->setText( mAddressee.familyNameLabel() );
		mUI->mLblEmail->setText( mAddressee.emailLabel() );
		mUI->mLblUrl->setText( mAddressee.urlLabel() );
		mUI->mLblHomePhone->setText( mAddressee.homePhoneLabel() );
		mUI->mLblWorkPhone->setText( mAddressee.businessPhoneLabel() );
		mUI->mLblMobilePhone->setText( mAddressee.mobilePhoneLabel() );
	}
}

void KopeteAddressBookExport::fetchKABCData()
{
	if ( !mAddressee.isEmpty() )
	{
		mAddrBookIcon = SmallIcon( "kaddressbook" );
		
		// given name
		QString given = mAddressee.givenName();
		if ( !given.isEmpty() )
			mUI->mFirstName->insertItem( mAddrBookIcon, given );
		else
			mUI->mFirstName->insertItem( mAddrBookIcon, i18n("<Not Set>") );
			
		// family name
		QString family = mAddressee.familyName();
		if ( !family.isEmpty() )
			mUI->mLastName->insertItem( mAddrBookIcon, family );
		else
			mUI->mLastName->insertItem( mAddrBookIcon, i18n("<Not Set>") );
		
		// url
		QString url = mAddressee.url().url();
		if ( !url.isEmpty() )
			mUI->mUrl->insertItem( mAddrBookIcon, url );
		else
			mUI->mUrl->insertItem( mAddrBookIcon, i18n("<Not Set>") );
		
		// emails
		QStringList emails = mAddressee.emails();
		numEmails = emails.count();
		for ( QStringList::Iterator it = emails.begin(); it != emails.end(); ++it )
			mUI->mEmails->insertItem( mAddrBookIcon, *it );
		if ( numEmails == 0 )
		{
			mUI->mEmails->insertItem( mAddrBookIcon, i18n("<Not Set>") );
			numEmails = 1;
		}
		
		// phone numbers
		fetchPhoneNumbers( mUI->mHomePhones, KABC::PhoneNumber::Home, numHomePhones );
		fetchPhoneNumbers( mUI->mWorkPhones, KABC::PhoneNumber::Work, numWorkPhones );
		fetchPhoneNumbers( mUI->mMobilePhones, KABC::PhoneNumber::Cell, numMobilePhones );
	}
}

void KopeteAddressBookExport::fetchPhoneNumbers( KListBox * listBox, int type, uint& counter )
{
	KABC::PhoneNumber::List phones = mAddressee.phoneNumbers( type );
	counter = phones.count();
	KABC::PhoneNumber::List::Iterator it;
	for ( it = phones.begin(); it != phones.end(); ++it )
		listBox->insertItem( mAddrBookIcon, (*it).number() );
	if ( counter == 0 )
	{
		listBox->insertItem( mAddrBookIcon, i18n("<Not Set>") );
		counter = 1;
	}
}

void KopeteAddressBookExport::fetchIMData()
{	
	QPtrList<Kopete::Contact> contacts = mMetaContact->contacts();
	QPtrListIterator<Kopete::Contact> cit( contacts );
	for( ; cit.current(); ++cit )
	{
		// for each contact, get the property content
		Kopete::Contact* c = cit.current();
		QPixmap contactIcon = c->account()->accountIcon( 16 );
		// given name
		populateIM( c, contactIcon, mUI->mFirstName, Kopete::Global::Properties::self()->firstName() );
		// family name
		populateIM( c, contactIcon, mUI->mLastName, Kopete::Global::Properties::self()->lastName() );
		// url
		// TODO: make URL/homepage a global template, currently only in IRC channel contact
		// emails
		populateIM( c, contactIcon, mUI->mEmails, Kopete::Global::Properties::self()->emailAddress() );
		// home phone
		populateIM( c, contactIcon, mUI->mHomePhones, Kopete::Global::Properties::self()->privatePhone() );
		// work phone
		populateIM( c, contactIcon, mUI->mWorkPhones, Kopete::Global::Properties::self()->workPhone() );
		// mobile phone
		populateIM( c, contactIcon, mUI->mMobilePhones, Kopete::Global::Properties::self()->privateMobilePhone() );
	}
}

void KopeteAddressBookExport::populateIM( const Kopete::Contact *contact, const QPixmap &icon, QComboBox *combo, const Kopete::ContactPropertyTmpl &property )
{
	Kopete::ContactProperty prop = contact->property( property );
	if ( !prop.isNull() )
	{
		combo->insertItem( icon, prop.value().toString() );
	}	
}

void KopeteAddressBookExport::populateIM( const Kopete::Contact *contact, const QPixmap &icon, KListBox *listBox, const Kopete::ContactPropertyTmpl &property )
{
	Kopete::ContactProperty prop = contact->property( property );
	if ( !prop.isNull() )
	{
		listBox->insertItem( icon, prop.value().toString() );
	}	
}

int KopeteAddressBookExport::showDialog()
{
	mAddressee = mAddressBook->findByUid( mMetaContact->metaContactId() );
	if ( !mAddressee.isEmpty() )
	{
		numEmails = 0;
		numHomePhones = 0;
		numWorkPhones = 0;
		numMobilePhones = 0;
		mDialog = new KDialogBase( mParent, "addressbookexportdialog", true, i18n("Export to Address Book"), KDialogBase::Ok|KDialogBase::Cancel );
		mUI = new AddressBookExportUI( mDialog );
		mDialog->setMainWidget( mUI );
		mDialog->setButtonOK( KGuiItem( i18n( "Export" ), 
							  QString::null, i18n( "Set address book fields using the selected data from Kopete" ) ) ); 

		initLabels();
		// fetch existing data from kabc
		fetchKABCData();
		// fetch data from contacts
		fetchIMData();
	
		return mDialog->exec();
	}
	else 
		return QDialog::Rejected;
}

void KopeteAddressBookExport::exportData()
{
	// write the data from the widget to KABC
	// update the Addressee
	// first name
	bool dirty = false;
	if ( newValue( mUI->mFirstName ) )
	{
		dirty = true;
		mAddressee.setGivenName( mUI->mFirstName->currentText() );
	}
	// last name
	if ( newValue( mUI->mLastName ) )
	{
		dirty = true;
		mAddressee.setFamilyName( mUI->mLastName->currentText() );
	}
	// url
	if ( newValue( mUI->mUrl ) )
	{
		dirty = true;
		mAddressee.setUrl( KURL( mUI->mUrl->currentText() ) );
	}

	QStringList newVals;
	// email
	newVals = newValues( mUI->mEmails, numEmails );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertEmail( *it );
	}
	// home phone
	newVals = newValues( mUI->mHomePhones, numHomePhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KABC::PhoneNumber( *it, KABC::PhoneNumber::Home ) );
	}
	// work phone
	newVals = newValues( mUI->mWorkPhones, numWorkPhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KABC::PhoneNumber( *it, KABC::PhoneNumber::Work ) );
	}
	// mobile
	newVals = newValues( mUI->mMobilePhones, numMobilePhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KABC::PhoneNumber( *it, KABC::PhoneNumber::Cell ) );
	}
	
	if ( dirty )
	{
		// write the changed addressbook
		mAddressBook->insertAddressee( mAddressee );
	
		KABC::Ticket *ticket = mAddressBook->requestSaveTicket();
		if ( !ticket )
			kdWarning( 14000 ) << k_funcinfo << "WARNING: Resource is locked by other application!" << endl;
		else
		{
			if ( !mAddressBook->save( ticket ) )
			{
				kdWarning( 14000 ) << k_funcinfo << "ERROR: Saving failed!" << endl;
				mAddressBook->releaseSaveTicket( ticket );
			}
		}
		kdDebug( 14000 ) << k_funcinfo << "Finished writing KABC" << endl;
	}
}

bool KopeteAddressBookExport::newValue( QComboBox *combo )
{
	// all data in position 0 is from KABC, so if position 0 is selected,
	// or if the selection is the same as the data at 0, return false
	return !( combo->currentItem() == 0 || 
			( combo->text( combo->currentItem() ) == combo->text( 0 ) ) );
}

QStringList KopeteAddressBookExport::newValues( KListBox *listBox, uint counter )
{
	QStringList newValues;
	// need to iterate all items except those from KABC and check if selected and not same as the first
	// counter is the number of KABC items, and hence the index of the first non KABC item
	for ( uint i = counter; i < listBox->count(); ++i )
	{
		if ( listBox->isSelected( i ) )
		{
			// check whether it matches any KABC item
			bool duplicate = false;
			for ( uint j = 0; j < counter; ++j )
			{
				if ( listBox->text( i ) == listBox->text( j ) )
					duplicate = true;
			}
			if ( !duplicate )
				newValues.append( listBox->text( i ) );
		}
	}
	return newValues;
}
