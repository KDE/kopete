/*
    kopeteaccount.cpp - Kopete Account

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>
    Copyright (c) 2003-2004 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopetegroup.h"
#include "kopetepassword.h"
#include "kopeteprefs.h"

namespace
{
QString configGroup( KopeteProtocol *protocol, const QString &accountId )
{
	return QString::fromLatin1( "Account_%2_%1" ).arg( accountId, protocol->pluginId() );
}

}

class KopeteAccountPrivate
{
public:
	KopeteAccountPrivate( KopeteProtocol *protocol, const QString &accountId )
	 : protocol( protocol ), id( accountId )
	 , password( configGroup( protocol, accountId ) )
	 , autologin( false ), priority( 0 ), myself( 0 )
	 , suppressStatusTimer( 0 ), suppressStatusNotification( false )
	{
	}

	KopeteProtocol *protocol;
	QString id;
	Kopete::Password password;
	bool autologin;
	uint priority;
	QDict<KopeteContact> contacts;
	QColor color;
	KopeteContact *myself;
	QTimer *suppressStatusTimer;
	bool suppressStatusNotification;
};

KopeteAccount::KopeteAccount( KopeteProtocol *parent, const QString &accountId, const char *name )
 : KopetePluginDataObject( parent, name ), d( new KopeteAccountPrivate( parent, accountId ) )
{
	d->suppressStatusTimer = new QTimer( this, "suppressStatusTimer" );
	QObject::connect( d->suppressStatusTimer, SIGNAL( timeout() ),
		this, SLOT( slotStopSuppression() ) );

	if ( KopeteAccountManager::manager()->registerAccount( this ) )
		QTimer::singleShot( 0, this, SLOT( slotAccountReady() ) );
	else
		deleteLater();
}

KopeteAccount::~KopeteAccount()
{
	// Delete all registered child contacts first
	while ( !d->contacts.isEmpty() )
		delete *QDictIterator<KopeteContact>( d->contacts );
	KopeteAccountManager::manager()->unregisterAccount( this );

	delete d;
}

void KopeteAccount::slotAccountReady()
{
	KopeteAccountManager::manager()->notifyAccountReady( this );
}

void KopeteAccount::connect( const KopeteOnlineStatus& )
{
	//do nothing
}

void KopeteAccount::disconnect( DisconnectReason reason )
{
	//reconnect if needed
	if ( KopetePrefs::prefs()->reconnectOnDisconnect() == true && reason > Manual )
		//use a timer to allow the plugins to clean up after return
		QTimer::singleShot(0, this, SLOT(connect()));
}

KopeteProtocol *KopeteAccount::protocol() const
{
	return d->protocol;
}

QString KopeteAccount::accountId() const
{
	return d->id;
}

const QColor KopeteAccount::color() const
{
	return d->color;
}

void KopeteAccount::setColor( const QColor &color )
{
	d->color = color;
}

void KopeteAccount::setPriority( uint priority )
{
 	d->priority = priority;
}

const uint KopeteAccount::priority() const
{
	return d->priority;
}

void KopeteAccount::setAccountId( const QString &accountId )
{
	if ( d->id != accountId )
	{
		d->id = accountId;
		emit ( accountIdChanged() );
	}
}

QPixmap KopeteAccount::accountIcon(const int size) const
{
	// FIXME: this code is duplicated with KopeteOnlineStatus, can we merge it somehow?
	QPixmap base = KGlobal::instance()->iconLoader()->loadIcon(
		d->protocol->pluginIcon(), KIcon::Small, size );

	if ( d->color.isValid() )
	{
		KIconEffect effect;
		base = effect.apply( base, KIconEffect::Colorize, 1, d->color, 0);
	}

	if ( size > 0 && base.width() != size )
	{
		base = QPixmap( base.convertToImage().smoothScale( size, size ) );
	}

	return base;
}

QString KopeteAccount::configGroup() const
{
	return ::configGroup( protocol(), accountId() );
}

void KopeteAccount::writeConfig( const QString &configGroupName )
{
	KConfig *config = KGlobal::config();
	config->setGroup( configGroupName );

	config->writeEntry( "Protocol", d->protocol->pluginId() );
	config->writeEntry( "AccountId", d->id );
	config->writeEntry( "Priority", d->priority );

	config->writeEntry( "AutoConnect", d->autologin );

	if ( d->color.isValid() )
		config->writeEntry( "Color", d->color );
	else
		config->deleteEntry( "Color" );

	// Store other plugin data
	KopetePluginDataObject::writeConfig( configGroupName );
}

void KopeteAccount::readConfig( const QString &configGroupName )
{
	kdDebug(14010) << k_funcinfo << "with group " << configGroupName << endl;

	KConfig *config = KGlobal::config();
	config->setGroup( configGroupName );

	d->autologin = config->readBoolEntry( "AutoConnect", false );
	d->color = config->readColorEntry( "Color", &d->color );
	d->priority = config->readNumEntry( "Priority", 0 );

	// Handle the plugin data, if any
	QMap<QString, QString> entries = config->entryMap( configGroupName );
	QMap<QString, QString>::Iterator entryIt;
	QMap<QString, QMap<QString, QString> > pluginData;
	for ( entryIt = entries.begin(); entryIt != entries.end(); ++entryIt )
	{
		if ( entryIt.key().startsWith( QString::fromLatin1( "PluginData_" ) ) )
		{
			QStringList data = QStringList::split( '_', entryIt.key(), true );
			data.pop_front(); // Strip "PluginData_" first
			QString pluginName = data.first();
			data.pop_front();

			// Join remainder and store it
			pluginData[ pluginName ][ data.join( QString::fromLatin1( "_" ) ) ] = entryIt.data();
		}
	}

	// Lastly, pass on the plugin data to the account
	QMap<QString, QMap<QString, QString> >::Iterator pluginDataIt;
	for ( pluginDataIt = pluginData.begin(); pluginDataIt != pluginData.end(); ++pluginDataIt )
	{
		KopetePlugin *plugin = KopetePluginManager::self()->plugin( pluginDataIt.key() );
		if ( plugin )
			setPluginData( plugin, pluginDataIt.data() );
		else
		{
			kdDebug( 14010 ) << k_funcinfo <<
				"No plugin object for id '" << pluginDataIt.key() << "'" << endl;
		}
	}

	kdDebug(14010) << k_funcinfo << "calling virtual void loaded()..." << endl;
	loaded();
	kdDebug(14010) << k_funcinfo << "virtual void loaded() returned." << endl;
}

void KopeteAccount::loaded()
{
	/* do nothing in default implementation */
}

