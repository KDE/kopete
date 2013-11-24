/*
    kopeteaddrbookexport.cpp - Kopete Online Status

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

#include "kopeteaddrbookexport.h"

#include <kabc/phonenumber.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <QPixmap>

#include <kdialog.h>
#include <kiconloader.h>
#include <k3listbox.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

KopeteAddressBookExport::KopeteAddressBookExport( QWidget *parent, Kopete::MetaContact *mc ) : QObject( parent ), Ui::AddressBookExportUI()
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
		mLblFirstName->setText( mAddressee.givenNameLabel() );
		mLblLastName->setText( mAddressee.familyNameLabel() );
		mLblEmail->setText( mAddressee.emailLabel() );
		mLblUrl->setText( mAddressee.urlLabel() );
		mLblHomePhone->setText( mAddressee.homePhoneLabel() );
		mLblWorkPhone->setText( mAddressee.businessPhoneLabel() );
		mLblMobilePhone->setText( mAddressee.mobilePhoneLabel() );
	}
}

void KopeteAddressBookExport::fetchKABCData()
{
	if ( !mAddressee.isEmpty() )
	{
		mAddrBookIcon = SmallIcon( "office-address-book" );
		
		// given name
		QString given = mAddressee.givenName();
		if ( !given.isEmpty() )
			mFirstName->addItem( QIcon(mAddrBookIcon), given );
		else
			mFirstName->addItem( QIcon(mAddrBookIcon), i18n("<Not Set>") );
			
		// family name
		QString family = mAddressee.familyName();
		if ( !family.isEmpty() )
			mLastName->addItem( QIcon(mAddrBookIcon), family );
		else
			mLastName->addItem( QIcon(mAddrBookIcon), i18n("<Not Set>") );
		
		// url
		QString url = mAddressee.url().url();
		if ( !url.isEmpty() )
			mUrl->addItem( QIcon(mAddrBookIcon), url );
		else
			mUrl->addItem( QIcon(mAddrBookIcon), i18n("<Not Set>") );
		
		// emails
		QStringList emails = mAddressee.emails();
		numEmails = emails.count();
		for ( QStringList::Iterator it = emails.begin(); it != emails.end(); ++it )
			mEmails->insertItem( mAddrBookIcon, *it );
		if ( numEmails == 0 )
		{
			mEmails->insertItem( mAddrBookIcon, i18n("<Not Set>") );
			numEmails = 1;
		}
		
		// phone numbers
		fetchPhoneNumbers( mHomePhones, KABC::PhoneNumber::Home, numHomePhones );
		fetchPhoneNumbers( mWorkPhones, KABC::PhoneNumber::Work, numWorkPhones );
		fetchPhoneNumbers( mMobilePhones, KABC::PhoneNumber::Cell, numMobilePhones );
	}
}

void KopeteAddressBookExport::fetchPhoneNumbers( K3ListBox * listBox, KABC::PhoneNumber::Type type, uint& counter )
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
	QList<Kopete::Contact*> contacts = mMetaContact->contacts();
	QList<Kopete::Contact*>::iterator cit, citEnd = contacts.end();
	for( cit = contacts.begin(); cit != citEnd; ++cit )
	{
		// for each contact, get the property content
		Kopete::Contact* c = (*cit);
		QPixmap contactIcon = c->account()->accountIcon( 16 );
		// given name
		populateIM( c, contactIcon, mFirstName, Kopete::Global::Properties::self()->firstName() );
		// family name
		populateIM( c, contactIcon, mLastName, Kopete::Global::Properties::self()->lastName() );
		// url
		// TODO: make URL/homepage a global template, currently only in IRC channel contact
		// emails
		populateIM( c, contactIcon, mEmails, Kopete::Global::Properties::self()->emailAddress() );
		// home phone
		populateIM( c, contactIcon, mHomePhones, Kopete::Global::Properties::self()->privatePhone() );
		// work phone
		populateIM( c, contactIcon, mWorkPhones, Kopete::Global::Properties::self()->workPhone() );
		// mobile phone
		populateIM( c, contactIcon, mMobilePhones, Kopete::Global::Properties::self()->privateMobilePhone() );
	}
}

void KopeteAddressBookExport::populateIM( const Kopete::Contact *contact, const QPixmap &icon, QComboBox *combo, const Kopete::PropertyTmpl &property )
{
	Kopete::Property prop = contact->property( property );
	if ( !prop.isNull() )
	{
		combo->addItem( QIcon(icon), prop.value().toString() );
	}	
}

void KopeteAddressBookExport::populateIM( const Kopete::Contact *contact, const QPixmap &icon, K3ListBox *listBox, const Kopete::PropertyTmpl &property )
{
	Kopete::Property prop = contact->property( property );
	if ( !prop.isNull() )
	{
		listBox->insertItem( icon, prop.value().toString() );
	}	
}

int KopeteAddressBookExport::showDialog()
{
	mAddressee = mAddressBook->findByUid( mMetaContact->kabcId() );
	if ( !mAddressee.isEmpty() )
	{
		numEmails = 0;
		numHomePhones = 0;
		numWorkPhones = 0;
		numMobilePhones = 0;
		mDialog = new KDialog( mParent );
		mDialog->setCaption( i18n("Export to Address Book") );
		mDialog->setButtons( KDialog::Ok|KDialog::Cancel );
		
		QWidget* w = new QWidget( mDialog );
		setupUi( w );
		mDialog->setMainWidget( w );
		mDialog->setButtonGuiItem( KDialog::Ok, KGuiItem( i18n( "Export" ), 
							  QString(), i18n( "Set address book fields using the selected data from Kopete" ) ) );

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
	if ( newValue( mFirstName ) )
	{
		dirty = true;
		mAddressee.setGivenName( mFirstName->currentText() );
	}
	// last name
	if ( newValue( mLastName ) )
	{
		dirty = true;
		mAddressee.setFamilyName( mLastName->currentText() );
	}
	// url
	if ( newValue( mUrl ) )
	{
		dirty = true;
		mAddressee.setUrl( KUrl( mUrl->currentText() ) );
	}

	QStringList newVals;
	// email
	newVals = newValues( mEmails, numEmails );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertEmail( *it );
	}
	// home phone
	newVals = newValues( mHomePhones, numHomePhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KABC::PhoneNumber( *it, KABC::PhoneNumber::Home ) );
	}
	// work phone
	newVals = newValues( mWorkPhones, numWorkPhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KABC::PhoneNumber( *it, KABC::PhoneNumber::Work ) );
	}
	// mobile
	newVals = newValues( mMobilePhones, numMobilePhones );
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
			kWarning( 14000 ) << "WARNING: Resource is locked by other application!";
		else
		{
			if ( !mAddressBook->save( ticket ) )
			{
				kWarning( 14000 ) << "ERROR: Saving failed!";
				mAddressBook->releaseSaveTicket( ticket );
			}
		}
		kDebug( 14000 ) << "Finished writing KABC";
	}
}

bool KopeteAddressBookExport::newValue( QComboBox *combo )
{
	// all data in position 0 is from KABC, so if position 0 is selected,
	// or if the selection is the same as the data at 0, return false
	return !( combo->currentIndex() == 0 || 
			( combo->itemText( combo->currentIndex() ) == combo->itemText( 0 ) ) );
}

QStringList KopeteAddressBookExport::newValues( K3ListBox *listBox, uint counter )
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
