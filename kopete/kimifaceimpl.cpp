/*
    kimifaceimpl.cpp - Kopete DCOP Interface

    Copyright (c) 2004 by Will Stephenson     <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qstringlist.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kplugininfo.h>

#include "kopeteaccount.h"
#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetepluginmanager.h"

#include "kimifaceimpl.h"

KIMIfaceImpl::KIMIfaceImpl() : QObject(), DCOPObject( "KIMIface" )
{
	connect( KopeteContactList::contactList(),
		SIGNAL( metaContactAdded( KopeteMetaContact * ) ),
		SLOT( slotMetaContactAdded( KopeteMetaContact * ) ) );
}

KIMIfaceImpl::~KIMIfaceImpl()
{
}

QStringList KIMIfaceImpl::allContacts()
{
	QStringList result;
	QPtrList<KopeteMetaContact> list = KopeteContactList::contactList()->metaContacts();
	QPtrListIterator<KopeteMetaContact> it( list );
	for( ; it.current(); ++it )
	{
		if ( !it.current()->metaContactId().isNull() )
			result.append( it.current()->metaContactId() );
	}

	return result;
}

QStringList KIMIfaceImpl::reachableContacts()
{
	KopeteAccountManager::manager()->connectAll();
	QStringList result;
	QPtrList<KopeteMetaContact> list = KopeteContactList::contactList()->metaContacts();
	QPtrListIterator<KopeteMetaContact> it( list );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isReachable() && !it.current()->metaContactId().isNull() )
			result.append( it.current()->metaContactId() );
	}

	return result;
}

QStringList KIMIfaceImpl::onlineContacts()
{
	KopeteAccountManager::manager()->connectAll();
	QStringList result;
	QPtrList<KopeteMetaContact> list = KopeteContactList::contactList()->metaContacts();
	QPtrListIterator<KopeteMetaContact> it( list );
	for( ; it.current(); ++it )
	{
		if ( it.current()->isOnline() && !it.current()->metaContactId().isNull() )
			result.append( it.current()->metaContactId() );
	}

	return result;
}

QStringList KIMIfaceImpl::fileTransferContacts()
{
	KopeteAccountManager::manager()->connectAll();
	QStringList result;
	QPtrList<KopeteMetaContact> list = KopeteContactList::contactList()->metaContacts();
	QPtrListIterator<KopeteMetaContact> it( list );
	for( ; it.current(); ++it )
	{
		if ( it.current()->canAcceptFiles() && !it.current()->metaContactId().isNull() )
			result.append( it.current()->metaContactId() );
	}

	return result;
}

bool KIMIfaceImpl::isPresent( const QString & uid )
{
	KopeteMetaContact *mc;
	mc = KopeteContactList::contactList()->metaContact( uid );
	
	return ( mc != 0 );
}

int KIMIfaceImpl::presenceStatus( const QString & uid )
{
	KopeteAccountManager::manager()->connectAll();
	int p;
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
	{
		KopeteOnlineStatus status = m->status();
		switch ( status.status() )
		{
			case KopeteOnlineStatus::Unknown:
				p = 0;	
			break;
			case KopeteOnlineStatus::Offline:
				p = 1;
			break;
			case KopeteOnlineStatus::Connecting:
				p = 2;
			break;
			case KopeteOnlineStatus::Away:
				p = 3;
			break;
			case KopeteOnlineStatus::Online:
				p = 4;
			break;
		}
	}
	else
	{
		p = -1;
	}
	return p;
}

QString KIMIfaceImpl::presenceString( const QString & uid )
{
	KopeteAccountManager::manager()->connectAll();
	QString p;
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
	{
		KopeteOnlineStatus status = m->status();
			p = status.description();
	}
	else
	{
		p = i18n("Error: could not discover presence.");
	}
	return p;
}

bool KIMIfaceImpl::canReceiveFiles( const QString & uid )
{
	KopeteMetaContact *mc;
	mc = KopeteContactList::contactList()->metaContact( uid );
	
	if ( mc )
		return mc->canAcceptFiles();
	else 
		return false;
}

bool KIMIfaceImpl::canRespond( const QString & uid )
{
	// FIXME: rework this when there is proper api support for detecting bidirectional protocols
	KopeteMetaContact *mc;
	mc = KopeteContactList::contactList()->metaContact( uid );
	
	if ( mc )
	{
		QPtrList<KopeteContact> list = mc->contacts();
		QPtrListIterator<KopeteContact> it( list );
		KopeteContact *contact;
		while ( ( contact = it.current() ) != 0 )
		{
			++it;
			if ( contact->isOnline() && contact->protocol()->pluginId() != "SMSProtocol" )
			return true;
		}
	}
	return false;
}

QString KIMIfaceImpl::locate( const QString & contactId, const QString & protocolId )
{
	KopeteMetaContact *mc = locateProtocolContact( contactId, protocolId );
	if ( mc )
		return mc->metaContactId();
	else
		return QString();
}

KopeteMetaContact * KIMIfaceImpl::locateProtocolContact( const QString & contactId, const QString & protocolId )
{
	KopeteMetaContact *mc;
	// find a matching protocol
	KopeteProtocol *protocol = dynamic_cast<KopeteProtocol*>( KopetePluginManager::self()->plugin( protocolId ) );
	
	if ( protocol )
	{
		// find its accounts
		QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( protocol );
		QDictIterator<KopeteAccount> it( accounts );
		for( ; it.current(); ++it )
			mc = KopeteContactList::contactList()->findContact( protocolId, it.currentKey(), contactId  );
	}
	return mc;
}

QPixmap KIMIfaceImpl::icon( const QString & uid )
{
	KopeteAccountManager::manager()->connectAll();
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	QPixmap p;
	if ( m )
		p = SmallIcon( m->statusIcon() );
	return p;
}

QString KIMIfaceImpl::context( const QString & uid )
{
	// TODO: support context
	// shush warning
	QString myUid = uid;
	
	return QString( "Home" );
}

QStringList KIMIfaceImpl::protocols()
{
	QValueList<KPluginInfo *> protocols = KopetePluginManager::self()->availablePlugins( "Protocols" );
	QStringList protocolList;
	for ( QValueList<KPluginInfo *>::Iterator it = protocols.begin(); it != protocols.end(); ++it )
		protocolList.append( (*it)->name() );
	
	return protocolList;
}

void KIMIfaceImpl::messageContact( const QString &uid, const QString& messageText )
{
	KopeteAccountManager::manager()->connectAll();
	// TODO: make it possible to specify the message here
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->sendMessage();
}

void KIMIfaceImpl::messageNewContact( const QString &protocol, const QString &contactId )
{
	KopeteAccountManager::manager()->connectAll();
	KopeteMetaContact *mc = locateProtocolContact( contactId, protocol );
	if ( mc )
		mc->sendMessage();
}

void KIMIfaceImpl::chatWithContact( const QString &uid )
{
	KopeteAccountManager::manager()->connectAll();
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->execute();
}

void KIMIfaceImpl::sendFile(const QString &uid, const KURL &sourceURL,
		const QString &altFileName, uint fileSize)
{
	KopeteAccountManager::manager()->connectAll();
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->sendFile( sourceURL, altFileName, fileSize );
}

bool KIMIfaceImpl::addContact( const QString &protocolId, const QString &contactId )
{
	KopeteAccountManager::manager()->connectAll();
	// find a matching protocol
	KopeteProtocol *protocol = dynamic_cast<KopeteProtocol*>( KopetePluginManager::self()->plugin( protocolId ) );
	
	if ( protocol )
	{
		// find its accounts
		QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( protocol );
		QDictIterator<KopeteAccount> it( accounts );
		KopeteAccount *ac = it.toFirst();
		if ( ac )
		{
			ac->addContact( contactId );
			return true;
		}
	}
	return false;
}

void KIMIfaceImpl::slotMetaContactAdded( KopeteMetaContact *mc )
{
	connect( mc, SIGNAL( onlineStatusChanged( KopeteMetaContact *, KopeteOnlineStatus::OnlineStatus ) ),
		SLOT( slotContactStatusChanged( KopeteMetaContact * ) ) );
}

void KIMIfaceImpl::slotContactStatusChanged( KopeteMetaContact *mc )
{
	if ( !mc->metaContactId().isNull() )
	{
		// tell anyone who's listening over DCOP
		QByteArray params;
		QDataStream stream(params, IO_WriteOnly);
		stream << mc->metaContactId();
		kapp->dcopClient()->emitDCOPSignal( "contactStatusChanged(QString)", params ); 
	}
}
