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

#include "kabcexport.h"

#include <qpushbutton.h>
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
#include <kopeteproperty.h>
#include <kopeteglobal.h>
#include <kopetemetacontact.h>
#include <kopetepicture.h>

class ContactLVI : public QListWidgetItem
{
	public:
		ContactLVI ( Kopete::MetaContact * mc, QListWidget * parent, const QString & text, QListWidgetItem::ItemType tt = Type ) : QListWidgetItem( text,parent, tt ), mc( mc )
		{
			
		}
		Kopete::MetaContact * mc;
		QString uid;
};

// ctor populates the resource list and contact list, and enables the next button on the first page 
KabcExportWizard::KabcExportWizard( QWidget *parent )
	: KAssistantDialog(parent)
{
	QWidget *page1Widget=new QWidget(this);
	m_page1.setupUi(page1Widget);
	m_page1WidgetItem=addPage(page1Widget,i18n("Select Address Book"));
	QWidget *page2Widget=new QWidget(this);
	m_page2.setupUi(page2Widget);
	m_page2WidgetItem=addPage(page2Widget,i18n("Select Contact"));
	

	connect( m_page1.addrBooks, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
		  SLOT(slotResourceSelectionChanged(QListWidgetItem*)));

	connect( m_page2.btnSelectAll, SIGNAL(clicked()), SLOT(slotSelectAll()) );
	connect( m_page2.btnDeselectAll, SIGNAL(clicked()), SLOT(slotDeselectAll()) );
	
	// fill resource selector
	m_addressBook = Kopete::KABCPersistence::self()->addressBook();

	QList<KABC::Resource*> kabcResources = m_addressBook->resources();

	QListIterator<KABC::Resource*> resIt( kabcResources );
	KABC::Resource *resource;
	
	uint counter = 0;
	while ( resIt.hasNext() ) 
	{
		resource = resIt.next();
		if ( !resource->readOnly() ) 
		{
			m_resourceMap.insert( counter, resource );
			m_page1.addrBooks->addItem( resource->resourceName() );
			counter++;
		}
	}

	setValid(m_page1WidgetItem,false);

	// if there were no writable address books, tell the user
	if ( counter == 0 )
	{
		m_page1.addrBooks->addItem( i18n( "No writeable address book resource found." ) );
		m_page1.addrBooks->addItem( i18n( "Add or enable one using the KDE System Settings." ) );
		m_page1.addrBooks->setEnabled( false );
	}

	if ( m_page1.addrBooks->count() == 1 )
		m_page1.addrBooks->setCurrentRow( 0 );
	
	// fill contact list
	QList<Kopete::MetaContact*> contacts = Kopete::ContactList::self()->metaContacts();
	QList<Kopete::MetaContact*>::iterator it, itEnd = contacts.end();
	counter = 0;
	QString alreadyIn = i18n( " (already in address book)" );
	for ( it = contacts.begin(); it != itEnd; ++it)
	{
		Kopete::MetaContact* mc = (*it);
		m_contactMap.insert( counter, mc );
		QListWidgetItem * lvi = new ContactLVI( mc, m_page2.contactList,
				mc->displayName() );
		lvi->setCheckState( Qt::Unchecked );
		const QString &kabcId = mc->kabcId();
		if ( kabcId.isEmpty() || m_addressBook->findByUid(kabcId).isEmpty() )
		{
			lvi->setCheckState( Qt::Checked );
			lvi->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);	
		}
		else
		{
			lvi->setText( lvi->text() + alreadyIn );
			lvi->setFlags( 0 );
		}
	}
}

KabcExportWizard::~KabcExportWizard()
{
	
}

void KabcExportWizard::slotDeselectAll()
{
	for(int i=0;i<m_page2.contactList->count();i++)
	{
		ContactLVI *item = static_cast<ContactLVI *>( m_page2.contactList->item(i) );
		item->setCheckState( Qt::Unchecked );
	}
}

void KabcExportWizard::slotSelectAll()
{
	for(int i=0;i<m_page2.contactList->count();i++)
	{
		ContactLVI *item = static_cast<ContactLVI *>( m_page2.contactList->item(i) );
		if ( item->flags() & Qt::ItemIsEnabled)
			item->setCheckState( Qt::Checked );
	}
}

void KabcExportWizard::slotResourceSelectionChanged( QListWidgetItem * lbi )
{
	setValid( m_page1WidgetItem,true );
	Q_UNUSED(lbi);
}

// accept runs the export algorithm
void KabcExportWizard::accept()
{
	// first add an addressee to the selected resource 
	// then set the metacontactId of each MC to that of the new addressee
	KABC::Resource * selectedResource = 
			m_resourceMap[ ( m_page1.addrBooks->currentRow() ) ];
	// for each item checked
	{
		for(int i=0;i<m_page2.contactList->count();i++)
		{
			ContactLVI *item = static_cast<ContactLVI *>(  m_page2.contactList->item(i) );
			// if it is checked and enabled
			if ( item->flags() & Qt::ItemIsEnabled && item->checkState() & Qt::Checked)
			{
				KABC::Addressee addr;
				addr = m_addressBook->findByUid( item->mc->kabcId() );
				if ( addr.isEmpty() ) // unassociated contact
				{
					kDebug( 14000 ) << "creating addressee " << item->mc->displayName() << " in address book " << selectedResource->resourceName();
					// create a new addressee in the selected resource
					addr.setResource( selectedResource );

					// set name
					QList<Kopete::Contact*> contacts = item->mc->contacts();
					if ( contacts.count() == 1 )
					{
						Kopete::Property prop;
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
					item->mc->setKabcId( addr.uid() );
				}
				else
				{
					exportDetails( item->mc, addr );
					m_addressBook->insertAddressee( addr );
				}
			}
		}
	}
	// request a write in case we only changed details on existing linked addressee
	Kopete::KABCPersistence::self()->writeAddressBook( selectedResource );
	QDialog::accept();
}

void KabcExportWizard::exportDetails( Kopete::MetaContact * mc, KABC::Addressee & addr )
{
	QList<Kopete::Contact*> contacts = mc->contacts();
	QList<Kopete::Contact*>::iterator cit, citEnd = contacts.begin();
	for( cit = contacts.begin(); cit != citEnd; ++cit )
	{
		Kopete::Property prop;
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
	
	if( !mc->picture().isNull() )
	{
		QImage photo = mc->picture().image();
		addr.setPhoto( KABC::Picture( photo ) );
	}
}

#include "kabcexport.moc"
