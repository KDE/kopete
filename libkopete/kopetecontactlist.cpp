/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

	Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontactlist.h"

#include "kopetemetacontact.h"

#include <kapplication.h>

KopeteContactList *KopeteContactList::s_contactList = 0L;

KopeteContactList *KopeteContactList::contactList()
{
	if( !s_contactList )
		s_contactList = new KopeteContactList;

	return s_contactList;
}

KopeteContactList::KopeteContactList()
: QObject( kapp, "KopeteContactList" )
{
}

KopeteContactList::~KopeteContactList()
{
}

KopeteMetaContact *KopeteContactList::findContact( const QString &contactId )
{
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		QPtrListIterator<KopeteContact> contactIt( it.current()->contacts() );
		for( ; contactIt.current(); ++contactIt )
		{
			if( contactIt.current()->id() == contactId )
				return it.current();
		}
	}

	// Contact not found, create a new meta contact
	KopeteMetaContact *c = new KopeteMetaContact();
	m_contacts.append( c );
	return c;
}

void KopeteContactList::loadXML()
{
	QString xml_filename;

	m_dom = new QDomDocument("ContactList");
	xml_filename = "/home/duncan/contacts.xml";

	QFile xml_file(xml_filename);
	xml_file.open(IO_ReadWrite);
	m_dom->setContent(&xml_file);

	QDomElement list = m_dom->documentElement();
	QDomNode node_person;
	QDomNode node_contact;
	node_person = list.firstChild();

}

QStringList KopeteContactList::meta_all()
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		meta_contacts.append( it.current()->displayName() );
	}
	return meta_contacts;
}

QStringList KopeteContactList::meta_status()
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		meta_contacts.append( QString ("%1 (%2)").arg( it.current()->displayName() ).arg( it.current()->statusString() ) );
	}
	return meta_contacts;
}

QStringList KopeteContactList::meta_reachable()
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isReachable() )
			meta_contacts.append( it.current()->displayName() );
	}
	return meta_contacts;
}

QStringList KopeteContactList::meta_online()
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() )
			meta_contacts.append( it.current()->displayName() );
	}
	return meta_contacts;
}


#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

