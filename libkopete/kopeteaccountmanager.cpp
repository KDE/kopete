/*
    kopeteaccountmanager.cpp - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

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
#include "kopetepluginmanager.h"

class KopeteAccountPtrList : public QPtrList<Kopete::Account>
{
	protected:
		int compareItems( KopeteAccountPtrList::Item a, KopeteAccountPtrList::Item b )
		{
			uint priority1 = static_cast<Kopete::Account*>(a)->priority();
			uint priority2 = static_cast<Kopete::Account*>(b)->priority();

			if( priority1 == priority2 )
				return 0;
			else if( priority1 > priority2 )
				return 1;
			else
				return -1;
		}
};

class KopeteAccountManagerPrivate
{
public:
	static Kopete::AccountManager *s_manager;

	KopeteAccountPtrList accounts;
};

Kopete::AccountManager * KopeteAccountManagerPrivate::s_manager = 0L;

Kopete::AccountManager * Kopete::AccountManager::self()
{
	if ( !KopeteAccountManagerPrivate::s_manager )
		KopeteAccountManagerPrivate::s_manager = new Kopete::AccountManager;

	return KopeteAccountManagerPrivate::s_manager;
}

Kopete::AccountManager::AccountManager()
: QObject( qApp, "Kopete::AccountManager" )
{
	d = new KopeteAccountManagerPrivate;
}

Kopete::AccountManager::~AccountManager()
{
	KopeteAccountManagerPrivate::s_manager = 0L;

	delete d;
}

void Kopete::AccountManager::connectAll()
{
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
		it.current()->connect();
}

void Kopete::AccountManager::disconnectAll()
{
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
		it.current()->disconnect();
}

void Kopete::AccountManager::setAwayAll( const QString &awayReason )
{
	Kopete::Away::setGlobalAway( true );

	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		// FIXME: ICQ's invisible online should be set to invisible away
		Kopete::Contact *self = it.current()->myself();
		bool isInvisible = self && self->onlineStatus().status() == Kopete::OnlineStatus::Invisible;
		if ( it.current()->isConnected() && !isInvisible )
			it.current()->setAway( true, awayReason.isNull() ? Kopete::Away::message() : awayReason );
	}
}

void Kopete::AccountManager::setAvailableAll()
{
	Kopete::Away::setGlobalAway( false );

	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->isConnected() && it.current()->isAway() )
			it.current()->setAway( false );
	}
}

QColor Kopete::AccountManager::guessColor( Kopete::Protocol *protocol )
{
	// FIXME: Use a different algoritm. It should check if the color is really not
	//        used - Olivier
	int protocolCount = 0;
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
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

bool Kopete::AccountManager::registerAccount( Kopete::Account *account )
{

	if ( !account || account->accountId().isNull() )
		return false;

	// If this account already exists, do nothing
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		if ( ( account->protocol() == it.current()->protocol() ) && ( account->accountId() == it.current()->accountId() ) )
			return false;
	}

	d->accounts.append( account );
	return true;
}

const QPtrList<Kopete::Account>& Kopete::AccountManager::accounts() const
{
	return d->accounts;
}

QDict<Kopete::Account> Kopete::AccountManager::accounts( const Kopete::Protocol *protocol )
{
	QDict<Kopete::Account> dict;
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol() == protocol && !it.current()->accountId().isNull() )
			dict.insert( it.current()->accountId(), it.current() );
	}

	return dict;
}

Kopete::Account * Kopete::AccountManager::findAccount( const QString &protocolId, const QString &accountId )
{
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol()->pluginId() == protocolId && it.current()->accountId() == accountId )
			return it.current();
	}
	return 0L;
}

void Kopete::AccountManager::removeAccount( Kopete::Account *account )
{
	kdDebug( 14010 ) << k_funcinfo << "Removing account '" <<
		account->accountId() << "' and cleaning up config" << endl;

	Kopete::Protocol *protocol = account->protocol();

	KConfig *config = KGlobal::config();
	QString groupName = account->configGroup();

	// Clean up the account list
	d->accounts.remove( account );

	delete account;

	// Clean up configuration
	config->deleteGroup( groupName );
	config->sync();

	if ( Kopete::AccountManager::self()->accounts( protocol ).isEmpty() )
	{
		// FIXME: pluginId() should return the internal name and not the class name, so
		//        we can get rid of this hack - Olivier/Martijn
		QString protocolName = protocol->pluginId().remove( QString::fromLatin1( "Protocol" ) ).lower();

		Kopete::PluginManager::self()->setPluginEnabled( protocolName, false );
		Kopete::PluginManager::self()->unloadPlugin( protocolName );
	}
}

void Kopete::AccountManager::unregisterAccount( Kopete::Account *account )
{
	/*kdDebug( 14010 ) << k_funcinfo << "Unregistering account " <<
		account->accountId() << endl;*/
	d->accounts.remove( account );
	emit accountUnregistered( account );
}

void Kopete::AccountManager::save()
{
	//kdDebug( 14010 ) << k_funcinfo << endl;
	d->accounts.sort();
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
		it.current()->writeConfig( it.current()->configGroup() );

	KGlobal::config()->sync();
}

void Kopete::AccountManager::load()
{
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin * ) ),
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
			Kopete::PluginManager::self()->loadPlugin( protocol, Kopete::PluginManager::LoadAsync );
	}
}

void Kopete::AccountManager::slotPluginLoaded( Kopete::Plugin *plugin )
{
	Kopete::Protocol* protocol = dynamic_cast<Kopete::Protocol*>( plugin );
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
		
		Kopete::Account *account = 0L;
		account = protocol->createNewAccount( accountId );
		if ( !account )
		{
			kdWarning( 14010 ) << k_funcinfo <<
				"Failed to create account for '" << accountId << "'" << endl;
			continue;
		}
		account->readConfig( *it );
	}
}

void Kopete::AccountManager::autoConnect()
{
	for ( QPtrListIterator<Kopete::Account> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->autoConnect() )
			it.current()->connect();
	}
}

void Kopete::AccountManager::notifyAccountReady( Kopete::Account *account )
{
	//kdDebug(14010) << k_funcinfo << account->accountId() << endl;
	emit accountReady( account );
	d->accounts.sort();

	// Connect to the account's status changed signal
	connect(account->myself(), SIGNAL(onlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
		this, SLOT(slotAccountOnlineStatusChanged(Kopete::Contact *,
			const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)));
}

void Kopete::AccountManager::slotAccountOnlineStatusChanged(Kopete::Contact *c,
	const Kopete::OnlineStatus &oldStatus, const Kopete::OnlineStatus &newStatus)
{
	Kopete::Account *account = c->account();
	if (!account)
		return;

	//kdDebug(14010) << k_funcinfo << endl;
	emit accountOnlineStatusChanged(account, oldStatus, newStatus);
}

#include "kopeteaccountmanager.moc"
// vim: set noet ts=4 sts=4 sw=4:
