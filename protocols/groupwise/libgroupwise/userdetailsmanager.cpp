//
// C++ Implementation: userdetailsmanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

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
	QStringList::Iterator found = m_knownDNs.find( dn );
	return ( found != m_knownDNs.end() );
}

QStringList UserDetailsManager::knownDNs()
{
	return m_knownDNs;
}

void UserDetailsManager::addContact( const QString & dn )
{
	m_knownDNs.append( dn );
	qDebug( "UserDetailsManager::addContact, we now know: " );
	dump( m_knownDNs );
}

void UserDetailsManager::removeContact( const QString & dn )
{
	m_knownDNs.remove( m_knownDNs.find( dn ) );
}

void UserDetailsManager::requestDetails( const QStringList & dnList )
{
	// build a list of DNs that are not already subject to a pending request
	QStringList requestList;
	QValueListConstIterator<QString> end = dnList.end();
	for ( QValueListConstIterator<QString> it = dnList.begin(); it != end; ++it )
	{
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
//		connect( gdt, SIGNAL( gotContactUserDetails( const ContactDetails & ) ), 
//			SLOT( slotReceiveContactDetails( const ContactDetails & ) ) );
		connect( gdt, SIGNAL( gotContactUserDetails( const GroupWise::ContactDetails & ) ), 
			SLOT( slotReceiveContactDetails( const GroupWise::ContactDetails & ) ) );
		// TODO: connect to gdt's finished() signal, check for failures, expand gdt to maintain a list of not found DNs?
		gdt->go();
	}
	else
	{
		qDebug( "UserDetailsManager::requestDetails - all requested contacts are already pending" );
	}
}

void UserDetailsManager::requestDetails( const QString & dn )
{
	qDebug( "UserDetailsManager::requestDetails for %s", dn.ascii() );
	QStringList list;
	list.append( dn );
	requestDetails( list );
}

void UserDetailsManager::slotReceiveContactDetails( const GroupWise::ContactDetails & details )
{
	qDebug( "UserDetailsManager::slotReceiveContactDetails()");
	m_pendingDNs.remove( m_pendingDNs.find( details.dn ) );
	emit gotContactDetails( details );
}

#include "userdetailsmanager.moc"