QString KopeteAccount::password( bool error, bool *ok, unsigned int maxLength ) const
{
	d->password.setMaximumLength( maxLength );
	QString prompt;
	if ( error )
	{
		prompt = i18n( "<b>The password was wrong;</b> please re-enter your"\
			" password for %1 account <b>%2</b>" ).arg( protocol()->displayName(),
				accountId() );
	}
	else
	{
		prompt = i18n( "Please enter your password for %1 account <b>%2</b>" )
			.arg( protocol()->displayName(), accountId() );
	}

	QString pass = d->password.retrieve(
		accountIcon( Kopete::Password::preferredImageSize() ), prompt,
		error ? Kopete::Password::FromUser : Kopete::Password::FromConfigOrUser );

	if ( ok )
		*ok = !pass.isNull();
	return pass;
}

void KopeteAccount::setPassword( const QString &pass )
{
	d->password.set( pass );
}

void KopeteAccount::setAutoLogin( bool b )
{
	d->autologin = b;
}

bool KopeteAccount::autoLogin() const
{
	return d->autologin;
}

bool KopeteAccount::rememberPassword() const
{
	return d->password.remembered();
}

void KopeteAccount::registerContact( KopeteContact *c )
{
	d->contacts.insert( c->contactId(), c );
	QObject::connect( c, SIGNAL( contactDestroyed( KopeteContact * ) ),
		SLOT( slotKopeteContactDestroyed( KopeteContact * ) ) );
}

void KopeteAccount::slotKopeteContactDestroyed( KopeteContact *c )
{
	//kdDebug( 14010 ) << "KopeteProtocol::slotKopeteContactDestroyed: " << c->contactId() << endl;
	d->contacts.remove( c->contactId() );
}

const QDict<KopeteContact>& KopeteAccount::contacts()
{
	return d->contacts;
}


