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
	bool canAddMyself;
	/*
	 * Make sure we always have a lastSeen and a fullname property as long as
	 * a protocol is loaded
	 */
	PropertyTmpl mStickLastSeen;
	PropertyTmpl mStickFullName;

	Kopete::OnlineStatus accountNotConnectedStatus;
};

Protocol::Protocol( const KComponentData &instance, QObject *parent, bool canAddMyself )
: Plugin( instance, parent ), d(new Private())
{
	d->canAddMyself = canAddMyself;
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

bool Protocol::canAddMyself() const
{
	return d->canAddMyself;
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

	connect( self->account(), SIGNAL(destroyed()),
		this, SLOT(slotAccountDestroyed()) );

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
								  SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
								  this, SLOT(slotAccountOnlineStatusChanged(Kopete::Contact*)) );
				a->disconnect();
			}
			else
			{
				// Remove account, it's already disconnected
				kDebug( 14010 ) << a->accountId() <<
						" is already disconnected, deleting..." << endl;

				QObject::connect( a, SIGNAL(destroyed()),
								  this, SLOT(slotAccountDestroyed()) );
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



void Protocol::serialize( MetaContact *metaContact )
{
	QList< QMap<QString, QString> > serializedDataList;
	QListIterator<Contact *> cit(metaContact->contacts());
	while ( cit.hasNext() )
	{
		Contact *c = cit.next();
		if( c->protocol()->pluginId() != pluginId() )
			continue;

		QMap<QString, QString> sd;
		QMap<QString, QString> ad;

		const QString &nameType = Kopete::Contact::nameTypeToString(c->preferredNameType());

		// Preset the contactId and preferredNameType, if the plugin doesn't want to save
		// them, or use its own format, it can call clear() on the provided list
		sd[ QString::fromLatin1( "contactId" ) ] =   c->contactId();
		sd[ QString::fromLatin1( "preferredNameType" ) ] = nameType;
		if(c->account())
			sd[ QString::fromLatin1( "accountId" ) ] = c->account()->accountId();

		// If there's an index field preset it too
		QString index = c->protocol()->addressBookIndexField();
		if( !index.isEmpty() )
			ad[ index ] = c->contactId();

		c->serializeProperties( sd );
		c->serialize( sd, ad );

		serializedDataList.append( sd );
	}

	// Pass all returned fields to the contact list even if empty (will remove the old one)
	metaContact->setPluginContactData( this, serializedDataList );

#if 0
	// FIXME: This isn't used anywhere right now.
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
#endif
}

void Protocol::deserializeContactList( MetaContact *metaContact, const QList< QMap<QString, QString> > &dataList )
{
	foreach ( const ContactListElement::ContactData &sd, dataList )
	{
		const QString& accountId = sd[ QString::fromLatin1( "accountId" ) ];
		if( !d->canAddMyself && accountId == sd[ QString::fromLatin1( "contactId" ) ] )
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

void Protocol::deserialize( MetaContact *metaContact, const QMap<QString, QString> &data )
{
	Q_UNUSED( metaContact )
	Q_UNUSED( data )
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

