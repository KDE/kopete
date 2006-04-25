/*
    oscarvisibilitydialog.cpp  -  Visibility Dialog

    Copyright (c) 2005 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarvisibilitydialog.h"

#include <qstringlist.h>
#include <qpushbutton.h>

#include <klocale.h>

#include "oscarvisibilitybase.h"
#include "client.h"


OscarVisibilityDialog::OscarVisibilityDialog( Client* client, QWidget* parent )
 : KDialog( parent, i18n( "Add Contacts to In/Visible List" ),
                Ok | Cancel ), m_client( client )
{
	m_visibilityUI = new OscarVisibilityBase( this );
	setMainWidget( m_visibilityUI );
	
	QObject::connect(m_visibilityUI->visibleAdd, SIGNAL(clicked()),
	                 this, SLOT(slotAddToVisible()));
	QObject::connect(m_visibilityUI->visibleRemove, SIGNAL(clicked()),
	                 this, SLOT(slotRemoveFromVisible()));
	QObject::connect(m_visibilityUI->invisibleAdd, SIGNAL(clicked()),
	                 this, SLOT(slotAddToInvisible()));
	QObject::connect(m_visibilityUI->invisibleRemove, SIGNAL(clicked()),
	                 this, SLOT(slotRemoveFromInvisible()));
}

void OscarVisibilityDialog::addContacts( const ContactMap& contacts )
{
	m_contactMap = contacts;
	
	ContactMap::Iterator it, cEnd = m_contactMap.end();
	for ( it = m_contactMap.begin(); it != cEnd; ++it )
		m_visibilityUI->contacts->insertItem( it.key() );
	
}

void OscarVisibilityDialog::addVisibleContacts( const QStringList& contactList )
{
	m_visibilityUI->visibleContacts->insertStringList( contactList );
}

void OscarVisibilityDialog::addInvisibleContacts( const QStringList& contactList )
{
	m_visibilityUI->invisibleContacts->insertStringList( contactList );
}

void OscarVisibilityDialog::slotAddToVisible()
{
	Q3ListBoxItem *itm = m_visibilityUI->contacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_visibleListChangesMap[contactId] = Add;
	
	if ( !m_visibilityUI->visibleContacts->findItem( itm->text(), Q3ListBox::CaseSensitive | Q3ListBox::ExactMatch ) )
		m_visibilityUI->visibleContacts->insertItem( itm->text() );
}

void OscarVisibilityDialog::slotRemoveFromVisible()
{
	Q3ListBoxItem *itm = m_visibilityUI->visibleContacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_visibleListChangesMap[contactId] = Remove;
	
	int visIdx = m_visibilityUI->visibleContacts->index( itm );
	m_visibilityUI->visibleContacts->removeItem( visIdx );
}

void OscarVisibilityDialog::slotAddToInvisible()
{
	Q3ListBoxItem *itm = m_visibilityUI->contacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_invisibleListChangesMap[contactId] = Add;
	
	if ( !m_visibilityUI->invisibleContacts->findItem( itm->text(), Q3ListBox::CaseSensitive | Q3ListBox::ExactMatch ) )
		m_visibilityUI->invisibleContacts->insertItem( itm->text() );
}

void OscarVisibilityDialog::slotRemoveFromInvisible()
{
	Q3ListBoxItem *itm = m_visibilityUI->invisibleContacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_invisibleListChangesMap[contactId] = Remove;
	
	int visIdx = m_visibilityUI->invisibleContacts->index( itm );
	m_visibilityUI->invisibleContacts->removeItem( visIdx );
}

void OscarVisibilityDialog::slotButtonClicked( int buttonCode )
{
	KDialog::slotButtonClicked(buttonCode);
	
	if( buttonCode == KDialog::Ok )
	{
		ChangeMap::Iterator it, cEnd = m_visibleListChangesMap.end();
		for ( it = m_visibleListChangesMap.begin(); it != cEnd; ++it ) {
			m_client->setVisibleTo( it.key(), it.value() );
		}
		
		cEnd = m_invisibleListChangesMap.end();
		for ( it = m_invisibleListChangesMap.begin(); it != cEnd; ++it ) {
			m_client->setInvisibleTo( it.key(), it.value() );
		}

		emit closing();
	}
	else if( buttonCode == KDialog::Cancel )
		emit closing();
}

#include "oscarvisibilitydialog.moc"
