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

#include <kcontacts/phonenumber.h>
#include <QComboBox>
#include <QLabel>
#include <QDialog>
#include <QPixmap>

#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteglobal.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"

KopeteAddressBookExport::KopeteAddressBookExport( QWidget *parent, Kopete::MetaContact *mc ) : QObject( parent ), Ui::AddressBookExportUI()
{
	// instantiate dialog and populate widgets
	mParent = parent;
	//DEPRECATED: mAddressBook = KContacts::StdAddressBook::self();
	mMetaContact = mc;

	mWorkPhones->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mMobilePhones->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mHomePhones->setSelectionMode(QAbstractItemView::ExtendedSelection);
	mEmails->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
		unsigned int rowCount = 0;
		numEmails = emails.count();
		for ( QStringList::Iterator it = emails.begin(); it != emails.end(); ++it ) {
			mEmails->insertItem(rowCount, *it);
			mEmails->item(rowCount)->setIcon(QIcon(mAddrBookIcon));
			++rowCount;
		}
		if ( numEmails == 0 )
		{
			QListWidgetItem *newItem = new QListWidgetItem(mAddrBookIcon, i18n("<Not Set>"));
			mEmails->addItem(newItem);
			numEmails = 1;
		}
		
		// phone numbers
		fetchPhoneNumbers( mHomePhones, KContacts::PhoneNumber::Home, numHomePhones );
		fetchPhoneNumbers( mWorkPhones, KContacts::PhoneNumber::Work, numWorkPhones );
		fetchPhoneNumbers( mMobilePhones, KContacts::PhoneNumber::Cell, numMobilePhones );
	}
}

void KopeteAddressBookExport::fetchPhoneNumbers( QListWidget * listBox, KContacts::PhoneNumber::Type type, uint& counter )
{
	KContacts::PhoneNumber::List phones = mAddressee.phoneNumbers( type );
	counter = phones.count();
	KContacts::PhoneNumber::List::Iterator it;
	unsigned int rowCount = 0;
	for ( it = phones.begin(); it != phones.end(); ++it ) {
		listBox->item(rowCount)->setIcon(QIcon(mAddrBookIcon));
		listBox->insertItem(rowCount, (*it).number());
	}
	if ( counter == 0 )
	{
		QListWidgetItem *newItem = new QListWidgetItem(mAddrBookIcon, i18n("<Not Set>"));
		listBox->addItem(newItem);
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

void KopeteAddressBookExport::populateIM( const Kopete::Contact *contact, const QPixmap &icon, QListWidget *listBox, const Kopete::PropertyTmpl &property )
{
	Kopete::Property prop = contact->property( property );
	if ( !prop.isNull() )
	{
		QListWidgetItem *newItem = new QListWidgetItem(QIcon(icon), prop.value().toString());
		listBox->addItem(newItem);
		delete newItem;
	}	
}

int KopeteAddressBookExport::showDialog()
{
	//DEPRECATED: mAddressee = mAddressBook->findByUid( mMetaContact->kabcId() );
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
		mAddressee.insertPhoneNumber( KContacts::PhoneNumber( *it, KContacts::PhoneNumber::Home ) );
	}
	// work phone
	newVals = newValues( mWorkPhones, numWorkPhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KContacts::PhoneNumber( *it, KContacts::PhoneNumber::Work ) );
	}
	// mobile
	newVals = newValues( mMobilePhones, numMobilePhones );
	for ( QStringList::Iterator it = newVals.begin(); it != newVals.end(); ++it )
	{
		dirty = true;
		mAddressee.insertPhoneNumber( KContacts::PhoneNumber( *it, KContacts::PhoneNumber::Cell ) );
	}
	
	if ( dirty )
	{
	  /** DEPRECATED : Skipping addressbook updation for the moment
		// write the changed addressbook
		mAddressBook->insertAddressee( mAddressee );
	
		KContacts::Ticket *ticket = mAddressBook->requestSaveTicket();
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
		*/
	}
}

bool KopeteAddressBookExport::newValue( QComboBox *combo )
{
	// all data in position 0 is from KABC, so if position 0 is selected,
	// or if the selection is the same as the data at 0, return false
	return !( combo->currentIndex() == 0 || 
			( combo->itemText( combo->currentIndex() ) == combo->itemText( 0 ) ) );
}

QStringList KopeteAddressBookExport::newValues( QListWidget *listBox, int counter )
{
	QStringList newValues;
	// need to iterate all items except those from KABC and check if selected and not same as the first
	// counter is the number of KABC items, and hence the index of the first non KABC item
	for ( int i = counter; i < listBox->count(); ++i )
	{
		if ( listBox->item(i)->isSelected() )
		{
			// check whether it matches any KABC item
			bool duplicate = false;
			for ( int j = 0; j < counter; ++j )
			{
				if ( listBox->item(i)->text() == listBox->item(j)->text() )
					duplicate = true;
			}
			if ( !duplicate )
				newValues.append( listBox->item(i)->text() );
		}
	}
	return newValues;
}
