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

#include "kdebug.h"

#include "kimifaceimpl.h"

KIMIfaceImpl::KIMIfaceImpl() : DCOPObject( "KIMIface" ), QObject()
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


QString KIMIfaceImpl::displayName( const QString & uid )
{
	KopeteMetaContact *mc;
	mc = KopeteContactList::contactList()->metaContact( uid );
	QString name;
	if ( mc )
		name = mc->displayName();
	
	return name;
}

int KIMIfaceImpl::presenceStatus( const QString & uid )
{
	kdDebug( 14000 ) << k_funcinfo << endl;
	int p = -1;
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
	return p;
}

QString KIMIfaceImpl::presenceString( const QString & uid )
{
	kdDebug( 14000 ) <<  "KIMIfaceImpl::presenceString" << endl;
	QString p;
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
	{
		KopeteOnlineStatus status = m->status();
			p = status.description();
		kdDebug( 14000 ) << "Got presence for " <<  uid << " : " << p.ascii() << endl;
	}
	else
	{
		kdDebug( 14000 ) << "Couldn't find MC: " << uid << endl;;
		p = QString();
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
		return QString::null;
}

KopeteMetaContact * KIMIfaceImpl::locateProtocolContact( const QString & contactId, const QString & protocolId )
{
	KopeteMetaContact *mc = 0;
	// find a matching protocol
	KopeteProtocol *protocol = dynamic_cast<KopeteProtocol*>( KopetePluginManager::self()->plugin( protocolId ) );

	if ( protocol )
	{
		// find its accounts
		QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( protocol );
		QDictIterator<KopeteAccount> it( accounts );
		for( ; it.current(); ++it )
		{
			mc = KopeteContactList::contactList()->findContact( protocolId, it.currentKey(), contactId  );
			if (mc)
				break;
		}
	}
	return mc;
}

QPixmap KIMIfaceImpl::icon( const QString & uid )
{
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

	return QString::null;
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
	// TODO: make it possible to specify the message here
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->sendMessage();
}

void KIMIfaceImpl::messageNewContact( const QString &contactId, const QString &protocol )
{
	KopeteMetaContact *mc = locateProtocolContact( contactId, protocol );
	if ( mc )
		mc->sendMessage();
}

void KIMIfaceImpl::chatWithContact( const QString &uid )
{
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->execute();
}

void KIMIfaceImpl::sendFile(const QString &uid, const KURL &sourceURL,
		const QString &altFileName, uint fileSize)
{
	KopeteMetaContact *m = KopeteContactList::contactList()->metaContact( uid );
	if ( m )
		m->sendFile( sourceURL, altFileName, fileSize );
}

bool KIMIfaceImpl::addContact( const QString &contactId, const QString &protocolId )
{
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
		int p = -1;
		KopeteOnlineStatus status = mc->status();
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
		// tell anyone who's listening over DCOP
		QByteArray params;
		QDataStream stream(params, IO_WriteOnly);
		stream << mc->metaContactId();
		stream << kapp->name();
		stream << p;
		kapp->dcopClient()->emitDCOPSignal( "contactPresenceChanged( QString, QCString, int )", params );
	}
}

#include "kimifaceimpl.moc"

