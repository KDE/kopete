/*
    kopetecontactlist.cpp - Kopete's Contact List backend

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
	Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

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

#include "kopete.h"
#include "kopetecontactlistview.h"
#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"

#include <qptrlist.h>
#include <qstringlist.h>
#include <qstylesheet.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

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

KopeteMetaContact *KopeteContactList::findContact( const QString &protocolId, const QString &contactId )
{
	//kdDebug() << "*** Looking for contact " << contactId << ", proto " << protocolId << endl;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
		{
		//kdDebug() << "*** Iterating " << it.current()->displayName() << endl;
		KopeteContact *c = it.current()->findContact( protocolId, contactId );
		if( c )
				return it.current();
		}
	//kdDebug() << "*** Not found!" << endl;

	// Contact not found, create a new meta contact
	KopeteMetaContact *mc = new KopeteMetaContact();
	//m_contacts.append( c );
	KopeteContactList::contactList()->addMetaContact(mc);

	return mc;
}


void KopeteContactList::addMetaContact( KopeteMetaContact *mc )
{
    QStringList groups;

	groups = mc->groups();

	m_contacts.append( mc );

	if ( groups.isEmpty() )
	{
		kdDebug() << "KopeteContactList::addMetaContact: "
			<< "adding metacontact with no groups" << endl;
		groups.append( "" );
	}
	else
	{
		QStringList::ConstIterator it = groups.begin();
		for( ; it != groups.end(); ++it )
		{
			QString group = *it;
			kdDebug() << "KopeteContactList::addMetaContact: "
				<< "adding metacontact to group " << group <<endl;
			kopeteapp->contactList()->addContact( new KopeteMetaContactLVI(
				mc, kopeteapp->contactList()->getGroup( group ) ) );
		}
	}
}


void KopeteContactList::loadXML()
{
	QDomDocument contactList( "messaging-contact-list" );

	QString filename = locateLocal( "appdata", "contactlist.xml" );

	if( filename.isEmpty() )
		return ;

	QFile contactListFile( filename );
	contactListFile.open( IO_ReadOnly );
	contactList.setContent( &contactListFile );

	QDomElement list = contactList.documentElement();
	QDomNode node = list.firstChild();
	while( !node.isNull() )
	{
		QDomElement element = node.toElement();
		if( !element.isNull() )
		{
			if( element.tagName() == "meta-contact" )
			{
				//TODO: id isn't used
				QString id = element.attribute( "id", QString::null );
				KopeteMetaContact *metaContact = new KopeteMetaContact();
				QDomNode contactNode = node.firstChild();
				if ( !metaContact->fromXML( contactNode ) ) {
					delete metaContact;
					metaContact = 0;
				} else
					KopeteContactList::contactList()->addMetaContact( metaContact );
						}
			else
						{
				kdDebug() << "KopeteContactList::loadXML: Warning: "
					  << "Unknown element '" << element.tagName()
					  << "' in contact list!" << endl;
				}

			}
		node = node.nextSibling();
		}

	contactListFile.close();
}

void KopeteContactList::saveXML()
{
	QString contactListFileName = locateLocal( "appdata", "contactlist.xml" );
	QFile contactListFile( contactListFileName );
	if( contactListFile.open( IO_WriteOnly ) )
	{
		QTextStream stream( &contactListFile );
		stream.setEncoding( QTextStream::UnicodeUTF8 );
	
	stream << toXML();

		contactListFile.close();
	}
	else
	{
		kdDebug() << "WARNING: Couldn't open contact list file "
			<< contactListFileName << ". Contact list not saved." << endl;
	}
}

QString KopeteContactList::toXML()
{
	QString xml = "<?xml version=\"1.0\"?>\n"
		"<messaging-contact-list>\n";

	QPtrListIterator<KopeteMetaContact> metaContactIt( m_contacts );
	for( ; metaContactIt.current(); ++metaContactIt )
	{
		kdDebug() << "KopeteContactList::toXML: Saving meta contact "
			<< ( *metaContactIt )->displayName() << endl;
		xml +=  ( *metaContactIt)->toXML();
	}

	xml += "</messaging-contact-list>\n";

    return xml;
}


QStringList KopeteContactList::contacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::contactStatuses() const
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		meta_contacts.append( QString ("%1 (%2)").arg( it.current()->displayName() ).arg( it.current()->statusString() ) );
	}
	return meta_contacts;
}

QStringList KopeteContactList::reachableContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isReachable() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::onlineContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::groups() const
{
    QStringList groups;

	QPtrListIterator<KopeteMetaContact> it( KopeteContactList::contactList()->m_contacts );
	kdDebug() << "[AddContactWizard] ****************************" <<endl;
	
	for( ; it.current(); ++it )
	{
		QStringList thisgroups;

		/* We get groups for this metacontact */
		thisgroups = it.current()->groups();

		if ( thisgroups.isEmpty() ) continue;

		for( QStringList::ConstIterator it = thisgroups.begin(); it != thisgroups.end(); ++it )
		{
			 /* We add the group only if it is not already there */
			QString groupname = (*it);
            if ( ! groups.contains( groupname ) && !groupname.isNull() )
			{
                kdDebug() << "[AddContactWizard] Adding group [" << groupname << "]" <<endl;
				groups.append( *it );
			}
		}
	}
	kdDebug() << "[AddContactWizard] ****************************" <<endl;
				

	return groups;
}

#include "kopetecontactlist.moc"



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

