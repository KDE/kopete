/*
    userdetailsmanager.cpp - Storage of all user details seen during this session
   
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "userdetailsmanager.h"

#include "client.h"
#include "tasks/getdetailstask.h"

UserDetailsManager::UserDetailsManager( Client * parent)
 : QObject(parent), m_client( parent )
{
}

UserDetailsManager::~UserDetailsManager()
{
}

void UserDetailsManager::dump( const QStringList & list )
{
	for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
	{
		m_client->debug( QString( " - %1" ).arg (*it) );
	}
}

bool UserDetailsManager::known( const QString & dn )
{
	if ( dn == m_client->userDN() )
		return true;
	// we always know the local user's details, so don't look them up
	return ( m_detailsMap.keys().contains( dn ) );
}

ContactDetails UserDetailsManager::details( const QString & dn )
{
//	qDebug() << "UserDetailsManager::details() requested for " << dn.toLatin1();
	return m_detailsMap[ dn ];
}

QStringList UserDetailsManager::knownDNs()
{
	return m_detailsMap.keys();
}

void UserDetailsManager::addDetails( const ContactDetails & details )
{
//	qDebug() << "UserDetailsManager::addDetails, got " << details.dn;
	m_detailsMap.insert( details.dn, details );
/*	QStringList keys = m_detailsMap.keys();
	dump( keys );
	qDebug( "UserDetailsManager::addContact, pending: " );
	dump( m_pendingDNs );*/
}

void UserDetailsManager::removeContact( const QString & dn )
{
	m_detailsMap.remove( dn );
}

void UserDetailsManager::requestDetails( const QStringList & dnList, bool onlyUnknown )
{
	// build a list of DNs that are not already subject to a pending request
	QStringList requestList;
	QStringListIterator it( dnList );
	while ( it.hasNext() )
	{
		QString dn = it.next();
		// don't request our own details
		if ( dn == m_client->userDN() )
			break;
		// don't request details we already have unless the caller specified this
		if ( onlyUnknown && known( dn ) )
			break;
		if ( !m_pendingDNs.contains( dn ) )
		{
			m_client->debug( QString( "UserDetailsManager::requestDetails - including %1" ).arg( dn ) );
			requestList.append( dn);
			m_pendingDNs.append( dn );
		}
	}
	if ( !requestList.empty() )
	{
		GetDetailsTask * gdt = new GetDetailsTask( m_client->rootTask() );
		gdt->userDNs( requestList );
		connect( gdt, SIGNAL(gotContactUserDetails(GroupWise::ContactDetails)), 
			SLOT(slotReceiveContactDetails(GroupWise::ContactDetails)) );
		// TODO: connect to gdt's finished() signal, check for failures, expand gdt to maintain a list of not found DNs?
		gdt->go( true );
	}
	else
	{
		m_client->debug( "UserDetailsManager::requestDetails - all requested contacts are already available or pending" );
	}
}

void UserDetailsManager::requestDetails( const QString & dn, bool onlyUnknown )
{
	m_client->debug( QString( "UserDetailsManager::requestDetails for %1" ).arg( dn ) );
	QStringList list;
	list.append( dn );
	requestDetails( list, onlyUnknown );
}

void UserDetailsManager::slotReceiveContactDetails( const GroupWise::ContactDetails & details )
{
	m_client->debug( "UserDetailsManager::slotReceiveContactDetails()" );
	m_pendingDNs.removeAll( details.dn );
	/*client()->userDetailsManager()->*/
	addDetails( details );
		kDebug() 
		<< "  Auth attribute: " << details.authAttribute
		<< "  , Away message: " << details.awayMessage
		<< "  , CN" << details.cn
		<< "  , DN" << details.dn
		<< "  , fullName" << details.fullName
		<< "  , surname" << details.surname
		<< "  , givenname" << details.givenName
		<< "  , status" << details.status
		<< endl;
	//emit temporaryContact( details );
	emit gotContactDetails( details );
}

#include "userdetailsmanager.moc"
