/*
    kopeteprotocol.cpp - Kopete Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qstringlist.h>

#include <kdebug.h>

#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"

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
//	kdDebug(14010) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	m_contacts.remove( c->contactId() );
}

void KopeteProtocol::slotMetaContactAboutToSave( KopeteMetaContact *metaContact )
{
	QMap<QString, QString> serializedData, sd;
	QMap<QString, QString> addressBookData, ad;
	QMap<QString, QString>::Iterator it;

	QDict<KopeteContact> mcContacts = contacts( metaContact );
	if( mcContacts.isEmpty() )
		return;

//	kdDebug( 14010 ) << "KopeteProtocol::metaContactAboutToSave: protocol " << pluginId() << ": serializing " << metaContact->displayName() << endl;
	QDictIterator<KopeteContact> contactIt( mcContacts );
	for( ; contactIt.current() ; ++contactIt )
	{
		sd.clear();
		ad.clear();

		// Preset the contactId and displayName, if the plugin doesn't want to save
		// them, or use its own format, it can call clear() on the provided list
		sd[ QString::fromLatin1( "contactId" ) ] =   contactIt.current()->contactId();
		sd[ QString::fromLatin1( "displayName" ) ] = contactIt.current()->displayName();

		// If there's an index field preset it too
		QString index = contactIt.current()->protocol()->addressBookIndexField();
		if( !index.isEmpty() )
			ad[ index ] = contactIt.current()->contactId();

		contactIt.current()->serialize( sd, ad );

		// Merge the returned fields with what we already (may) have
		for( it = sd.begin(); it != sd.end(); ++it )
		{
			// The Unicode chars E000-F800 are non-printable and reserved for
			// private use in applications. For more details, see also
			// http://www.unicode.org/charts/PDF/UE000.pdf.
			// Inside libkabc the use of QChar( 0xE000 ) has been standardized
			// as separator for the string lists, use this also for the 'normal'
			// serialized data.
			if( serializedData.contains( it.key() ) )
				serializedData[ it.key() ] = serializedData[ it.key() ] + QChar( 0xE000 ) + it.data();
			else
				serializedData[ it.key() ] = it.data();
		}

		for( it = ad.begin(); it != ad.end(); ++it )
		{
			if( addressBookData.contains( it.key() ) )
				addressBookData[ it.key() ] = addressBookData[ it.key() ] + QChar( 0xE000 ) + it.data();
			else
				addressBookData[ it.key() ] = it.data();
		}
	}

	// Pass all returned fields to the contact list
	if( !serializedData.isEmpty() )
		metaContact->setPluginData( this, serializedData );

	for( it = addressBookData.begin(); it != addressBookData.end(); ++it )
	{
		//kdDebug( 14010 ) << "KopeteProtocol::metaContactAboutToSave: addressBookData: key: " << it.key() << ", data: " << it.data() << endl;
		// FIXME: This is a terrible hack to check the key name for the phrase "messaging/"
		//        to indicate what app name to use, but for now it's by far the easiest
		//        way to get this working.
		//        Once all this is in CVS and the actual storage in libkabc is working
		//        we can devise a better API, but with the constantly changing
		//        requirements every time I learn more about kabc I'd better no touch
		//        the API yet - Martijn
		if( it.key().startsWith( QString::fromLatin1( "messaging/" ) ) )
		{
			metaContact->setAddressBookField( this, it.key(), QString::fromLatin1( "All" ), it.data() );
//			kdDebug() << k_funcinfo << "metaContact->setAddressBookField( " << this << ", " << it.key() << ", \"All\", " << it.data() << " );" << endl;
		}
		else
			metaContact->setAddressBookField( this, QString::fromLatin1( "kopete" ), it.key(), it.data() );
	}
}

void KopeteProtocol::deserialize( KopeteMetaContact *metaContact, const QMap<QString, QString> &data )
{
	//kdDebug( 14010 ) << "KopeteProtocol::deserialize: protocol " << pluginId() << ": deserializing " << metaContact->displayName() << endl;

	QMap<QString, QStringList> serializedData;
	QMap<QString, QStringList::Iterator> serializedDataIterators;
	QMap<QString, QString>::ConstIterator it;
	for( it = data.begin(); it != data.end(); ++it )
	{
		serializedData[ it.key() ] = QStringList::split( QChar( 0xE000 ), it.data() );
		serializedDataIterators[ it.key() ] = serializedData[ it.key() ].begin();
	}

	uint count = serializedData.begin().data().count();

	// Prepare the independent entries to pass to the plugin's implementation
	for( uint i = 0; i < count ; i++ )
	{
		QMap<QString, QString> sd;
		QMap<QString, QStringList::Iterator>::Iterator serializedDataIt;
		for( serializedDataIt = serializedDataIterators.begin(); serializedDataIt != serializedDataIterators.end(); ++serializedDataIt )
		{
			sd[ serializedDataIt.key() ] = *( serializedDataIt.data() );
			++( serializedDataIt.data() );
		}

		// FIXME: This code almost certainly breaks when having more than
		//        one contact in a meta contact. There are solutions, but
		//        they are all hacky and the API needs revision anyway (see
		//        FIXME a few lines below :), so I'm not going to add that
		//        for now.
		//        Note that even though it breaks, the current code will
		//        never notice, since none of the plugins use the address
		//        book data in the deserializer yet, only when serializing.
		//        - Martijn
		QMap<QString, QString> ad;
		QStringList kabcFields = addressBookFields();
		for( QStringList::Iterator fieldIt = kabcFields.begin(); fieldIt != kabcFields.end(); ++fieldIt )
		{
			// FIXME: This hack is even more ugly, and has the same reasons as the similar
			//        hack in the serialize code.
			//        Once this code is actually capable of talking to kabc this hack
			//        should be removed ASAP! - Martijn
			if( ( *fieldIt ).startsWith( QString::fromLatin1( "messaging/" ) ) )
				ad[ *fieldIt ] = metaContact->addressBookField( this, *fieldIt, QString::fromLatin1( "All" ) );
			else
				ad[ *fieldIt ] = metaContact->addressBookField( this, QString::fromLatin1( "kopete" ), *fieldIt );
		}

		deserializeContact( metaContact, sd, ad );
	}
}

void KopeteProtocol::deserializeContact( KopeteMetaContact * /* metaContact */, const QMap<QString, QString> & /* serializedData */,
	const QMap<QString, QString> & /* addressBookData */ )
{
	/* Default implementation does nothing */
}

bool KopeteProtocol::addContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact, const QString &groupName, bool isTemporary )
{
//	kdDebug(14010) << "[KopeteProtocol] addMetaContact() contactId:" << contactId << "; displayName: " << displayName
//		<< "; groupName: " << groupName  << endl;

	KopeteGroup *parentGroup=0L;
	//If this is a temporary contact, use the temporary group
	if(!groupName.isNull())
		parentGroup = isTemporary ? KopeteGroup::temporary : KopeteContactList::contactList()->getGroup( groupName );

	if( parentContact )
	{
		//If we are given a MetaContact to add to that is marked as temporary. but
		//this contact is not temporary, then change the metacontact to non-temporary
		if( parentContact->isTemporary() && !isTemporary )
			parentContact->setTemporary( false, parentGroup );
		else
			parentContact->addToGroup( parentGroup );
	}
	else
	{
		//Check if this MetaContact exists
		parentContact = KopeteContactList::contactList()->findContact( QString::fromLatin1( pluginId() ), QString::null, contactId );
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
	kdDebug(14010) << "KopeteProtocol::addContactToMetaContact() Not Implemented!!!" << endl;
	return false;
}

#include "kopeteprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

