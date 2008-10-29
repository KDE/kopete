/*
    kopeteprotocol.cpp - Kopete Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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
#include <klocale.h>
#include <kjob.h>

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopeteglobal.h"
#include "kopeteproperty.h"
#include "kopetemetacontact.h"

namespace Kopete
{

class Protocol::Private
{
public:
	bool unloading;
	Protocol::Capabilities capabilities;
	/*
	 * Make sure we always have a lastSeen and a fullname property as long as
	 * a protocol is loaded
	 */
	PropertyTmpl mStickLastSeen;
	PropertyTmpl mStickFullName;

	Kopete::OnlineStatus accountNotConnectedStatus;
};

Protocol::Protocol( const KComponentData &instance, QObject *parent )
: Plugin( instance, parent )
{
	d = new Private;
	d->mStickLastSeen = Global::Properties::self()->lastSeen();
	d->mStickFullName = Global::Properties::self()->fullName();
	d->unloading = false;
	d->accountNotConnectedStatus = Kopete::OnlineStatus( Kopete::OnlineStatus::Unknown, 0, this, Kopete::OnlineStatus::AccountOffline, QStringList(QString::fromLatin1( "account_offline_overlay" )), i18n( "Account Offline" ) );
}

Protocol::~Protocol()
{
	// Remove all active accounts
	foreach( Account *a , AccountManager::self()->accounts() )
	{
		if( a->protocol() == this )
		{
			kWarning( 14010 ) << "Deleting protocol with existing accounts! Did the account unloading go wrong?  account: " 
					<< a->accountId() << endl;
		
			delete a;
		}
	}
	delete d;
}

Protocol::Capabilities Protocol::capabilities() const
{
	return d->capabilities;
}

void Protocol::setCapabilities( Protocol::Capabilities capabilities )
{
	d->capabilities = capabilities;
}


Kopete::OnlineStatus Protocol::accountOfflineStatus() const
{
	return d->accountNotConnectedStatus;
}

void Protocol::slotAccountOnlineStatusChanged( Contact *self )
{//slot connected in aboutToUnload
	if ( !self || !self->account() || self->account()->isConnected())
		return;
	// some protocols change status several times during shutdown.  We should only call deleteLater() once
	disconnect( self, 0, this, 0 );

	connect( self->account(), SIGNAL(destroyed( )),
		this, SLOT( slotAccountDestroyed( ) ) );

	self->account()->deleteLater();
}

void Protocol::slotAccountDestroyed( )
{
	foreach( Account *a , AccountManager::self()->accounts() )
	{
		if( a->protocol() == this )
		{
			//all accounts has not been deleted yet
			return;
		}
	}

	// While at this point we are still in a stack trace from the destroyed
	// account it's safe to emit readyForUnload already, because it uses a
	// deleteLater rather than a delete for exactly this reason, to keep the
	// API managable
	emit( readyForUnload() );
}

void Protocol::aboutToUnload()
{

	d->unloading = true;

	int accountcountcount=0;
	// Disconnect all accounts
	foreach( Account *a , AccountManager::self()->accounts() )
	{
		if( a->protocol() == this )
		{
			accountcountcount++;
			
			if ( a->myself() && a->myself()->isOnline() )
			{
				kDebug( 14010 ) << a->accountId() <<
						" is still connected, disconnecting..." << endl;

				QObject::connect( a->myself(),
								  SIGNAL( onlineStatusChanged( Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus & ) ),
								  this, SLOT( slotAccountOnlineStatusChanged( Kopete::Contact * ) ) );
				a->disconnect();
			}
			else
			{
				// Remove account, it's already disconnected
				kDebug( 14010 ) << a->accountId() <<
						" is already disconnected, deleting..." << endl;

				QObject::connect( a, SIGNAL( destroyed( ) ),
								  this, SLOT( slotAccountDestroyed( ) ) );
				a->deleteLater();
			}
		}
	}

	if( accountcountcount == 0 )
	{
		//no accounts in there anymore ,   we can unload safelly
		emit readyForUnload();
	}
}



