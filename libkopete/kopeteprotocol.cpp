/*
    kopeteprotocol.cpp - Kopete Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprotocol.h"

#include <kdebug.h>
#include <kaction.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

class KopeteProtocolPrivate
{
public:
	bool unloading;
};

KopeteProtocol::KopeteProtocol( KInstance *instance, QObject *parent, const char *name )
: KopetePlugin( instance, parent, name )
{
	d = new KopeteProtocolPrivate;
	d->unloading = false;
}

KopeteProtocol::~KopeteProtocol()
{
	// Remove all active accounts
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );
	if ( !accounts.isEmpty() )
	{
		kdWarning( 14010 ) << k_funcinfo << "Deleting protocol with existing accounts! Did the account unloading go wrong?" << endl;

		for( QDictIterator<KopeteAccount> it( accounts ); it.current() ; ++it )
			delete *it;
	}

	delete d;
}

bool KopeteProtocol::supportsRichText() const
{
	//Return false by default
	return false;
}

KActionMenu* KopeteProtocol::protocolActions()
{
	QDict<KopeteAccount> dict=KopeteAccountManager::manager()->accounts(this);
	QDictIterator<KopeteAccount> it( dict );
	if(dict.count() == 1 )
	{
		return (it.current())->actionMenu();
	}

	KActionMenu *m_menu = new KActionMenu(displayName(),pluginIcon(),this);

	for( ; KopeteAccount *account = it.current(); ++it )
	{
		KActionMenu *accountMenu = account->actionMenu();
		if(accountMenu->parent())
			accountMenu->parent()->removeChild( accountMenu );
		m_menu->insertChild( accountMenu );
		m_menu->insert( accountMenu );
	}

	return m_menu;
}


void KopeteProtocol::slotMetaContactAboutToSave( KopeteMetaContact *metaContact )
{
	QMap<QString, QString> serializedData, sd;
	QMap<QString, QString> addressBookData, ad;
	QMap<QString, QString>::Iterator it;

	//kdDebug( 14010 ) << "KopeteProtocol::metaContactAboutToSave: protocol " << pluginId() << ": serializing " << metaContact->displayName() << endl;

	QPtrList<KopeteContact> contacts=metaContact->contacts();
	for (KopeteContact *c=contacts.first() ; c ; c=contacts.next() )
	{
		if( c->protocol()->pluginId() != pluginId() )
			continue;

		sd.clear();
		ad.clear();

		// Preset the contactId and displayName, if the plugin doesn't want to save
		// them, or use its own format, it can call clear() on the provided list
		sd[ QString::fromLatin1( "contactId" ) ] =   c->contactId();
		sd[ QString::fromLatin1( "displayName" ) ] = c->displayName();
		if(c->account())
			sd[ QString::fromLatin1( "accountId" ) ] = c->account()->accountId();


		// If there's an index field preset it too
		QString index = c->protocol()->addressBookIndexField();
		if( !index.isEmpty() )
			ad[ index ] = c->contactId();

		c->serialize( sd, ad );

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
	//if( !serializedData.isEmpty() ) //even if we are empty, that mean there are no contact, so remove old value
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
//			kdDebug(14010) << k_funcinfo << "metaContact->setAddressBookField( " << this << ", " << it.key() << ", \"All\", " << it.data() << " );" << endl;
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
		serializedData[ it.key() ] = QStringList::split( QChar( 0xE000 ), it.data(), true );
		serializedDataIterators[ it.key() ] = serializedData[ it.key() ].begin();
	}

	uint count = serializedData[QString::fromLatin1("contactId")].count();

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

		// Check if we have an account id. If not we're deserializing a Kopete 0.6 contact
		// (our our config is corrupted). Pick the first available account there. This
		// might not be what you want for corrupted accounts, but it's correct for people
		// who migrate from 0.6, as there's only one account in that case
		if( sd[ QString::fromLatin1( "accountId" ) ].isNull() )
		{
			QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );
			if ( accounts.count() > 0 )
			{
				sd[ QString::fromLatin1( "accountId" ) ] = QDictIterator<KopeteAccount>( accounts ).currentKey();
			}
			else
			{
				kdWarning() << k_funcinfo << "No account available and account not set in contactlist.xml either!" << endl
					<< "Not deserializing this contact." << endl;
				return;
			}
		}

		deserializeContact( metaContact, sd, ad );
	}
}

void KopeteProtocol::deserializeContact( KopeteMetaContact * /* metaContact */, const QMap<QString, QString> & /* serializedData */,
	const QMap<QString, QString> & /* addressBookData */ )
{
	/* Default implementation does nothing */
}

void KopeteProtocol::slotAccountOnlineStatusChanged( KopeteContact *self, const KopeteOnlineStatus &newStatus,
	const KopeteOnlineStatus & /* old */ )
{
	if ( !self || !self->account() || newStatus.status() != KopeteOnlineStatus::Offline )
		return;

	connect( self->account(), SIGNAL( destroyed( QObject * ) ),
		this, SLOT( slotAccountDestroyed( QObject * ) ) );

	self->account()->deleteLater();
}

void KopeteProtocol::slotAccountDestroyed( QObject * /* account */ )
{
	QDict<KopeteAccount> dict = KopeteAccountManager::manager()->accounts( this );
	if ( dict.isEmpty() )
	{
		// While at this point we are still in a stack trace from the destroyed
		// account it's safe to emit readyForUnload already, because it uses a
		// deleteLater rather than a delete for exactly this reason, to keep the
		// API managable
		emit( readyForUnload() );
	}
}

void KopeteProtocol::aboutToUnload()
{
	bool allDisconnected = true;

	d->unloading = true;

	// Disconnect all accounts
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );
	for ( QDictIterator<KopeteAccount> it( accounts ); it.current() ; ++it )
	{
		if ( it.current()->myself() && it.current()->myself()->isOnline() )
		{
			kdDebug( 14010 ) << k_funcinfo << it.current()->accountId() << " is still connected, disconnecting..." << endl;

			QObject::connect( it.current()->myself(),
				SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
				this, SLOT( slotAccountOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ) );
			it.current()->disconnect();

			allDisconnected = false;
		}
		else
		{
			// Remove account, it's already disconnected
			kdDebug( 14010 ) << k_funcinfo << it.current()->accountId() << " is already disconnected, deleting..." << endl;

			connect( it.current(), SIGNAL( destroyed( QObject * ) ), this, SLOT( slotAccountDestroyed( QObject * ) ) );
			it.current()->deleteLater();
		}
	}
}

#include "kopeteprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:

