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

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kplugininfo.h>

#include "kopeteaccount.h"
#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetepluginmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"

namespace Kopete {

class AccountManager::Private
{
public:

	class AccountPtrList : public QPtrList<Account>
	{
		protected:
			int compareItems( AccountPtrList::Item a, AccountPtrList::Item b )
			{
				uint priority1 = static_cast<Account*>(a)->priority();
				uint priority2 = static_cast<Account*>(b)->priority();

				if( a==b ) //two account are equal only if they are equal :-)
					return 0;  // remember than an account can be only once on the list, but two account may have the same priority when loading
				else if( priority1 > priority2 )
					return 1;
				else
					return -1;
			}
	} accounts;

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
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if(it.current()->isConnected())
			return true;
	}
	return false;
}

void AccountManager::connectAll()
{
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
		if(!it.current()->excludeConnect())
			it.current()->connect();
}

void AccountManager::setAvailableAll( const QString &awayReason )
{
	Away::setGlobalAway( false );
	bool anyConnected = isAnyAccountConnected();

	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if ( anyConnected )
		{
			if ( it.current()->isConnected() )
				it.current()->setAway( false, awayReason );
		}
		else
			if(!it.current()->excludeConnect())
				it.current()->connect();
	}
}

void AccountManager::disconnectAll()
{
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
		it.current()->disconnect();
}

void AccountManager::setAwayAll( const QString &awayReason, bool away )
{
	Away::setGlobalAway( true );
	bool anyConnected = isAnyAccountConnected();

	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		// FIXME: ICQ's invisible online should be set to invisible away
		Contact *self = it.current()->myself();
		bool isInvisible = self && self->onlineStatus().status() == OnlineStatus::Invisible;
		if ( anyConnected )
		{
			if ( it.current()->isConnected() && !isInvisible )
				it.current()->setAway( away, awayReason );
		}
		else
		{
			if ( !it.current()->excludeConnect() && !isInvisible )
				it.current()->setAway( away, awayReason );
		}
	}
}

void AccountManager::setOnlineStatus( uint category , const QString& awayMessage, uint flags )
{
	OnlineStatusManager::Categories katgor=(OnlineStatusManager::Categories)category;
	bool anyConnected = isAnyAccountConnected();

	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		Account *account = it.current();
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

	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol()->pluginId() == protocol->pluginId() )
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
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if ( ( account->protocol() == it.current()->protocol() ) && ( account->accountId() == it.current()->accountId() ) )
		{
			account->deleteLater();
			return 0L;
		}
	}

	d->accounts.append( account );
	d->accounts.sort();

	// Connect to the account's status changed signal
	connect(account->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
		this, SLOT(slotAccountOnlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)));

	connect(account, SIGNAL(accountDestroyed(const Kopete::Account *)) , this, SLOT( unregisterAccount(const Kopete::Account *) ));

	emit accountRegistered( account );
	return account;
}

void AccountManager::unregisterAccount( const Account *account )
{
	kdDebug( 14010 ) << k_funcinfo << "Unregistering account " << account->accountId() << endl;
	d->accounts.remove( account );
	emit accountUnregistered( account );
}

const QPtrList<Account>& AccountManager::accounts() const
{
	return d->accounts;
}

QDict<Account> AccountManager::accounts( const Protocol *protocol ) const
{
	QDict<Account> dict;
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol() == protocol && !it.current()->accountId().isNull() )
			dict.insert( it.current()->accountId(), it.current() );
	}

	return dict;
}

Account * AccountManager::findAccount( const QString &protocolId, const QString &accountId )
{
	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol()->pluginId() == protocolId && it.current()->accountId() == accountId )
			return it.current();
	}
	return 0L;
}

void AccountManager::removeAccount( Account *account )
{
	if(!account->removeAccount())
		return;

	Protocol *protocol = account->protocol();


	KConfigGroup *configgroup = account->configGroup();

	// Clean up the contact list
	QDictIterator<Kopete::Contact> it( account->contacts() );
	for ( ; it.current(); ++it )
	{
		Contact* c = it.current();
		MetaContact* mc = c->metaContact();
		if ( mc == ContactList::self()->myself() )
			continue;
		mc->removeContact( c );
		c->deleteLater();
		if ( mc->contacts().count() == 0 ) //we can delete the metacontact
		{
			//get the first group and it's members
			Group* group = mc->groups().first();
			QPtrList<MetaContact> groupMembers = group->members();
			ContactList::self()->removeMetaContact( mc );
			if ( groupMembers.count() == 1 && groupMembers.findRef( mc ) != -1 )
				ContactList::self()->removeGroup( group );
		}
	}

	// Clean up the account list
	d->accounts.remove( account );

	// Clean up configuration
	configgroup->deleteGroup();
	configgroup->sync();

	delete account;

	if ( accounts( protocol ).isEmpty() )
	{
		// FIXME: pluginId() should return the internal name and not the class name, so
		//        we can get rid of this hack - Olivier/Martijn
		QString protocolName = protocol->pluginId().remove( QString::fromLatin1( "Protocol" ) ).lower();

		PluginManager::self()->setPluginEnabled( protocolName, false );
		PluginManager::self()->unloadPlugin( protocolName );
	}
}

void AccountManager::save()
{
	//kdDebug( 14010 ) << k_funcinfo << endl;
	d->accounts.sort();

	for ( QPtrListIterator<Account> it( d->accounts ); it.current(); ++it )
	{
		KConfigBase *config = it.current()->configGroup();

		config->writeEntry( "Protocol", it.current()->protocol()->pluginId() );
		config->writeEntry( "AccountId", it.current()->accountId() );
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
			protocol = QString::fromLatin1( "kopete_" ) + protocol.lower().remove( QString::fromLatin1( "protocol" ) );

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
