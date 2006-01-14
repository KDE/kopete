/*
    kabcexport.cpp - Export Contacts to Address Book Wizard for Kopete

    Copyright (c) 2005 by Will Stephenson        <will@stevello.free-online.co.uk>
    Resource selector taken from KRES::SelectDialog
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qmap.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kabc/addressee.h>
#include <kabc/addressbook.h>
#include <kabc/phonenumber.h>
#include <kabc/picture.h>
#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>

#include <kabcpersistence.h>
#include <kopetecontact.h>
#include <kopetecontactlist.h>
#include <kopetecontactproperty.h>
#include <kopeteglobal.h>
#include <kopetemetacontact.h>

#include "kabcpersistence.h"

#include "kabcexport.h"

class ContactLVI : public QCheckListItem
{
	public:
		ContactLVI ( Kopete::MetaContact * mc, QListView * parent, const QString & text, Type tt = RadioButtonController ) : QCheckListItem( parent, text, tt ), mc( mc )
		{	}
		Kopete::MetaContact * mc;
		QString uid;
};

// ctor populates the resource list and contact list, and enables the next button on the first page 
KabcExportWizard::KabcExportWizard( QWidget *parent, const char *name )
	: KabcExportWizard_Base( parent, name )
{
	connect( m_addrBooks, SIGNAL( selectionChanged( QListBoxItem * ) ), SLOT( slotResourceSelectionChanged( QListBoxItem * ) ) );

	connect( m_btnSelectAll, SIGNAL( clicked() ), SLOT( slotSelectAll() ) );
	connect( m_btnDeselectAll, SIGNAL( clicked() ), SLOT( slotDeselectAll() ) );
	
	// fill resource selector
	m_addressBook = Kopete::KABCPersistence::self()->addressBook();

	QPtrList<KABC::Resource> kabcResources = m_addressBook->resources();

	QPtrListIterator<KABC::Resource> resIt( kabcResources );
	KABC::Resource *resource;
	
	uint counter = 0;
	while ( ( resource = resIt.current() ) != 0 ) 
	{
		++resIt;
		if ( !resource->readOnly() ) 
		{
			m_resourceMap.insert( counter, resource );
			m_addrBooks->insertItem( resource->resourceName() );
			counter++;
		}
	}
	setNextEnabled( QWizard::page( 0 ), false );
	setFinishEnabled( QWizard::page( 1 ), true );
	// if there were no writable address books, tell the user
	if ( counter == 0 )
	{
		m_addrBooks->insertItem( i18n( "No writeable addressbook resource found." ) );
		m_addrBooks->insertItem( i18n( "Add or enable one using the KDE Control Center." ) );
		m_addrBooks->setEnabled( false );
	}

	if ( m_addrBooks->count() == 1 )
		m_addrBooks->setSelected( 0, true );
	
	// fill contact list
	QPtrList<Kopete::MetaContact> contacts = Kopete::ContactList::self()->metaContacts();
	QPtrListIterator<Kopete::MetaContact> it( contacts );
	counter = 0;
	QString alreadyIn = i18n( " (already in address book)" );
	for (; it.current(); ++it)
	{
		m_contactMap.insert( counter, it.current() );
		QCheckListItem * lvi = new ContactLVI( it.current(), m_contactList,
				it.current()->displayName(), QCheckListItem::CheckBox );
		lvi->setOn( false );
		if ( it.current()->metaContactId().contains(':') )
		{
			lvi->setOn( true );
			lvi->setEnabled( true );
		}
		else
			lvi->setText( 0, lvi->text( 0 ) + alreadyIn );
	}
}

KabcExportWizard::~KabcExportWizard()
{
	
}

void KabcExportWizard::slotDeselectAll()
{
	QListViewItemIterator it( m_contactList );
	while ( it.current() )
	{
		ContactLVI *item = static_cast<ContactLVI *>( it.current() );
		item->setOn( false );
		++it;
	}
}

void KabcExportWizard::slotSelectAll()
{
	QListViewItemIterator it( m_contactList );
	while ( it.current() )
	{
		ContactLVI *item = static_cast<ContactLVI *>( it.current() );
		++it;
		if ( !item->isEnabled() )
			continue;
		item->setOn( true );
	}
}

void KabcExportWizard::slotResourceSelectionChanged( QListBoxItem * lbi )
{
	setNextEnabled( QWizard::page( 0 ), lbi->isSelected() );
}

// accept runs the export algorithm
void KabcExportWizard::accept()
{
	// first add an addressee to the selected resource 
	// then set the metacontactId of each MC to that of the new addressee
	KABC::Resource * selectedResource = 
			m_resourceMap[ ( m_addrBooks->index( m_addrBooks->selectedItem() ) ) ];
	// for each item checked
	{
		QListViewItemIterator it( m_contactList );
		while ( it.current() )
		{
			ContactLVI *item = static_cast<ContactLVI *>( it.current() );
			// if it is checked and enabled
			if ( item->isEnabled() && item->isOn() )
			{
				KABC::Addressee addr;
				addr = m_addressBook->findByUid( item->mc->metaContactId() );
				if ( addr.isEmpty() ) // unassociated contact
				{
					kdDebug( 14000 ) << "creating addressee " << item->mc->displayName() << " in address book " << selectedResource->resourceName() << endl;
					// create a new addressee in the selected resource
					addr.setResource( selectedResource );

					// set name
					QPtrList<Kopete::Contact> contacts = item->mc->contacts();
					if ( contacts.count() == 1 )
					{
						Kopete::ContactProperty prop;
						prop = contacts.first()->property(
								Kopete::Global::Properties::self()->fullName() );
						if ( prop.isNull() )
							addr.setNameFromString( item->mc->displayName() );
						else
							addr.setNameFromString(  prop.value().toString() );
					}
					else
						addr.setNameFromString( item->mc->displayName() );

					// set details
					exportDetails( item->mc, addr );
					m_addressBook->insertAddressee( addr );
					// set the metacontact's id to that of the new addressee 
					// - this causes the addressbook to be written by libkopete
					item->mc->setMetaContactId( addr.uid() );
				}
				else
				{
					exportDetails( item->mc, addr );
					m_addressBook->insertAddressee( addr );
				}
			}
			++it;
		}
	}
	// request a write in case we only changed details on existing linked addressee
	Kopete::KABCPersistence::self()->writeAddressBook( selectedResource );
	QDialog::accept();
}

void KabcExportWizard::exportDetails( Kopete::MetaContact * mc, KABC::Addressee & addr )
{
	// for each contact
	QPtrList<Kopete::Contact> contacts = mc->contacts();
	QPtrListIterator<Kopete::Contact> cit( contacts );
	for( ; cit.current(); ++cit )
	{
		Kopete::ContactProperty prop;
		prop = (*cit)->property( Kopete::Global::Properties::self()->emailAddress() );
		if ( !prop.isNull() )
		{
			addr.insertEmail( prop.value().toString() );
		}
		prop = (*cit)->property( Kopete::Global::Properties::self()->privatePhone() );
		if ( !prop.isNull() )
		{
			addr.insertPhoneNumber( KABC::PhoneNumber( prop.value().toString(), KABC::PhoneNumber::Home ) );
		}
		prop = (*cit)->property( Kopete::Global::Properties::self()->workPhone() );
		if ( !prop.isNull() )
		{
			addr.insertPhoneNumber( KABC::PhoneNumber( prop.value().toString(), KABC::PhoneNumber::Work ) );
		}
		prop = (*cit)->property( Kopete::Global::Properties::self()->privateMobilePhone() );
		if ( !prop.isNull() )
		{
			addr.insertPhoneNumber( KABC::PhoneNumber( prop.value().toString(), KABC::PhoneNumber::Cell ) );
		}
	
	}
	// metacontact photo
	QImage photo = mc->photo();
	if ( !photo.isNull() )
		addr.setPhoto( KABC::Picture( photo ) );
}

#include "kabcexport.moc"