void Protocol::slotMetaContactAboutToSave( MetaContact *metaContact )
{
	QMap<QString, QString> serializedData, sd;
	QMap<QString, QString> addressBookData, ad;
	QMap<QString, QString>::Iterator it;

	//kDebug( 14010 ) << "Protocol::metaContactAboutToSave: protocol " << pluginId() << ": serializing " << metaContact->displayName();

	QListIterator<Contact *> cit(metaContact->contacts());
	while ( cit.hasNext() )
	{
		Contact *c = cit.next();
		if( c->protocol()->pluginId() != pluginId() )
			continue;

		sd.clear();
		ad.clear();

		// Preset the contactId and displayName, if the plugin doesn't want to save
		// them, or use its own format, it can call clear() on the provided list
		sd[ QString::fromLatin1( "contactId" ) ] =   c->contactId();
		//TODO(nick) remove
		sd[ QString::fromLatin1( "displayName" ) ] = c->property(Global::Properties::self()->nickName()).value().toString();
		if(c->account())
			sd[ QString::fromLatin1( "accountId" ) ] = c->account()->accountId();

		// If there's an index field preset it too
		QString index = c->protocol()->addressBookIndexField();
		if( !index.isEmpty() )
			ad[ index ] = c->contactId();

		c->serializeProperties( sd );
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
				serializedData[ it.key() ] = serializedData[ it.key() ] + QChar( 0xE000 ) + it.value();
			else
				serializedData[ it.key() ] = it.value();
		}

		for( it = ad.begin(); it != ad.end(); ++it )
		{
			if( addressBookData.contains( it.key() ) )
				addressBookData[ it.key() ] = addressBookData[ it.key() ] + QChar( 0xE000 ) + it.value();
			else
				addressBookData[ it.key() ] = it.value();
		}
	}

	// Pass all returned fields to the contact list
	//if( !serializedData.isEmpty() ) //even if we are empty, that mean there are no contact, so remove old value
	metaContact->setPluginData( this, serializedData );

	for( it = addressBookData.begin(); it != addressBookData.end(); ++it )
	{
		//kDebug( 14010 ) << "Protocol::metaContactAboutToSave: addressBookData: key: " << it.key() << ", data: " << it.data();
		// FIXME: This is a terrible hack to check the key name for the phrase "messaging/"
		//        to indicate what app name to use, but for now it's by far the easiest
		//        way to get this working.
		//        Once all this is in CVS and the actual storage in libkabc is working
		//        we can devise a better API, but with the constantly changing
		//        requirements every time I learn more about kabc I'd better no touch
		//        the API yet - Martijn
		if( it.key().startsWith( QString::fromLatin1( "messaging/" ) ) )
		{
			metaContact->setAddressBookField( this, it.key(), QString::fromLatin1( "All" ), it.value() );
//			kDebug(14010) << "metaContact->setAddressBookField( " << this << ", " << it.key() << ", \"All\", " << it.data() << " );";
		}
		else
			metaContact->setAddressBookField( this, QString::fromLatin1( "kopete" ), it.key(), it.value() );
	}
}

void Protocol::deserialize( MetaContact *metaContact, const QMap<QString, QString> &data )
{
	/*kDebug( 14010 ) << "Protocol::deserialize: protocol " <<
		pluginId() << ": deserializing " << metaContact->displayName() << endl;*/
	
	QMap<QString, QStringList> serializedData;
	QMap<QString, QStringList::Iterator> serializedDataIterators;
	QMap<QString, QString>::ConstIterator it;
	for( it = data.begin(); it != data.end(); ++it )
	{
		serializedData[ it.key() ] = it.value().split( QChar( 0xE000 ), QString::KeepEmptyParts );
		serializedDataIterators[ it.key() ] = serializedData[ it.key() ].begin();
	}

	int count = serializedData[QString::fromLatin1("contactId")].count();

	// Prepare the independent entries to pass to the plugin's implementation
	for( int i = 0; i < count ; i++ )
	{
		QMap<QString, QString> sd;
#ifdef __GNUC__
#warning  write this properly
#endif
#if 0	
		QMap<QString, QStringList::Iterator>::Iterator serializedDataIt;
		for( serializedDataIt = serializedDataIterators.begin(); serializedDataIt != serializedDataIterators.end(); ++serializedDataIt )
		{
			sd[ serializedDataIt.key() ] = *( serializedDataIt.data() );
			++( serializedDataIt.data() );
		}
		
#else
		QMap<QString, QStringList>::Iterator serializedDataIt;
		QMap<QString, QStringList>::Iterator serializedDataItEnd = serializedData.end();
		for( serializedDataIt = serializedData.begin(); serializedDataIt != serializedDataItEnd; ++serializedDataIt )
		{
			QStringList sl=serializedDataIt.value();
			if(sl.count()>i)
				sd[ serializedDataIt.key() ] = sl[i];
		}
	
	
#endif

		const QString& accountId=sd[ QString::fromLatin1( "accountId" ) ];
		// myself was allowed in the contact list in old version of kopete.
		// But if one keep it on the contact list now, it may conflict witht he myself metacontact.
		// So ignore it
		if(accountId == sd[ QString::fromLatin1( "contactId" ) ] )
		{
			kDebug( 14010 ) << "Myself contact was on the contactlist.xml for account " << accountId << ".  Ignore it";
			continue;
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
		

#if 0
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
		if( accountId.isNull() )
		{
			Q3Dict<Account> accounts = AccountManager::self()->accounts( this );
			if ( accounts.count() > 0 )
			{
				sd[ QString::fromLatin1( "accountId" ) ] = Q3DictIterator<Account>( accounts ).currentKey();
			}
			else
			{
				kWarning( 14010 ) <<
					"No account available and account not set in " \
					"contactlist.xml either!" << endl
					<< "Not deserializing this contact." << endl;
				return;
			}
		}
#endif


		Contact *c = deserializeContact( metaContact, sd, ad );
		if (c) // should never be null but I do not like crashes
			c->deserializeProperties( sd );
	}
}

Contact *Protocol::deserializeContact(
	MetaContact * metaContact,
	const QMap<QString, QString> & serializedData,
	const QMap<QString, QString> & addressBookData )
{
	Q_UNUSED( metaContact )
	Q_UNUSED( serializedData )
	Q_UNUSED( addressBookData )
	/* Default implementation does nothing */
	return 0;
}

KJob *Protocol::createProtocolTask(const QString &taskType)
{
	// Default implementation does nothing
	Q_UNUSED( taskType )
	return 0;
}

bool Protocol::validatePassword( const QString & password ) const
{
	Q_UNUSED( password )
    return true;
}

} //END namespace Kopete

#include "kopeteprotocol.moc"

