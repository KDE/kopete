/*
    kopeteaccountmanager.cpp - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003-2004 by Olivier Goffart       <ogoffart @ kde.org>

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

#include <qapplication.h>
#include <qregexp.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kplugininfo.h>

#include "kopeteaccount.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "kopetepluginmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"

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
};

AccountManager * AccountManager::s_self = 0L;

AccountManager * AccountManager::self()
{
	if ( !s_self )
		s_self = new AccountManager;

	return s_self;
}


AccountManager::AccountManager()
: QObject( qApp, "KopeteAccountManager" )
{
	d = new Private;
}


AccountManager::~AccountManager()
{
	s_self = 0L;

	delete d;
}

bool AccountManager::isAnyAccountConnected()
{
	bool anyConnected = false;
	foreach( Account *a , d->accounts )
		anyConnected |= a->isConnected();

	return anyConnected;
}

void AccountManager::connectAll()
{
	setOnlineStatus( OnlineStatusManager::Online  );
}

void AccountManager::setAvailableAll( const QString &awayReason )
{
	setOnlineStatus( OnlineStatusManager::Online  , awayReason );
}

void AccountManager::disconnectAll()
{
	setOnlineStatus( OnlineStatusManager::Offline   );
}

void AccountManager::setAwayAll( const QString &awayReason, bool away )
{
	setOnlineStatus( away ? OnlineStatusManager::Away : OnlineStatusManager::Online  , awayReason );
}

void AccountManager::setOnlineStatus( uint category , const QString& awayMessage, uint flags )
{
	OnlineStatusManager::Categories katgor=(OnlineStatusManager::Categories)category;
	bool anyConnected = isAnyAccountConnected();
	
	foreach( Account *account ,  d->accounts )
	{
		Kopete::OnlineStatus status = OnlineStatusManager::self()->onlineStatus(account->protocol() , katgor);
		if ( anyConnected )
		{
			if ( account->isConnected() || ( (flags & ConnectIfOffline) && !account->excludeConnect() ) )
				account->setOnlineStatus( status , awayMessage );
		}
		else
		{
			if ( !account->excludeConnect() )
				account->setOnlineStatus( status , awayMessage );
		}
	}
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

	connect(account, SIGNAL(accountDestroyed(const Kopete::Account *)) , this, SLOT( unregisterAccount(const Kopete::Account *) ));

	emit accountRegistered( account );
	return account;
}

void AccountManager::unregisterAccount( Account *account )
{
	kdDebug( 14010 ) << k_funcinfo << "Unregistering account " << account->accountId() << endl;
	d->accounts.remove( account );
	emit accountUnregistered( account );
}

const QList<Account *>& AccountManager::accounts() const
{
	return d->accounts;
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
	if(!account->removeAccount())
		return;

	Protocol *protocol = account->protocol();

	KConfigGroup *configgroup = account->configGroup();

	// Clean up the account list
	d->accounts.remove( account );

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

void AccountManager::save()
{
	//kdDebug( 14010 ) << k_funcinfo << endl;
	qSort( d->accounts.begin(), d->accounts.end(), compareAccountsByPriority );
	
	for ( QListIterator<Account *> it( d->accounts ); it.hasNext(); )
	{
		Account *a = it.next();
		KConfigBase *config = a->configGroup();
	
		config->writeEntry( "Protocol", a->protocol()->pluginId() );
		config->writeEntry( "AccountId", a->accountId() );
	}

	KGlobal::config()->sync();
}

void AccountManager::load()
{
	connect( PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin * ) ),
		this, SLOT( slotPluginLoaded( Kopete::Plugin * ) ) );

	// Iterate over all groups that start with "Account_" as those are accounts
	// and load the required protocols if the account is enabled.
	// Don't try to optimize duplicate calls out, the plugin queue is smart enough
	// (and fast enough) to handle that without adding complexity here
	KConfig *config = KGlobal::config();
	QStringList accountGroups = config->groupList().grep( QRegExp( QString::fromLatin1( "^Account_" ) ) );
	for ( QStringList::Iterator it = accountGroups.begin(); it != accountGroups.end(); ++it )
	{
		config->setGroup( *it );

		QString protocol = config->readEntry( "Protocol" );
		if ( protocol.endsWith( QString::fromLatin1( "Protocol" ) ) )
			protocol = QString::fromLatin1( "kopete_" ) + protocol.toLower().remove( QString::fromLatin1( "protocol" ) );

		if ( config->readBoolEntry( "Enabled", true ) )
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
	KConfig *config = KGlobal::config();
	QStringList accountGroups = config->groupList().grep( QRegExp( QString::fromLatin1( "^Account_" ) ) );
	for ( QStringList::Iterator it = accountGroups.begin(); it != accountGroups.end(); ++it )
	{
		config->setGroup( *it );

		if ( config->readEntry( "Protocol" ) != protocol->pluginId() )
			continue;

		// There's no GUI for this, but developers may want to disable an account.
		if ( !config->readBoolEntry( "Enabled", true ) )
			continue;

		QString accountId = config->readEntry( "AccountId" );
		if ( accountId.isEmpty() )
		{
			kdWarning( 14010 ) << k_funcinfo <<
				"Not creating account for empty accountId." << endl;
			continue;
		}

		kdDebug( 14010 ) << k_funcinfo <<
			"Creating account for '" << accountId << "'" << endl;
		
		Account *account = 0L;
		account = registerAccount( protocol->createNewAccount( accountId ) );
		if ( !account )
		{
			kdWarning( 14010 ) << k_funcinfo <<
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

	//kdDebug(14010) << k_funcinfo << endl;
	emit accountOnlineStatusChanged(account, oldStatus, newStatus);
}

} //END namespace Kopete

#include "kopeteaccountmanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-mode csands;
