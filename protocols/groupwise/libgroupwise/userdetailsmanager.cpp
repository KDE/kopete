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

QStringList UserDetailsManager::knownDNs()
{
	return m_knownDNs;
}

void UserDetailsManager::addContact( const QString & dn )
{
	m_knownDNs.append( dn );
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
		if ( found != m_pendingDNs.end() )
		{
			qDebug( "UserDetailsManager::requestDetails - including %s", (*it).ascii() );
			requestList.append( *it );
			m_pendingDNs.append( *it );
		}
	}
	
	GetDetailsTask * gdt = new GetDetailsTask( m_client->rootTask() );
	gdt->userDNs( requestList );
	connect( gdt, SIGNAL( gotContactUserDetails( const GroupWise::ContactDetails & ) ), SLOT( slotReceiveContactDetails( const GroupWise::ContactDetails & ) ) );
	// TODO: connect to gdt's finished() signal, check for failures, expand gdt to maintain a list of not found DNs?
	gdt->go();
}

void UserDetailsManager::requestDetails( const QString & dn )
{
	QStringList list;
	list.append( dn );
	requestDetails( list );
}

void UserDetailsManager::slotReceiveContactDetails( const GroupWise::ContactDetails & details )
{
	m_knownDNs.append( details.dn );
	m_pendingDNs.remove( m_pendingDNs.find( details.dn ) );
	emit gotContactDetails( details );
}

#include "userdetailsmanager.moc"
