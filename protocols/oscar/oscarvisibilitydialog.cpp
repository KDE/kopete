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
 : KDialogBase( parent,  0, false, i18n( "Add Contacts to Visible or Invisible List" ),
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
	QListBoxItem *itm = m_visibilityUI->contacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_visibleListChangesMap[contactId] = Add;
	
	if ( !m_visibilityUI->visibleContacts->findItem( itm->text(), Qt::CaseSensitive | Qt::ExactMatch ) )
		m_visibilityUI->visibleContacts->insertItem( itm->text() );
}

void OscarVisibilityDialog::slotRemoveFromVisible()
{
	QListBoxItem *itm = m_visibilityUI->visibleContacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_visibleListChangesMap[contactId] = Remove;
	
	int visIdx = m_visibilityUI->visibleContacts->index( itm );
	m_visibilityUI->visibleContacts->removeItem( visIdx );
}

void OscarVisibilityDialog::slotAddToInvisible()
{
	QListBoxItem *itm = m_visibilityUI->contacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_invisibleListChangesMap[contactId] = Add;
	
	if ( !m_visibilityUI->invisibleContacts->findItem( itm->text(), Qt::CaseSensitive | Qt::ExactMatch ) )
		m_visibilityUI->invisibleContacts->insertItem( itm->text() );
}

void OscarVisibilityDialog::slotRemoveFromInvisible()
{
	QListBoxItem *itm = m_visibilityUI->invisibleContacts->selectedItem();
	if ( !itm ) return;
	
	QString contactId = m_contactMap[itm->text()];
	m_invisibleListChangesMap[contactId] = Remove;
	
	int visIdx = m_visibilityUI->invisibleContacts->index( itm );
	m_visibilityUI->invisibleContacts->removeItem( visIdx );
}

void OscarVisibilityDialog::slotOk()
{
	ChangeMap::Iterator it, cEnd = m_visibleListChangesMap.end();
	for ( it = m_visibleListChangesMap.begin(); it != cEnd; ++it ) {
		m_client->setVisibleTo( it.key(), it.data() );
	}
	
	cEnd = m_invisibleListChangesMap.end();
	for ( it = m_invisibleListChangesMap.begin(); it != cEnd; ++it ) {
		m_client->setInvisibleTo( it.key(), it.data() );
	}
	
	KDialogBase::slotOk();
	emit closing();
}

void OscarVisibilityDialog::slotCancel()
{
	KDialogBase::slotCancel();
	emit closing();
}

#include "oscarvisibilitydialog.moc"
