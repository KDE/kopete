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

#include <qdom.h>
#include <qfile.h>
#include <qstylesheet.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
#include "kopetegroup.h"


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
// save is currently called in ~kopete (before the deletion of plugins)
//	save();
}

KopeteMetaContact *KopeteContactList::findContact( const QString &protocolId,
	const QString &identityId, const QString &contactId )
{
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->findContact( protocolId, identityId, contactId ) )
			return it.current();
	}
	// kdDebug() << "KopeteContactList::findContact  *** Not found!" << endl;
	return 0L;
}

void KopeteContactList::addMetaContact( KopeteMetaContact *mc )
{
	m_contacts.append( mc );

/*	connect( mc,
		SIGNAL( removedFromGroup( KopeteMetaContact *, const QString & ) ),
		SLOT( slotRemovedFromGroup( KopeteMetaContact *, const QString & ) ) );*/

	emit metaContactAdded( mc );
}

/*void KopeteContactList::slotRemovedFromGroup( KopeteMetaContact *mc,
	const QString &  )
{
	if( mc->groups().isEmpty() )
	{
		kdDebug() << "KopeteContactList::slotRemovedFromGroup: "
			<< "contact removed from all groups: now toplevel." << endl;
		//m_contacts.remove( mc );
		//mc->deleteLater();
	}
}                  */

void KopeteContactList::loadXML()
{
	addGroup( KopeteGroup::toplevel );

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
				if ( !metaContact->fromXML( contactNode ) )
				{
					delete metaContact;
					metaContact = 0;
				}
				else
				{
					KopeteContactList::contactList()->addMetaContact(
						metaContact );
				}
			}
			else if( element.tagName() == "kopete-group" )
			{
				KopeteGroup *group = new KopeteGroup();
				if( !group->fromXML( node.firstChild() ) )
				{
					delete group;
					group = 0;
				}
				else
				{
					KopeteContactList::contactList()->addGroup( group );
				}
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

	//kdDebug() << "KopeteContactList::saveXML: Contact List File: "
	//	<< contactListFileName << endl;

	KSaveFile contactListFile( contactListFileName );
	if( contactListFile.status() == 0 )
	{
		QTextStream *stream = contactListFile.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );

		*stream << toXML();

		if ( !contactListFile.close() )
		{
			kdDebug() << "failed to write contactlist, error code is: " << contactListFile.status() << endl;
		}
	}
	else
	{
		kdDebug() << "WARNING: Couldn't open contact list file "
			<< contactListFileName << ". Contact list not saved." << endl;
	}

/*	QFile contactListFile( contactListFileName );
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
*/
}

QString KopeteContactList::toXML()
{
	QString xml = "<?xml version=\"1.0\"?>\n"
		"<messaging-contact-list version=\"0.5\">\n";

	// Save group information. ie: Open/Closed, pehaps later icons? Who knows.
	KopeteGroup *groupIt;
	for( groupIt = m_groupList.first(); groupIt; groupIt = m_groupList.next() )
	    xml += groupIt->toXML();

	// Save metacontact information.
	QPtrListIterator<KopeteMetaContact> metaContactIt( m_contacts );
	for( ; metaContactIt.current(); ++metaContactIt )
	{
		if(!(*metaContactIt)->isTemporary())
		{
//			kdDebug() << "KopeteContactList::toXML: Saving meta contact "
//				<< ( *metaContactIt )->displayName() << endl;
			xml +=  ( *metaContactIt)->toXML();
		}
	}

	xml += "</messaging-contact-list>\n";

	return xml;
}


QStringList KopeteContactList::contacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::contactStatuses() const
{
	QStringList meta_contacts;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		meta_contacts.append( QString( "%1 (%2)" ).arg(
			it.current()->displayName() ).arg(
			it.current()->statusString() ) );
	}
	return meta_contacts;
}

QStringList KopeteContactList::reachableContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
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
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

QStringList KopeteContactList::onlineContacts( const QString &protocolId ) const
{
	QStringList onlineContacts;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		// FIXME: This loop is not very efficient :(
		if ( it.current()->isOnline() )
		{
			QPtrList<KopeteContact> contacts = it.current()->contacts();
			QPtrListIterator<KopeteContact> cit( contacts );
			for( ; cit.current(); ++cit )
			{
				if ( cit.current()->protocol()->pluginId() == protocolId )
					onlineContacts.append( it.current()->displayName() );
			}
		}
	}
	return onlineContacts;
}

QStringList KopeteContactList::fileTransferContacts() const
{
	QStringList contacts;
	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if ( it.current()->canAcceptFiles() )
			contacts.append( it.current()->displayName() );
	}
	return contacts;
}

