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
#include <qfile.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>

#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"
#include "kopeteaccount.h"

KopeteAccountManager* KopeteAccountManager::s_manager = 0L;

KopeteAccountManager* KopeteAccountManager::manager()
{
	if( !s_manager )
			s_manager = new KopeteAccountManager;

	return s_manager;
}

KopeteAccountManager::KopeteAccountManager()
: QObject( qApp, "KopeteAccountManager" )
{
}

KopeteAccountManager::~KopeteAccountManager()
{
	s_manager = 0L;
}


void KopeteAccountManager::connectAll()
{
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		account->connect();
	}
}

void KopeteAccountManager::disconnectAll()
{
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		account->disconnect();
	}
}

void KopeteAccountManager::setAwayAll( const QString &awayReason )
{
	KopeteAway::setGlobalAway( true );

	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( account->isConnected() && !account->isAway() )
			account->setAway( true, awayReason.isNull() ? KopeteAway::message() : awayReason );
	}
}

void KopeteAccountManager::setAvailableAll()
{
	KopeteAway::setGlobalAway( false );

	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( account->isConnected() && account->isAway() )
			account->setAway( false );
	}
}

QColor KopeteAccountManager::guessColor( KopeteProtocol *protocol )
{
	// FIXME: Use a different algoritm. It should check if the color is really not
	//        used - Olivier

	int thisProtocolCounter = 0;
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( account->protocol()->pluginId() == protocol->pluginId() )
			thisProtocolCounter++;
	}

	// let's figure a color
	QColor color;
	switch ( thisProtocolCounter % 7 )
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

void KopeteAccountManager::registerAccount(KopeteAccount *i)
{

	if ( !i || i->accountId().isNull() )
		return;

	// If this account already exists, do nothing
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if ( ( i->protocol() == account->protocol() ) && ( i->accountId() == account->accountId() ) )
			return;
	}

	m_accounts.append( i );
}

const QPtrList<KopeteAccount>& KopeteAccountManager::accounts() const
{
	return m_accounts;
}

QDict<KopeteAccount> KopeteAccountManager::accounts(const KopeteProtocol *p)
{
	QDict<KopeteAccount> dict;
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( ( account->protocol() == p ) && !account->accountId().isNull() )
			dict.insert( account->accountId(), account );
	}
	return dict;
}


KopeteAccount* KopeteAccountManager::findAccount(const QString& protocolId, const QString& accountId)
{
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( account->protocol()->pluginId() == protocolId && account->accountId() == accountId )
			return account;
	}
	return 0L;
}

void KopeteAccountManager::removeAccount( KopeteAccount *account )
{
	kdDebug(14010) << k_funcinfo << "Removing account and cleanning up config" << account->accountId() << endl;

	KConfig *config = KGlobal::config();
	QString groupName = account->configGroup();

	delete account;

	// Clean up configuration
	config->deleteGroup( groupName );
	config->sync();
}

void KopeteAccountManager::unregisterAccount( KopeteAccount *account )
{
	kdDebug(14010) << k_funcinfo << "Unregistering account " << account->accountId() << endl;
	m_accounts.remove( account );
	emit accountUnregistered( account );
}

void KopeteAccountManager::save()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	KConfig *config = KGlobal::config();
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		account->writeConfig( account->configGroup() );
	}

	config->sync();
}

void KopeteAccountManager::load()
{
	connect( KopetePluginManager::self(), SIGNAL( pluginLoaded( KopetePlugin* ) ), SLOT( slotPluginLoaded( KopetePlugin * ) ) );
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

		if ( config->readEntry("Protocol") != protocol->pluginId() )
			continue;

		QString accountId = config->readEntry( "AccountId" );
		if ( accountId.isEmpty() )
		{
			kdDebug(14010) << k_funcinfo << "NOT creating account for empty accountId." << endl;
			continue; //return;
		}

		kdDebug(14010) << k_funcinfo << "Creating account for '" << accountId << "'" << endl;

		KopeteAccount *account = protocol->createNewAccount( accountId );
		if (!account)
		{
			kdDebug(14010) << k_funcinfo << "FAILED creating account for " << accountId << endl;
			continue; //return;
		}
		account->readConfig( *it );
	}
}

void KopeteAccountManager::autoConnect()
{
	QPtrListIterator<KopeteAccount> it( m_accounts );
	KopeteAccount *account;
	while ( ( account = it.current() ) != 0 )
	{
		++it;
		if( account->autoLogin() )
			account->connect();
	}
}

/**
 * Called to cause KopeteAccountManager to inform others that
 * an account is fully created and ready for use
 */
void KopeteAccountManager::notifyAccountReady( KopeteAccount *account )
{
	emit accountReady( account );
}

void KopeteAccountManager::moveAccount(KopeteAccount *account , KopeteAccountManager::moveDirrection dirrection)
{
	int index=m_accounts.findRef(account);

	m_accounts.take(index);
	switch(dirrection)
	{
		case Up:
			index--;
			break;
		case Down:
			index++;
			break;
	}

	if(index<0) index=0;

	m_accounts.insert(index, account);

}

#include "kopeteaccountmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

