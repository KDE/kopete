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

#include "kopetecontactlistview.h"
#include "kopetecontactlist.h"

#include "kopetemetacontact.h"
#include "kopetemetacontactlvi.h"

#include "kopete.h"
#include "pluginloader.h"
#include "kopeteprotocol.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdebug.h>
#include <qstringlist.h>

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
	QString xml_filename;

	m_dom = new QDomDocument("ContactList");
	xml_filename = locateLocal("data","kopete/contacts.xml");

    /* No contacts */
	if ( xml_filename.isNull() )
		return ;

	QFile xml_file(xml_filename);
	xml_file.open(IO_ReadWrite);
	m_dom->setContent(&xml_file);

	QDomElement list = m_dom->documentElement();

	QDomNode nodel1;
	nodel1 = list.firstChild();

	while ( ! nodel1.isNull() )
	{
		QDomElement elementl1 = nodel1.toElement();

		if ( ! elementl1.isNull())
		{
			// We have found a metacontact person
			if ( elementl1.tagName() == "person" )
			{
				QString person_name = elementl1.attribute("name", "No Name");
				kdDebug() << "XML Reader: New Person " << person_name << endl;
				KopeteMetaContact *mc = new KopeteMetaContact();
				mc->setDisplayName(person_name);

				// Now we have to find all contacts and metadata for this person
				QDomNode nodel2;
				nodel2 = nodel1.firstChild();

				while ( !nodel2.isNull() )
				{
					// try to convert it to an element
					QDomElement elementl2 = nodel2.toElement();

					// Was it an element ?
					if( !elementl2.isNull())
					{
						// We have found a plugin contact
						if( elementl2.tagName() == "contact" )
						{
							QString protocol = elementl2.attribute("protocol", "Unknown");
							QString serializedData = elementl2.attribute("data", "none");
							kdDebug() << "XML Reader: Protocol: " << protocol
								<< " Data: " << serializedData << endl;
							KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>(
								kopeteapp->libraryLoader()->searchByID( protocol ) );
							if( proto )
							{
								KopeteContact *c = proto->createContact( mc, serializedData );
								c->setDisplayName(mc->displayName()); //FIXME: Protocols should do that theirself

								// FIXME: Retrieve the groups somewhere
								mc->addContact( c, QStringList() );
							}
							else
								kdDebug() << "Protocol " << protocol << " could not be found!" << endl;
						}
						if ( elementl2.tagName() == "metadata" )
						{
							QString pluginid = elementl2.attribute("pluginid", "Ups");
							QString mdkey = elementl2.attribute("key", "No key");
							kdDebug() << "XML Reader: \tNew Metadata PluginID: " << pluginid << " Key: " << mdkey << endl;
							//FIXME metadata is not supported

						}

						if ( elementl2.tagName() == "group" )
						{
                            QString groupname = elementl2.attribute("name", "Ups");
                            kdDebug() << "Group: "<< groupname << endl;
							mc->addToGroup(groupname);
						}
					}

					/* We go for the next contact, metadata, etc */
					nodel2 = nodel2.nextSibling();
				}

            	/* We add the metacontact */
				KopeteContactList::contactList()->addMetaContact( mc );
			}
		}
		nodel1 = nodel1.nextSibling();
	}

	/* All ok! */
	xml_file.close();
}

void KopeteContactList::saveXML()
{
	QString xml_filename = locateLocal("data","kopete/contacts.xml");
	QFile xml_file( xml_filename );

	xml_file.open(IO_ReadWrite | IO_Append);
	QTextStream stream(&xml_file);

	stream.setEncoding(QTextStream::UnicodeUTF8);
	
	stream << toXML();

	stream.device()->flush();
	xml_file.close();
}

QString KopeteContactList::toXML()
{
	QString xml;

	xml = "<list>\n";

	QPtrListIterator<KopeteMetaContact> it( m_contacts );
	for( ; it.current(); ++it )
	{
		xml = xml + "\t" + (it.current())->toXML() + "\n";	
	}
	xml = xml + "</list>";

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
	// FIXME: having this in a GUI class is horrible to put it mildly...
	return kopeteapp->contactList()->groups();
}

#include "kopetecontactlist.moc"

// vim: set noet ts=4 sts=4 sw=4:

