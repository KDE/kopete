/*
	kopeteaccountmanager.cpp - Kopete Identity Manager

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

KopeteIdentityManager* KopeteIdentityManager::s_manager = 0L;

KopeteIdentityManager* KopeteIdentityManager::manager()
{
		if( !s_manager )
				s_manager = new KopeteIdentityManager;

		return s_manager;
}

KopeteIdentityManager::KopeteIdentityManager()
				: QObject( qApp, "KopeteIdentityManager" )
{
}

KopeteIdentityManager::~KopeteIdentityManager()
{
		s_manager = 0L;
}


void KopeteIdentityManager::connectAll()
{
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
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
						kdDebug(14010) << "KopeteIdentityManager::connectAll: "
													 << "Connecting plugin: " << proto->pluginId() << endl;

						proto->connect();
				}
		}
}

void KopeteIdentityManager::disconnectAll()
{
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
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
						kdDebug(14010) << "KopeteIdentityManager::disconnectAll: "
													 << "Disonnecting plugin: " << proto->pluginId() << endl;

						proto->disconnect();
				}
		}
}

void KopeteIdentityManager::setAwayAll()
{
	KopeteAway::setGlobalAway( true );
	
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
	{
		if(i->isConnected() && !i->isAway())
			i->setAway(true);
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
						kdDebug(14010) << "KopeteIdentityManager::setAwayAll: "
													 << "Setting plugin to away: " << proto->pluginId() << endl;

						proto->setAway();
				}
		}
}

void KopeteIdentityManager::setAvailableAll()
{
	KopeteAway::setGlobalAway( false );
	
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
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
						kdDebug(14010) << "KopeteIdentityManager::setAvailableAll: "
													 << "Setting plugin to available: " << proto->pluginId() << endl;

						proto->setAvailable();
				}
		}
}

void KopeteIdentityManager::registerIdentity(KopeteIdentity *i)
{
	m_identities.append( i );
	QObject::connect( i, SIGNAL( identityDestroyed( KopeteIdentity * ) ), this , SLOT( slotIdentityDestroyed( KopeteIdentity * ) ) );
}

const QPtrList<KopeteIdentity>& KopeteIdentityManager::identities() const
{
	return m_identities;
}

QDict<KopeteIdentity> KopeteIdentityManager::identities(const KopeteProtocol *p)
{
	QDict<KopeteIdentity> dict;
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
	{
		if(i->protocol() == p)
			dict.insert(i->identityId() , i);
	}
	return dict;
}


KopeteIdentity* KopeteIdentityManager::findIdentity(const QString& protocolId, const QString& identityId)
{
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
	{
		if(QString::fromLatin1(i->protocol()->pluginId()) == protocolId && i->identityId() == identityId)
			return i;
	}
	return 0L;
}



void KopeteIdentityManager::slotIdentityDestroyed(KopeteIdentity* i)
{
	m_identities.remove(i);
}



void KopeteIdentityManager::save()
{
	QString fileName = locateLocal( "appdata", QString::fromLatin1( "identities.xml" ) );

	KSaveFile file( fileName );
	if( file.status() == 0 )
	{
		QTextStream *stream = file.textStream();
		stream->setEncoding( QTextStream::UnicodeUTF8 );

		QString xml = QString::fromLatin1("<?xml version=\"1.0\"?>\n"
			"<!DOCTYPE kopete-identities>\n"
		"<kopete-identities version=\"1.0\">\n" );

		for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
		{
			xml +=  i->toXML();
		}

		xml += QString::fromLatin1( "</kopete-identities>\n" );

		*stream << xml;
		if ( !file.close() )
		{
			kdDebug(14010) << "KopeteIdentityManager::save: ERROR: failed to write identities, error code is: " << file.status() << endl;
		}
	}
	else
	{
		kdWarning(14010) << "KopeteIdentityManager::save: ERROR: Couldn't open identities file " << fileName << " identities not saved." << endl;
	}
}

void KopeteIdentityManager::load()
{
	QString filename = locateLocal( "appdata", QString::fromLatin1( "identities.xml" ) );
	if( filename.isEmpty() )
		return ;
		
	kdDebug(14010) << k_funcinfo <<endl;

	m_identityList = QDomDocument( QString::fromLatin1( "kopete-identities" ) );

	QFile file( filename );
	file.open( IO_ReadOnly );
	m_identityList.setContent( &file );

	file.close();
	
	connect( LibraryLoader::pluginLoader(), SIGNAL( pluginLoaded(KopetePlugin*) ),
		this, SLOT( loadProtocol(KopetePlugin*) ) );

}

void KopeteIdentityManager::loadProtocol( KopetePlugin *plu )
{
	KopeteProtocol* protocol=dynamic_cast<KopeteProtocol*>(plu);
	if(!protocol)
		return;

	kdDebug(14010) << k_funcinfo <<endl;

	QDomNode node = m_identityList.documentElement().firstChild();
	while( !node.isNull() )
	{
		QDomElement element = node.toElement();
		if( !element.isNull() )
		{
			if( element.tagName() == QString::fromLatin1("identity") )
			{
				QString identityId = element.attribute( QString::fromLatin1("identity-id"), QString::null );
				QString protocolId = element.attribute( QString::fromLatin1("protocol-id"), QString::null );

				if( protocolId == QString::fromLatin1(protocol->pluginId()) )
				{
					KopeteIdentity *identity = protocol->createNewIdentity(identityId);
					QDomNode identityNode = node.firstChild();
					if (identity && !identity->fromXML( identityNode ) )
					{
						delete identity;
						identity = 0L;
					}
				}
			}
			else
			{
				kdWarning(14010) << k_funcinfo << "Unknown element '" << element.tagName()
					  << "' in contact list!" << endl;
			}
		}
		node = node.nextSibling();
	}
}

void KopeteIdentityManager::autoConnect()
{
	for(KopeteIdentity *i=m_identities.first() ; i; i=m_identities.next() )
	{
		if(i->autoLogin())
			i->connect();
	}
}

#include "kopeteaccountmanager.moc"

// vim: set noet ts=4 sts=4 sw=4:

