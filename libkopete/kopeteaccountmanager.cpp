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

#include "kopeteaway.h"
#include "kopeteprotocol.h"
#include "pluginloader.h"
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
	//When a status is changed, the protocol loops into the account to update the menu icon.
	//and then it change the m_accounts.current().. so we make a copy
	QPtrList<KopeteAccount>accounts = m_accounts;

	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
		i->connect();
}

void KopeteAccountManager::disconnectAll()
{
	//When a status is changed, the protocol loops into the account to update the menu icon.
	//and then it change the m_accounts.current().. so we make a copy
	QPtrList<KopeteAccount>accounts = m_accounts;
	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
		i->disconnect();
}

void KopeteAccountManager::setAwayAll( const QString &awayReason )
{
	KopeteAway::setGlobalAway( true );
	//When a status is changed, the protocol loops into the account to update the menu icon.
	//and then it change the m_accounts.current().. so we make a copy
	QPtrList<KopeteAccount>accounts = m_accounts;
	for(KopeteAccount *i = accounts.first() ; i; i = accounts.next() )
	{
		if(i->isConnected() && !i->isAway())
		{
			if( !awayReason.isNull() )
				i->setAway(true, awayReason);
			else
				i->setAway(true, KopeteAway::message() );
		}
	}
}

void KopeteAccountManager::setAvailableAll()
{
	KopeteAway::setGlobalAway( false );
	//When a status is changed, the protocol loops into the account to update the menu icon.
	//and then it change the m_accounts.current().. so we make a copy
	QPtrList<KopeteAccount>accounts = m_accounts;
	for(KopeteAccount *i=accounts.first() ; i; i=accounts.next() )
	{
		if(i->isConnected() && i->isAway())
			i->setAway(false);
	}
}

QColor KopeteAccountManager::guessColor( KopeteProtocol* protocol )
{
	//TODO: use a different algoritm. It should really check if the color is really not used

	/* counter for accounts of this protocol */
	int thisProtocolCounter = 0;

	for ( KopeteAccount *acc = m_accounts.first() ; acc ; acc = m_accounts.next() )
	{
		if(  acc->protocol()->pluginId() == protocol->pluginId() )
		{
			thisProtocolCounter++;
		}
	}

	QColor color;
	/* lets figure a color */
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

	/* Anti-Crash: Valid account pointer? */
	if ( !i )
		return;

	/* No, we don't allow accounts without id */
	if ( !(i->accountId()).isNull() )
	{
		/* Lets check if account exists already in protocol namespace */
		for ( KopeteAccount *acc = m_accounts.first() ; acc ; acc = m_accounts.next() )
		{
			if(  (i->protocol() == acc->protocol()) && ( i->accountId() == acc->accountId() ) )
			{
					/* Duplicate!! */
					return;
			}
		}

		m_accounts.append( i );
	}

}

const QPtrList<KopeteAccount>& KopeteAccountManager::accounts() const
{
	return m_accounts;
}

QDict<KopeteAccount> KopeteAccountManager::accounts(const KopeteProtocol *p)
{
	QDict<KopeteAccount> dict;
	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		if( (i->protocol() == p) && !(i->accountId().isNull()) )
			dict.insert(i->accountId() , i);
	}
	return dict;
}


KopeteAccount* KopeteAccountManager::findAccount(const QString& protocolId, const QString& accountId)
{
	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		if( i->protocol()->pluginId() == protocolId && i->accountId() == accountId )
			return i;
	}
	return 0L;
}

void KopeteAccountManager::removeAccount( KopeteAccount *account )
{
	kdDebug(14010) << k_funcinfo << "Removing account and cleanning up config" << account->accountId() << endl;

	KConfig *config = KGlobal::config();
	QString groupName = QString::fromLatin1( "Account_%2_%1" ).arg( account->accountId() ).arg( account->protocol()->pluginId() );

	delete account;
	/* Clean up configuration */
	config->deleteGroup(groupName);
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
	for ( KopeteAccount *account = m_accounts.first() ; account; account = m_accounts.next() )
	{
		QString groupName = QString::fromLatin1( "Account_%2_%1" ).arg( account->accountId() ).arg( account->protocol()->pluginId() );
		config->setGroup( groupName );

		account->writeConfig( groupName );
	}

	config->sync();
}

void KopeteAccountManager::load()
{
	kdDebug( 14010 ) << k_funcinfo << endl;

	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded( KopetePlugin* ) ), SLOT( slotPluginLoaded( KopetePlugin * ) ) );
}

void KopeteAccountManager::slotPluginLoaded( KopetePlugin *plugin )
{
	kdDebug(14010) << k_funcinfo << "Called." << endl;

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
	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		if(i->autoLogin())
			i->connect();
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

#include "kopeteaccountmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