void KopeteContactList::sendFile( const QString &displayName, const KURL &sourceURL, 
	const QString &altFileName, const long unsigned int fileSize)
{	/*
	 * FIXME: We should be using either some kind of unique ID (kabc ID?)
	 * here, or force user to only enter unique display names. A
	 * unique identifier is needed for external DCOP refs like this!
	 */
	 
//	kdDebug() << "Send To Display Name: " << displayName << "\n";

	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
//		kdDebug() << "Display Name: " << it.current()->displayName() << "\n";
		if( it.current()->displayName() == displayName ) {
			it.current()->sendFile( sourceURL, altFileName, fileSize );
			return;
		}
	}
}

QStringList KopeteContactList::contactFileProtocols(QString displayName)
{
//	kdDebug() << "Get contacts for: " << displayName << "\n";
	QStringList protocols;

	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		if( it.current()->displayName() == displayName ) {
//			kdDebug() << "Found them!" << endl;
			QPtrList<KopeteContact> mContacts = it.current()->contacts();
			kdDebug() << mContacts.count() << endl;
			QPtrListIterator<KopeteContact> jt( mContacts );
			for ( ; jt.current(); ++jt )
			{
				kdDebug() << "1" << jt.current()->protocol()->pluginId() << "\n";
				if( jt.current()->canAcceptFiles() ) {
					kdDebug() << jt.current()->protocol()->pluginId() << "\n";
					protocols.append ( jt.current()->protocol()->pluginId() );
				}
			}
			return protocols;
		}
	}
	return QStringList();
}


KopeteGroupList KopeteContactList::groups() const
{
	return m_groupList;
}

void KopeteContactList::removeMetaContact(KopeteMetaContact *m)
{
	for(KopeteContact *c = m->contacts().first(); c ; c = m->contacts().next() )
	{
		c->slotDeleteContact();
	}
	emit metaContactDeleted( m );
	m_contacts.remove( m );
	delete m;
}

QPtrList<KopeteMetaContact> KopeteContactList::metaContacts() const
{
	return m_contacts;
}

void KopeteContactList::addGroup( KopeteGroup * g)
{
	if(!m_groupList.contains(g) )
	{
		m_groupList.append( g );
		emit groupAdded( g );
		connect( g , SIGNAL ( renamed(KopeteGroup* , const QString & )) , this , SIGNAL ( groupRenamed(KopeteGroup* , const QString & )) ) ;
	}
}

void KopeteContactList::removeGroup( KopeteGroup *g)
{
	m_groupList.remove( g );
	emit groupRemoved( g );
	delete g;
}

bool KopeteContactList::dcopAddContact( const QString &protocolName, const QString &contactId,
	const QString &displayName, KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{
	//Get the protocol instance
	KopeteProtocol *myProtocol = (KopeteProtocol*) LibraryLoader::pluginLoader()->searchByName( protocolName );
	
	if( myProtocol != 0L ) 
	{
		QString contactName;
		
		//If the nickName isn't specified we need to display the userId in the prompt
		if( displayName.isEmpty() || displayName.isNull() )
			contactName = contactId;
		else
			contactName = displayName;
		
		//Confirm with the user before we add the contact
		if( KMessageBox::questionYesNo( 0, i18n("An external application is attempting to add the "
				" %1 contact \"%2\" to your contact list. Do you want to allow this?"
				).arg(protocolName).arg(contactName), i18n("Allow contact?")) == 3) // Yes == 3
		{
			//User said Yes
			myProtocol->addContact( contactId, displayName, parentContact, groupName, isTemporary );
			return true;
		} else {
			//User said No
			return false;
		}
		
	} else {
		//This protocol is not loaded
		KMessageBox::error( 0, i18n("An external application has attempted to add a contact using "
				" the %1 protocol, which does not exist, or is not loaded.").arg( protocolName ),
				i18n("Missing Protocol"));
		
		return false;
	}
}

KopeteGroup * KopeteContactList::getGroup(const QString& displayName, KopeteGroup::GroupType type)
{
	if( type == KopeteGroup::Temporary )
		return KopeteGroup::temporary;
	
	KopeteGroup *groupIterator;;
	for ( groupIterator = m_groupList.first(); groupIterator; groupIterator = m_groupList.next() )
	{
		if( groupIterator->type() == type && groupIterator->displayName() == displayName )
			return groupIterator;
	}

	KopeteGroup *newGroup = new KopeteGroup( displayName, type );
	addGroup( newGroup );
	return  newGroup;
}

#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

