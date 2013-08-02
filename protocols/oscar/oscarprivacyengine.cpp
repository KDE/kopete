/*
    oscarprivacyengine.cpp  -  Oscar Privacy Engine

    Copyright (c) 2005-2006 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2005-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarprivacyengine.h"

#include <QtGui/QComboBox>
#include <QtGui/QAbstractItemView>
#include <QtGui/QAbstractButton>

#include "client.h"
#include "oscaraccount.h"
#include "oscarcontact.h"
#include "contactmanager.h"

OscarPrivacyEngine::OscarPrivacyEngine( OscarAccount* account, Type type )
: QObject(), m_type( type )
{
	m_client = account->engine();
	
	ContactMap contactMap;
	
	QList<OContact> contactList = m_client->ssiManager()->contactList();
	QList<OContact>::const_iterator it, cEnd;
	
	cEnd = contactList.constEnd();
	for ( it = contactList.constBegin(); it != cEnd; ++it )
	{
		QString contactId = ( *it ).name();
		
		OscarContact* oc = dynamic_cast<OscarContact*>( account->contacts().value( ( *it ).name() ) );
		if ( oc )
		{	//for better orientation in lists use displayName and id
			QString screenName( "%1 (%2)" );
			screenName = screenName.arg( oc->displayName(), contactId );
			contactMap.insert( contactId, screenName );
		}
		else
		{
			contactMap.insert( contactId, contactId );
		}
	}
	addAllContacts( contactMap );
	
	switch( type )
	{
	case Visible:
		contactList = m_client->ssiManager()->visibleList();
		break;
	case Invisible:
		contactList = m_client->ssiManager()->invisibleList();
		break;
	case Ignore:
		contactList = m_client->ssiManager()->ignoreList();
		break;
	}
	
	QSet<QString> tmpSet;
	
	cEnd = contactList.constEnd();
	for ( it = contactList.constBegin(); it != cEnd; ++it )
		tmpSet.insert( ( *it ).name() );
	
	addContacts( contactMap, tmpSet );
}

OscarPrivacyEngine::~OscarPrivacyEngine()
{
}

void OscarPrivacyEngine::setContactsView( QAbstractItemView* view )
{
	view->setModel( &m_contactsModel );
	m_listView = view;
}

void OscarPrivacyEngine::setAllContactsView( QComboBox* combo )
{
	combo->setModel( &m_allContactsModel );
	m_comboBox = combo;
}

void OscarPrivacyEngine::slotAdd()
{
	QString id;
	
	int cutIndex = m_comboBox->currentIndex();
	if ( cutIndex != -1 && m_comboBox->currentText() == m_comboBox->itemText( cutIndex ) )
		id = m_comboBox->itemData( cutIndex, Qt::UserRole ).toString();
	else
		id = m_comboBox->currentText();
	
	if ( id.isEmpty() || m_idSet.contains( id ) )
		return;
	
	int rowCount = m_contactsModel.rowCount();
	m_contactsModel.insertRows( rowCount, 1 );
	
	QModelIndex index = m_contactsModel.index( rowCount, 0 );
	m_contactsModel.setData( index, id, Qt::UserRole );
	m_contactsModel.setData( index, m_comboBox->currentText(), Qt::DisplayRole );
	
	m_idSet.insert( id );
	m_changesMap[id] = Add;
}

void OscarPrivacyEngine::slotRemove()
{
	QItemSelectionModel *selectionModel = m_listView->selectionModel();
	
	foreach ( const QModelIndex &selectedIndex, selectionModel->selectedIndexes() )
	{
		QString id = selectedIndex.data( Qt::UserRole ).toString();
		
		m_contactsModel.removeRows( selectedIndex.row(), 1 );
		
		m_idSet.remove( id );
		m_changesMap[id] = Remove;
	}
}

void OscarPrivacyEngine::storeChanges()
{
	if ( !m_client->isActive() )
		return;
	
	ChangeMap::ConstIterator it, cEnd = m_changesMap.constEnd();
	for ( it = m_changesMap.constBegin(); it != cEnd; ++it )
	{
		switch( m_type )
		{
		case Visible:
			m_client->setVisibleTo( it.key(), it.value() );
			break;
		case Invisible:
			m_client->setInvisibleTo( it.key(), it.value() );
			break;
		case Ignore:
			m_client->setIgnore( it.key(), it.value() );
			break;
		}
	}
}

void OscarPrivacyEngine::addContacts( const ContactMap& contacts, const QSet<QString>& idSet )
{
	m_idSet = idSet;
	m_contactsModel.clear();
	m_contactsModel.insertColumns( 0, 1 );
	m_contactsModel.insertRows( 0, idSet.size() );
	
	int i = 0;
	foreach ( const QString& id, idSet )
	{
		QModelIndex index = m_contactsModel.index( i++, 0 );
		
		m_contactsModel.setData( index, id, Qt::UserRole );
		m_contactsModel.setData( index, contacts.value( id, id ), Qt::DisplayRole );
	}
}

void OscarPrivacyEngine::addAllContacts( const ContactMap& contacts )
{
	m_allContactsModel.insertColumns( 0, 1 );
	m_allContactsModel.insertRows( 0, contacts.count() );
	
	int i = 0;
	ContactMap::ConstIterator it, cEnd = contacts.end();
	for ( it = contacts.begin(); it != cEnd; ++it )
	{
		QModelIndex idx = m_allContactsModel.index( i++, 0 );
		
		m_allContactsModel.setData( idx, it.key(), Qt::UserRole );
		m_allContactsModel.setData( idx, it.value(), Qt::DisplayRole );
	}
}

#include "oscarprivacyengine.moc"
