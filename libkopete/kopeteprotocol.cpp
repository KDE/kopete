/*
    kopeteprotocol.cpp - Kopete Protocol

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprotocol.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetegroup.h"
#include <kdebug.h>

KopeteProtocol::KopeteProtocol(QObject *parent, const char *name)
    : KopetePlugin( parent, name )
{
}

KopeteProtocol::~KopeteProtocol()
{
}

bool KopeteProtocol::unload()
{
	KopeteMessageManagerFactory::factory()->cleanSessions(this);
	return KopetePlugin::unload();
}


QString KopeteProtocol::statusIcon() const
{
	return m_statusIcon;
}

void KopeteProtocol::setStatusIcon( const QString &icon )
{
	if( icon != m_statusIcon )
	{
		m_statusIcon = icon;
		emit( statusIconChanged( this, icon ) );
	}
}

KActionMenu* KopeteProtocol::protocolActions()
{
	return 0L;
}

const QDict<KopeteContact>& KopeteProtocol::contacts()
{
	return m_contacts;
}

QDict<KopeteContact> KopeteProtocol::contacts( KopeteMetaContact *mc )
{
	QDict<KopeteContact> result;

	QDictIterator<KopeteContact> it( contacts() );
	for ( ; it.current() ; ++it )
	{
		if( ( *it )->metaContact() == mc )
			result.insert( ( *it )->contactId(), *it );
	}
	return result;
}

void KopeteProtocol::registerContact( KopeteContact *c )
{
	m_contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
		SLOT( slotKopeteContactDestroyed( KopeteContact * ) ) );
}

void KopeteProtocol::slotKopeteContactDestroyed( KopeteContact *c )
{
	kdDebug(14010) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	m_contacts.remove( c->contactId() );
}

bool KopeteProtocol::addContact( const QString &contactId, const QString &displayName, 
	KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{	
	kdDebug(14010) << "[KopeteProtocol] addMetaContact() contactId:" << contactId << "; displayName: " << displayName
		<< "; groupName: " << groupName  << endl;

	//If this is a temporary contact, use the temporary group
	KopeteGroup *parentGroup;
	isTemporary ? parentGroup = KopeteGroup::temporary : parentGroup = KopeteContactList::contactList()->getGroup( groupName );

	if( parentContact )
	{
		//If we are given a MetaContact to add to that is marked as temporary. but 
		//this contact is not temporary, then change the metacontact to non-temporary
		if( parentContact->isTemporary() && !isTemporary )
			parentContact->setTemporary( false, parentGroup );
		else
			parentContact->addToGroup( parentGroup );

	} else {
		//Check if this MetaContact exists
		parentContact = KopeteContactList::contactList()->findContact( pluginId(), QString::null, contactId );
		if( !parentContact )
		{
			//Create a new MetaContact
			parentContact = new KopeteMetaContact();
			parentContact->setDisplayName( displayName );
			KopeteContactList::contactList()->addMetaContact( parentContact );
		
			//Set it as a temporary contact if requested
			if( isTemporary )
				parentContact->setTemporary(true);
		}
		
		//Add the MetaContact to the correct group
		if( !isTemporary )
			parentContact->addToGroup( parentGroup );
	}

	//We should now have a parentContact.
	//Call the protocols function to add the contact to this parent
	if( parentContact )
		return addContactToMetaContact( contactId, displayName, parentContact );
	else
		return false;
}

bool KopeteProtocol::addContactToMetaContact( const QString &, const QString &, KopeteMetaContact *)
{
	kdDebug(14010) << "[KopeteProtocol] addContactToMetaContact() Not Implemented!!!" << endl;
	return false;
}

#include "kopeteprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

