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

#include "client.h"
#include "tasks/getdetailstask.h"

#include "userdetailsmanager.h"

UserDetailsManager::UserDetailsManager( Client * parent, const char *name)
 : QObject(parent, name), m_client( parent )
{
}

UserDetailsManager::~UserDetailsManager()
{
}

void dump( QStringList & list )
{
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
	{
		qDebug( " - %s", (*it).ascii() );
	}
}

bool UserDetailsManager::known( const QString & dn )
{
	if ( dn == m_client->userDN() )
		return true;
	QStringList::Iterator found = m_detailsMap.keys().find( dn );
	// we always know the local user's details, so don't look them up
	return ( found !=m_detailsMap.keys().end() );
}

ContactDetails UserDetailsManager::details( const QString & dn )
{
	return m_detailsMap[ dn ];
}

QStringList UserDetailsManager::knownDNs()
{
	return m_detailsMap.keys();
}

void UserDetailsManager::addDetails( const ContactDetails & details )
{
	qDebug( "UserDetailsManager::addContact, got %s, we now know: ", details.dn.ascii() );
	m_detailsMap.insert( details.dn, details );
	QStringList keys = m_detailsMap.keys();
	dump( keys );
	qDebug( "UserDetailsManager::addContact, pending: " );
	dump( m_pendingDNs );
}

void UserDetailsManager::removeContact( const QString & dn )
{
	m_detailsMap.remove( dn );
}

void UserDetailsManager::requestDetails( const QStringList & dnList, bool onlyUnknown )
{
	// build a list of DNs that are not already subject to a pending request
	QStringList requestList;
	QValueListConstIterator<QString> end = dnList.end();
	for ( QValueListConstIterator<QString> it = dnList.begin(); it != end; ++it )
	{
		// don't request our own details
		if ( *it == m_client->userDN() )
			break;
		// don't request details we already have unless the caller specified this
		if ( onlyUnknown && known( *it ) )
			break;
		QStringList::Iterator found = m_pendingDNs.find( *it );
		if ( found == m_pendingDNs.end() )
		{
			qDebug( "UserDetailsManager::requestDetails - including %s", (*it).ascii() );
			requestList.append( *it );
			m_pendingDNs.append( *it );
		}
	}
	if ( !requestList.empty() )
	{
		GetDetailsTask * gdt = new GetDetailsTask( m_client->rootTask() );
		gdt->userDNs( requestList );
		connect( gdt, SIGNAL( gotContactUserDetails( const GroupWise::ContactDetails & ) ), 
			SLOT( slotReceiveContactDetails( const GroupWise::ContactDetails & ) ) );
		// TODO: connect to gdt's finished() signal, check for failures, expand gdt to maintain a list of not found DNs?
		gdt->go( true );
	}
	else
	{
		qDebug( "UserDetailsManager::requestDetails - all requested contacts are already available or pending" );
	}
}

void UserDetailsManager::requestDetails( const QString & dn, bool onlyUnknown )
{
	qDebug( "UserDetailsManager::requestDetails for %s", dn.ascii() );
	QStringList list;
	list.append( dn );
	requestDetails( list, onlyUnknown );
}

void UserDetailsManager::slotReceiveContactDetails( const GroupWise::ContactDetails & details )
{
	qDebug( "UserDetailsManager::slotReceiveContactDetails()");
	m_pendingDNs.remove( details.dn );
	/*client()->userDetailsManager()->*/
	addDetails( details );
	//emit temporaryContact( details );
	emit gotContactDetails( details );
}

#include "userdetailsmanager.moc"
