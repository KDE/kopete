/*
	kopeteaccountmanager.cpp - Kopete Account Manager

	Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
	Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

	Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/

#include "kopeteaccountmanager.h"

#include <qapplication.h>
#include <qstylesheet.h>
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <ksavefile.h>
#include <kstandarddirs.h>

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
	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		i->connect();
	}

	//------ OBSOLETE
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
			KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
			if( !proto )
					continue;

			if( !proto->isConnected() )
			{
					kdDebug(14010) << "KopeteAccountManager::connectAll: "
												 << "Connecting plugin: " << proto->pluginId() << endl;

					proto->connect();
			}
	}
}

void KopeteAccountManager::disconnectAll()
{
	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		i->disconnect();
	}

	//------ OBSOLETE
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
			KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
			if( !proto )
					continue;

			if( proto->isConnected() )
			{
					kdDebug(14010) << "KopeteAccountManager::disconnectAll: "
												 << "Disonnecting plugin: " << proto->pluginId() << endl;

					proto->disconnect();
			}
	}
}

void KopeteAccountManager::setAwayAll( const QString &awayReason )
{
	KopeteAway::setGlobalAway( true );

	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		if(i->isConnected() && !i->isAway())
			i->setAway(true, awayReason);
	}

	//------ OBSOLETE
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
		KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
		if( !proto )
				continue;

		if( proto->isConnected() && !proto->isAway() )
		{
				kdDebug(14010) << "KopeteAccountManager::setAwayAll: "
											 << "Setting plugin to away: " << proto->pluginId() << endl;

				proto->setAway();
		}
	}
}

void KopeteAccountManager::setAvailableAll()
{
	KopeteAway::setGlobalAway( false );

	for(KopeteAccount *i=m_accounts.first() ; i; i=m_accounts.next() )
	{
		if(i->isConnected() && i->isAway())
			i->setAway(false);
	}

	//------ OBSOLETE
	QPtrList<KopetePlugin> plugins = LibraryLoader::pluginLoader()->plugins();
	for( KopetePlugin *p = plugins.first() ; p ; p = plugins.next() )
	{
			KopeteProtocol *proto = dynamic_cast<KopeteProtocol*>( p );
			if( !proto )
					continue;

			if( proto->isConnected() && proto->isAway() )
			{
					kdDebug(14010) << "KopeteAccountManager::setAvailableAll: "
												 << "Setting plugin to available: " << proto->pluginId() << endl;

					proto->setAvailable();
			}
	}
}

void KopeteAccountManager::registerAccount(KopeteAccount *i)
{
	/* Anti-Crash: Valid account pointer? */
	if ( !i )
		return;

	/* No, we dont allow accounts without id */
	if ( !(i->accountId()).isNull() )
	{
		/* Lets check if account exists already in protocol namespace */
		for ( KopeteAccount *acc = m_accounts.first() ; acc ; acc = m_accounts.next() )
		{
			if( ( i->protocol() == acc->protocol() ) && ( i->accountId() == acc->accountId() ) )
			{
				/* Duplicate!! */
				return;
			}
		}
		/* Ok seems sane */
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

void KopeteAccountManager::unregisterAccount( KopeteAccount *account )
{
	m_accounts.remove( account );
}

void KopeteAccountManager::save()
{
	QDomDocument accounts;
	QString fileName = locateLocal( "appdata", QString::fromLatin1( "accounts.xml" ) );

	KSaveFile file( fileName );
	if( file.status() == 0 )
	{
		accounts.appendChild( accounts.createElement( QString::fromLatin1("kopete-accounts") ) );
		accounts.documentElement().setAttribute( QString::fromLatin1("version"), QString::fromLatin1("1.0") );

		QTextStream *stream = file.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );

		for(KopeteAccount *i = m_accounts.first() ; i; i = m_accounts.next() )
			accounts.documentElement().appendChild( accounts.importNode( i->toXML(), true ) );

		accounts.save( *stream, 4 );
		if ( !file.close() )
		{
			kdDebug(14010) << "KopeteAccountManager::save: ERROR: failed to write accounts, error code is: " << file.status() << endl;
		}
	}
	else
	{
		kdWarning(14010) << "KopeteAccountManager::save: ERROR: Couldn't open accounts file " << fileName << " accounts not saved." << endl;
	}
}

void KopeteAccountManager::load()
{
	QString filename = locateLocal( "appdata", QString::fromLatin1( "accounts.xml" ) );
	if( filename.isEmpty() )
		return ;

	kdDebug(14010) << k_funcinfo <<endl;

	m_accountList = QDomDocument( QString::fromLatin1( "kopete-accounts" ) );

	QFile file( filename );
	file.open( IO_ReadOnly );
	m_accountList.setContent( &file );

	file.close();

	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
		this, SLOT( loadProtocol(KopetePlugin*) ) );

}

void KopeteAccountManager::loadProtocol( KopetePlugin *plu )
{
	KopeteProtocol* protocol=dynamic_cast<KopeteProtocol*>(plu);
	if(!protocol)
		return;

	kdDebug(14010) << k_funcinfo <<endl;

	QDomElement element = m_accountList.documentElement().firstChild().toElement();
	while( !element.isNull() )
	{
		if( element.tagName() == QString::fromLatin1("account") )
		{
			QString accountId = element.attribute( QString::fromLatin1("account-id"), QString::null );
			QString protocolId = element.attribute( QString::fromLatin1("protocol-id"), QString::null );

			if( (protocolId == protocol->pluginId()) )
			{
				if ( !accountId.isEmpty() )
				{
					KopeteAccount *account = protocol->createNewAccount(accountId);
					if (account && !account->fromXML( element ) )
					{
						delete account;
						account = 0L;
					}
				}
				else
				{
					kdWarning(14010) << k_funcinfo << "Account with emtpy id!" << endl;
				}
			}
		}
		else
		{
			kdWarning(14010) << k_funcinfo << "Unknown element '" << element.tagName()
				<< "' in accounts.xml!" << endl;
		}
		element = element.nextSibling().toElement();
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

#include "kopeteaccountmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

