/*
    kopeteaccountmanager.cpp - Kopete Account Manager

    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

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
#include "kopetepluginmanager.h"

class KopeteAccountPtrList : public QPtrList<KopeteAccount>
{
	protected:
		int compareItems( KopeteAccountPtrList::Item a, KopeteAccountPtrList::Item b )
		{
			uint priority1 = static_cast<KopeteAccount*>(a)->priority();
			uint priority2 = static_cast<KopeteAccount*>(b)->priority();

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
	static KopeteAccountManager *s_manager;

	KopeteAccountPtrList accounts;
};

KopeteAccountManager * KopeteAccountManagerPrivate::s_manager = 0L;

KopeteAccountManager * KopeteAccountManager::manager()
{
	if ( !KopeteAccountManagerPrivate::s_manager )
		KopeteAccountManagerPrivate::s_manager = new KopeteAccountManager;

	return KopeteAccountManagerPrivate::s_manager;
}

KopeteAccountManager::KopeteAccountManager()
: QObject( qApp, "KopeteAccountManager" )
{
	d = new KopeteAccountManagerPrivate;
}

KopeteAccountManager::~KopeteAccountManager()
{
	KopeteAccountManagerPrivate::s_manager = 0L;

	delete d;
}

void KopeteAccountManager::connectAll()
{
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
		it.current()->connect();
}

void KopeteAccountManager::disconnectAll()
{
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
		it.current()->disconnect();
}

void KopeteAccountManager::setAwayAll( const QString &awayReason )
{
	KopeteAway::setGlobalAway( true );

	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->isConnected() && !it.current()->isAway() )
			it.current()->setAway( true, awayReason.isNull() ? KopeteAway::message() : awayReason );
	}
}

void KopeteAccountManager::setAvailableAll()
{
	KopeteAway::setGlobalAway( false );

	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->isConnected() && it.current()->isAway() )
			it.current()->setAway( false );
	}
}

QColor KopeteAccountManager::guessColor( KopeteProtocol *protocol )
{
	// FIXME: Use a different algoritm. It should check if the color is really not
	//        used - Olivier
	int protocolCount = 0;
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
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

void KopeteAccountManager::registerAccount( KopeteAccount *account )
{

	if ( !account || account->accountId().isNull() )
		return;

	// If this account already exists, do nothing
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( ( account->protocol() == it.current()->protocol() ) && ( account->accountId() == it.current()->accountId() ) )
			return;
	}

	d->accounts.append( account );
}

const QPtrList<KopeteAccount>& KopeteAccountManager::accounts() const
{
	return d->accounts;
}

QDict<KopeteAccount> KopeteAccountManager::accounts( const KopeteProtocol *protocol )
{
	QDict<KopeteAccount> dict;
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol() == protocol && !it.current()->accountId().isNull() )
			dict.insert( it.current()->accountId(), it.current() );
	}

	return dict;
}

KopeteAccount * KopeteAccountManager::findAccount( const QString &protocolId, const QString &accountId )
{
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->protocol()->pluginId() == protocolId && it.current()->accountId() == accountId )
			return it.current();
	}
	return 0L;
}

void KopeteAccountManager::removeAccount( KopeteAccount *account )
{
	kdDebug( 14010 ) << k_funcinfo << "Removing account '" << account->accountId() << "' and cleanning up config" << endl;

	KopeteProtocol *protocol = account->protocol();

	KConfig *config = KGlobal::config();
	QString groupName = account->configGroup();

	// Clean up the account list
	d->accounts.remove( account );

	delete account;

	// Clean up configuration
	config->deleteGroup( groupName );
	config->sync();

	if ( KopeteAccountManager::manager()->accounts( protocol ).isEmpty() )
	{
		// FIXME: pluginId() should return the internal name and not the class name, so
		//        we can get rid of this hack - Olivier/Martijn
		QString protocolName = protocol->pluginId().remove( QString::fromLatin1( "Protocol" ) ).lower();

		KopetePluginManager::self()->setPluginEnabled( protocolName, false );
		KopetePluginManager::self()->unloadPlugin( protocolName );
	}
}

void KopeteAccountManager::unregisterAccount( KopeteAccount *account )
{
	kdDebug( 14010 ) << k_funcinfo << "Unregistering account " << account->accountId() << endl;
	d->accounts.remove( account );
	emit accountUnregistered( account );
}

void KopeteAccountManager::save()
{
	kdDebug( 14010 ) << k_funcinfo << endl;
	d->accounts.sort();
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
		it.current()->writeConfig( it.current()->configGroup() );

	KGlobal::config()->sync();
}

void KopeteAccountManager::load()
{
	connect( KopetePluginManager::self(), SIGNAL( pluginLoaded( KopetePlugin * ) ), SLOT( slotPluginLoaded( KopetePlugin * ) ) );

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
			KopetePluginManager::self()->loadPlugin( protocol, KopetePluginManager::LoadAsync );
	}
}

void KopeteAccountManager::slotPluginLoaded( KopetePlugin *plugin )
{
	KopeteProtocol* protocol = dynamic_cast<KopeteProtocol*>( plugin );
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
			kdWarning( 14010 ) << k_funcinfo << "Not creating account for empty accountId." << endl;
			continue;
		}

		kdDebug( 14010 ) << k_funcinfo << "Creating account for '" << accountId << "'" << endl;
		KopeteAccount *account = protocol->createNewAccount( accountId );
		if ( !account )
		{
			kdWarning( 14010 ) << k_funcinfo << "Failed to create account for '" << accountId << "'" << endl;
			continue;
		}
		account->readConfig( *it );
	}
}

void KopeteAccountManager::autoConnect()
{
	for ( QPtrListIterator<KopeteAccount> it( d->accounts ); it.current(); ++it )
	{
		if ( it.current()->autoLogin() )
			it.current()->connect();
	}
}

void KopeteAccountManager::notifyAccountReady( KopeteAccount *account )
{
	kdDebug() << k_funcinfo << account->accountId() << endl;
	emit accountReady( account );
	d->accounts.sort();
}

#include "kopeteaccountmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