bool KopeteAccount::addContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact *parentContact, const AddMode mode, const QString &groupName, bool isTemporary )
{
	if ( contactId == accountId() )
	{
		kdDebug( 14010 ) << k_funcinfo <<
			"WARNING: the user try to add myself to his contactlist - abort" << endl;
		return false;
	}

	KopeteGroup *parentGroup = 0L;
	//If this is a temporary contact, use the temporary group
	if ( !groupName.isNull() )
		parentGroup = isTemporary ? KopeteGroup::temporary() : KopeteContactList::contactList()->getGroup( groupName );

	if(!parentGroup && parentContact)
		parentGroup=parentContact->groups().first();


	KopeteContact *c = d->contacts[ contactId ];
	if ( c && c->metaContact() )
	{
		if ( c->metaContact()->isTemporary() && !isTemporary )
		{
			kdDebug( 14010 ) <<
				"KopeteAccount::addContact: You are trying to add an existing temporary contact. Just add it on the list" << endl;
			/* //FIXME: calling this can produce a message to delete the old contazct which should be deleted in many case.
			if(c->metaContact() != parentContact)
				c->setMetaContact(parentContact);*/

			c->metaContact()->setTemporary(false, parentGroup );
			if(!KopeteContactList::contactList()->metaContacts().contains(c->metaContact()))
				KopeteContactList::contactList()->addMetaContact(c->metaContact());
		}
		else
		{
			// should we here add the contact to the parentContact if any?
			kdDebug( 14010 ) << "KopeteAccount::addContact: Contact already exists" << endl;
		}
		return false; //(the contact is not in the correct metacontact, so false)
	}

	if ( parentContact )
	{
		//If we are given a MetaContact to add to that is marked as temporary. but
		//this contact is not temporary, then change the metacontact to non-temporary
		if ( parentContact->isTemporary() && !isTemporary )
			parentContact->setTemporary( false, parentGroup );
		else
			parentContact->addToGroup( parentGroup );
	}
	else
	{
		//Create a new MetaContact
		parentContact = new KopeteMetaContact();
		//parentContact->setDisplayName( displayName );

		//Set it as a temporary contact if requested
		if ( isTemporary )
			parentContact->setTemporary( true );
		else
			parentContact->addToGroup( parentGroup );

		KopeteContactList::contactList()->addMetaContact( parentContact );
	}

	//Fix for 77835 (forward the metacontact name is displayName is empty)
	QString realDisplayName;
	if ( displayName.isEmpty() )
	{
		realDisplayName = parentContact->displayName();
	}
	else
	{
		realDisplayName = displayName;
	}

	if ( c )
	{
		c->setMetaContact( parentContact );
	}
	else
	{
		if ( !addContactToMetaContact( contactId, realDisplayName, parentContact ) )
			return false;
	}

	if ( mode == ChangeKABC )
	{
		kdDebug( 14010 ) << k_funcinfo << " changing KABC" << endl;
		parentContact->updateKABC();
	}
	/*else
		kdDebug( 14010 ) << k_funcinfo << " leaving KABC" << endl;*/
	return true;
}

KActionMenu * KopeteAccount::actionMenu()
{
	//default implementation
	return 0L;
}

bool KopeteAccount::isConnected() const
{
	return d->myself && ( d->myself->onlineStatus().status() != KopeteOnlineStatus::Offline ) ;
}

bool KopeteAccount::isAway() const
{
	return d->myself && ( d->myself->onlineStatus().status() == KopeteOnlineStatus::Away );
}

KopeteContact * KopeteAccount::myself() const
{
	return d->myself;
}

void KopeteAccount::setMyself( KopeteContact *myself )
{
	d->myself = myself;

	QObject::connect( myself, SIGNAL( onlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ),
		this, SLOT( slotOnlineStatusChanged( KopeteContact *, const KopeteOnlineStatus &, const KopeteOnlineStatus & ) ) );
}

void KopeteAccount::slotOnlineStatusChanged( KopeteContact * /* contact */,
	const KopeteOnlineStatus &newStatus, const KopeteOnlineStatus &oldStatus )
{
	if ( oldStatus.status() == KopeteOnlineStatus::Offline ||
		oldStatus.status() == KopeteOnlineStatus::Connecting ||
		newStatus.status() == KopeteOnlineStatus::Offline )
	{
		// Wait for five seconds until we treat status notifications for contacts
		// as unrelated to our own status change.
		// Five seconds may seem like a long time, but just after your own
		// connection it's basically neglectible, and depending on your own
		// contact list's size, the protocol you are using, your internet
		// connection's speed and your computer's speed you *will* need it.
		d->suppressStatusNotification = true;
		d->suppressStatusTimer->start( 5000, true );
	}
}

void KopeteAccount::slotStopSuppression()
{
	d->suppressStatusNotification = false;
}

bool KopeteAccount::suppressStatusNotification() const
{
	return d->suppressStatusNotification;
}

#include "kopeteaccount.moc"
// vim: set noet ts=4 sts=4 sw=4:
