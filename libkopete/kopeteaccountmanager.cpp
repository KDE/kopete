/*
    kopeteaccountmanager.cpp - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart@kde.org>

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

#include "kopeteaccountmanager.h"

#include <QtGui/QApplication>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtCore/QHash>
#include <QtDBus/QDBusInterface>

#include <ksharedconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kplugininfo.h>
#include <kconfiggroup.h>
#include <solid/networking.h>
#include <solid/powermanagement.h>

#include "kopeteaccount.h"
#include "kopetebehaviorsettings.h"
#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteidentitymanager.h"
#include "kopetepluginmanager.h"
#include "kopetestatusitems.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopetestatusmanager.h"


namespace Kopete {

static int compareAccountsByPriority( Account *a, Account *b )
{
	uint priority1 = a->priority();
	uint priority2 = b->priority();

	if( a==b ) //two account are equal only if they are equal :-)
		return 0;  // remember than an account can be only once on the list, but two account may have the same priority when loading
	else if( priority1 > priority2 )
		return 1;
	else
		return -1;
}

class AccountManager::Private
{
public:
	QList<Account *> accounts;
	QList<Account *> accountsToBeRemoved;
	bool suspended;
	Kopete::StatusMessage suspendedStatusMessage;
	uint suspendedStatusCategory;
};

AccountManager * AccountManager::s_self = 0L;

AccountManager * AccountManager::self()
{
	if ( !s_self )
		s_self = new AccountManager;

	return s_self;
}


AccountManager::AccountManager()
: QObject( qApp ), d(new Private())
{
	setObjectName( "KopeteAccountManager" );
	connect( Solid::Networking::notifier(), SIGNAL(shouldConnect()), this, SLOT(networkConnected()) );
	connect( Solid::Networking::notifier(), SIGNAL(shouldDisconnect()), this, SLOT(networkDisconnected()) );
	connect( Solid::PowerManagement::notifier(), SIGNAL(resumingFromSuspend()), this, SLOT(resume()) );
#ifdef __GNUC__
#warning TODO: Switch to a org.kde.Solid.PowerManagement Sleeping/Suspending signal when available.
#endif
	QDBusConnection::systemBus().connect( "org.freedesktop.UPower", "/org/freedesktop/UPower", "", "Sleeping", this, SLOT( suspend() ) );
	d->suspended = false;
}


AccountManager::~AccountManager()
{
	s_self = 0L;

	delete d;
}

bool AccountManager::isAnyAccountConnected() const
{
	foreach( Account *a , d->accounts )
	{
		if( a->isConnected() )
			return true;
	}

	return false;
}

void AccountManager::setOnlineStatus( uint category, const Kopete::StatusMessage &statusMessage, uint flags, bool forced )
{
	kDebug() << "category: " << category << "status title: " << statusMessage.title() << "status message: " << statusMessage.message();
	OnlineStatusManager::Categories categories
		= (OnlineStatusManager::Categories)category;
	const bool onlyChangeConnectedAccounts = ( !forced && isAnyAccountConnected() );
	d->suspended = false;

	foreach( Account *account, d->accounts )
	{
		Kopete::OnlineStatus status = OnlineStatusManager::self()->onlineStatus( account->protocol(), categories );
		// Going offline is always respected
		if ( category & Kopete::OnlineStatusManager::Offline ) {
			account->setOnlineStatus( status, statusMessage );
			continue;
		}
		
		if ( onlyChangeConnectedAccounts ) { //If global status is offline, change all account to new status
			if ( account->isConnected() || ( ( (flags & ConnectIfOffline) || Kopete::StatusManager::self()->globalStatusCategory() == Kopete::OnlineStatusManager::Offline ) && !account->excludeConnect() ) )
				account->setOnlineStatus( status, statusMessage );
		}
		else {
			if ( !account->excludeConnect() )
				account->setOnlineStatus( status, statusMessage );
		}
	}
	// mark ourselves as globally away if appropriate
	Kopete::StatusManager::self()->setGlobalStatus( category, statusMessage );
}

void AccountManager::setOnlineStatus( uint category, const Kopete::StatusMessage &statusMessage, uint flags )
{
    setOnlineStatus(category, statusMessage, flags, false);
}

void AccountManager::setStatusMessage(const QString &message)
{
	foreach( Account *account, d->accounts )
	{
		account->setStatusMessage(message);
	}
}

void AccountManager::suspend()
{
	if ( d->suspended )
		return;

	d->suspended = true;
	d->suspendedStatusMessage = Kopete::StatusManager::self()->globalStatusMessage();
	d->suspendedStatusCategory = Kopete::StatusManager::self()->globalStatusCategory();

	Kopete::StatusMessage statusMessage( i18n( "Offline" ), "" );
	QList <Kopete::Status::StatusItem *> statusList = Kopete::StatusManager::self()->getRootGroup()->childList();
	//find first Status for OffineStatus
	for ( QList <Kopete::Status::StatusItem *>::ConstIterator it = statusList.constBegin(); it != statusList.constEnd(); ++it )
	{
		if ( ! (*it)->isGroup() && (*it)->category() == Kopete::OnlineStatusManager::Offline )
		{
			QString message, title;
			title = (*it)->title();
			message = (static_cast <Kopete::Status::Status*> (*it))->message(); //if it is not group, it's status
			statusMessage.setTitle( title );
			statusMessage.setMessage( message );
			break;
		}
	}

	foreach( Account *account, d->accounts )
	{
		account->suspend( statusMessage );
	}
	Kopete::StatusManager::self()->setGlobalStatus( Kopete::OnlineStatusManager::Offline, statusMessage );
}

bool AccountManager::resume()
{
	bool networkAvailable = ( Solid::Networking::status() == Solid::Networking::Unknown || Solid::Networking::status() == Solid::Networking::Connected );
	if ( !d->suspended || !networkAvailable )
		return false;

	foreach( Account *account, d->accounts )
	{
		account->resume();
	}
	Kopete::StatusManager::self()->setGlobalStatus( d->suspendedStatusCategory, d->suspendedStatusMessage );
	d->suspended = false;
	return true;
}

QColor AccountManager::guessColor( Protocol *protocol ) const
{
	// In a perfect wold, we should check if the color is actually not used by the account.
	// Anyway, this is not really required,  It would be a difficult job for about nothing more.
	//   -- Olivier
	int protocolCount = 0;

	for ( QListIterator<Account *> it( d->accounts ); it.hasNext(); )
	{
		Account *a = it.next();
		if ( a->protocol()->pluginId() == protocol->pluginId() )
			protocolCount++;
	}

	// let's figure a color
	QColor color;
	switch ( protocolCount % 7 )
	{
	case 0:
		color = QColor();
		break;
	case 1:
		color = Qt::red;
		break;
	case 2:
		color = Qt::green;
		break;
	case 3:
		color = Qt::blue;
		break;
	case 4:
		color = Qt::yellow;
		break;
	case 5:
		color = Qt::magenta;
		break;
	case 6:
		color = Qt::cyan;
		break;
	}

	return color;
}

Account* AccountManager::registerAccount( Account *account )
{
	if( !account || d->accounts.contains( account ) )
		return account;

	if( account->accountId().isEmpty() )
	{
		account->deleteLater();
		return 0L;
	}

	// If this account already exists, do nothing
	QListIterator<Account *> it( d->accounts );
	while ( it.hasNext() )
	{
		Account *curracc = it.next();
		if ( ( account->protocol() == curracc->protocol() ) && ( account->accountId() == curracc->accountId() ) )
		{
			account->deleteLater();
			return 0L;
		}
	}

	d->accounts.append( account );
	qSort( d->accounts.begin(), d->accounts.end(), compareAccountsByPriority );

	// Connect to the account's status changed signal
	connect(account->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
		this, SLOT(slotAccountOnlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)));

	connect(account, SIGNAL(accountDestroyed(const Kopete::Account*)) , this, SLOT(unregisterAccount(const Kopete::Account*)));

	if ( !account->identity() )
	{
		// the account's Identity must be set here instead of in the Kopete::Account ctor, because there the
		// identity cannot pick up any state set in the derived Account ctor
		Identity *identity = Kopete::IdentityManager::self()->findIdentity( account->configGroup()->readEntry("Identity", QString()) );
		// if the identity was not found, use the default one which will for sure exist
		// FIXME: get rid of this, the account's identity should always exist at this point
		if (!identity)
		{
			kWarning( 14010 ) << "No identity for account " << account->accountId() << ": falling back to default";
			identity = Kopete::IdentityManager::self()->defaultIdentity();
		}
		account->setIdentity( identity );
	}

	emit accountRegistered( account );
	return account;
}

void AccountManager::unregisterAccount( const Account *account )
{
	kDebug( 14010 ) << "Unregistering account " << account->accountId();
	d->accounts.removeAll( const_cast<Account*>(account) );
	emit accountUnregistered( account );
}

const QList<Account *>& AccountManager::accounts() const
{
	return d->accounts;
}

QList<Account*> AccountManager::accounts( Protocol* protocol ) const
{
	QList<Account*> protocolAccounts;
	foreach( Account* acct, d->accounts )
	{
		if ( acct->protocol() == protocol )
			protocolAccounts.append( acct );
	}
	return protocolAccounts;
}

Account * AccountManager::findAccount( const QString &protocolId, const QString &accountId )
{
	for ( QListIterator<Account *> it( d->accounts ); it.hasNext(); )
	{
		Account *a = it.next();
		if ( a->protocol()->pluginId() == protocolId && a->accountId() == accountId )
			return a;
	}
	return 0L;
}

void AccountManager::removeAccount( Account *account )
{
	if( !account->removeAccount() )
		return;

	if ( !account->isConnected() )
	{
		d->accountsToBeRemoved.append( account );
		QTimer::singleShot( 0, this, SLOT(removeAccountInternal()) );
	}
	else
	{
		kDebug( 14010 ) << account->accountId() << " is still connected, disconnecting...";
		connect( account, SIGNAL(isConnectedChanged()), this, SLOT(removeAccountConnectedChanged()) );
		account->disconnect();
	}
}

void AccountManager::save()
{
	//kDebug( 14010 ) ;
	qSort( d->accounts.begin(), d->accounts.end(), compareAccountsByPriority );

	for ( QListIterator<Account *> it( d->accounts ); it.hasNext(); )
	{
		Account *a = it.next();
		KConfigGroup *config = a->configGroup();

		config->writeEntry( "Protocol", a->protocol()->pluginId() );
		config->writeEntry( "AccountId", a->accountId() );
	}

	KGlobal::config()->sync();
}

void AccountManager::load()
{
	connect( PluginManager::self(), SIGNAL(pluginLoaded(Kopete::Plugin*)),
		this, SLOT(slotPluginLoaded(Kopete::Plugin*)) );

	// Iterate over all groups that start with "Account_" as those are accounts
	// and load the required protocols if the account is enabled.
	// Don't try to optimize duplicate calls out, the plugin queue is smart enough
	// (and fast enough) to handle that without adding complexity here
	KSharedConfig::Ptr config = KGlobal::config();
	QStringList accountGroups = config->groupList().filter( QRegExp( QString::fromLatin1( "^Account_" ) ) );
	for ( QStringList::Iterator it = accountGroups.begin(); it != accountGroups.end(); ++it )
	{
		KConfigGroup cg( config, *it );
		KConfigGroup pluginConfig( config, QLatin1String("Plugins") );

		QString protocol = cg.readEntry( "Protocol", QString() );
		if ( protocol.endsWith( QString::fromLatin1( "Protocol" ) ) )
			protocol = QString::fromLatin1( "kopete_" ) + protocol.toLower().remove( QString::fromLatin1( "protocol" ) );

		if ( cg.readEntry( "Enabled", true ) && pluginConfig.readEntry(protocol + QLatin1String("Enabled"), true) )
			PluginManager::self()->loadPlugin( protocol, PluginManager::LoadAsync );
	}
}

void AccountManager::slotPluginLoaded( Plugin *plugin )
{
	Protocol* protocol = dynamic_cast<Protocol*>( plugin );
	if ( !protocol )
		return;

	// Iterate over all groups that start with "Account_" as those are accounts
	// and parse them if they are from this protocol
	KSharedConfig::Ptr config = KGlobal::config();
	const QStringList accountGroups = config->groupList().filter( QRegExp( QString::fromLatin1( "^Account_" ) ) );
	for ( QStringList::ConstIterator it = accountGroups.constBegin(); it != accountGroups.constEnd(); ++it )
	{
		KConfigGroup cg( config, *it );

		if ( cg.readEntry( "Protocol" ) != protocol->pluginId() )
			continue;

		// There's no GUI for this, but developers may want to disable an account.
		if ( !cg.readEntry( "Enabled", true ) )
			continue;

		QString accountId = cg.readEntry( "AccountId", QString() );
		if ( accountId.isEmpty() )
		{
			kWarning( 14010 ) <<
				"Not creating account for empty accountId." << endl;
			continue;
		}

		kDebug( 14010 ) <<
			"Creating account for '" << accountId << "'" << endl;

		Account *account = 0L;
		account = registerAccount( protocol->createNewAccount( accountId ) );
		if ( !account )
		{
			kWarning( 14010 ) <<
				"Failed to create account for '" << accountId << "'" << endl;
			continue;
		}
	}
}

void AccountManager::slotAccountOnlineStatusChanged(Contact *c,
	const OnlineStatus &oldStatus, const OnlineStatus &newStatus)
{
	Account *account = c->account();
	if (!account)
		return;

	//kDebug(14010) ;
	emit accountOnlineStatusChanged(account, oldStatus, newStatus);
}

void AccountManager::networkConnected()
{
	if( !resume() )
		setOnlineStatus( Kopete::StatusManager::self()->globalStatusCategory(), Kopete::StatusManager::self()->globalStatusMessage(), 0, true);
}


void AccountManager::networkDisconnected()
{
	suspend();
}

void AccountManager::removeAccountConnectedChanged()
{
	Account *account = qobject_cast<Account*>(sender());
	Q_ASSERT( account );

	if ( !account->isConnected() )
	{
		disconnect( account, SIGNAL(isConnectedChanged()), this, SLOT(removeAccountConnectedChanged()) );
		// Use singleShot so we don't delete the account when we use it.
		d->accountsToBeRemoved.append( account );
		QTimer::singleShot( 0, this, SLOT(removeAccountInternal()) );
	}
}

void AccountManager::removeAccountInternal()
{
	if ( d->accountsToBeRemoved.isEmpty() )
		return;

	Account* account = d->accountsToBeRemoved.takeFirst();
	if ( account->isConnected() )
	{
		kWarning( 14010 ) << "Error, trying to remove connected account " << account->accountId();
		return;
	}

	Protocol *protocol = account->protocol();

	KConfigGroup *configgroup = account->configGroup();

	// Clean up the contact list
	const QHash<QString, Kopete::Contact*> contactList = account->contacts();
	QHash<QString, Kopete::Contact*>::ConstIterator it, itEnd = contactList.constEnd();

	for ( it = contactList.constBegin(); it != itEnd; ++it )
	{
		Contact* c = it.value();
		if ( !c )
			continue;

		MetaContact* mc = c->metaContact();
		mc->removeContact( c );
		c->deleteLater();
		if ( mc->contacts().count() == 0 ) //we can delete the metacontact
		{
			//get the first group and it's members
			Group* group = mc->groups().first();
			MetaContact::List groupMembers = group->members();
			ContactList::self()->removeMetaContact( mc );
			if ( groupMembers.count() == 1 && groupMembers.indexOf( mc ) != -1 )
				ContactList::self()->removeGroup( group );
		}
	}

	// Clean up the account list
	d->accounts.removeAll( account );

	// Clean up configuration
	configgroup->deleteGroup();
	configgroup->sync();

	delete account;

	foreach( Account *account , d->accounts )
	{
		if( account->protocol() == protocol )
			return;
	}
	//there is nomore account from the protocol,  we can unload it

	// FIXME: pluginId() should return the internal name and not the class name, so
	//        we can get rid of this hack - Olivier/Martijn
	QString protocolName = protocol->pluginId().remove( QString::fromLatin1( "Protocol" ) ).toLower();

	PluginManager::self()->setPluginEnabled( protocolName, false );
	PluginManager::self()->unloadPlugin( protocolName );
}

} //END namespace Kopete

#include "kopeteaccountmanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-mode csands;
