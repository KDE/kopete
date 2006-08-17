/*
    gwcontactlist.cpp - Kopete GroupWise Protocol

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qobjectlist.h>

#include <kdebug.h>

#include "gwcontactlist.h"
#include "gwerror.h" //debug area

GWContactList::GWContactList( QObject * parent )
 : QObject( parent ), rootFolder( new GWFolder( this, 0, 0, QString::null ) )
{  }

GWFolder * GWContactList::addFolder( unsigned int id, unsigned int sequence, const QString & displayName )
{
	if ( rootFolder )
		return new GWFolder( rootFolder, id, sequence, displayName );
	else
		return 0;
}

GWContactInstance * GWContactList::addContactInstance( unsigned int id, unsigned int parent, unsigned int sequence, const QString & displayName, const QString & dn )
{
	QObjectList * l = queryList( "GWFolder", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;
	GWContactInstance * contact = 0;
    while ( (obj = it.current()) != 0 )
	{
		GWFolder * folder = ::qt_cast< GWFolder * >( obj );
		if ( folder && folder->id == parent )
		{
			contact = new GWContactInstance( folder, id, sequence, displayName, dn );
			break;
		}
		++it;
	}
	delete l;
	return contact;
}

GWFolder * GWContactList::findFolderById( unsigned int id )
{
	QObjectList * l = queryList( "GWFolder", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;
	GWFolder * candidate, * folder = 0;
    while ( (obj = it.current()) != 0 )
	{
		candidate = ::qt_cast< GWFolder * >( obj );
		if ( candidate->id == id )
		{
			folder = candidate;
			break;
		}
		++it;
	}
	delete l;
	return folder;
}

GWFolder * GWContactList::findFolderByName( const QString & displayName )
{
	QObjectList * l = queryList( "GWFolder", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;
	GWFolder *  folder = 0;
    while ( (obj = it.current()) != 0 )
	{
		GWFolder * candidate = ::qt_cast< GWFolder * >( obj );
		if ( candidate->displayName == displayName )
		{
			folder = candidate;
			break;
		}
		++it;
	}
	delete l;
	return folder;
}

int GWContactList::maxSequenceNumber()
{
	QObjectList * l = queryList( "GWFolder", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
	QObject *obj;
	unsigned int sequence = 0;
	while ( (obj = it.current()) != 0 )
	{
		GWFolder * current = ::qt_cast< GWFolder * >( obj );
		sequence = QMAX( sequence, current->sequence );
		++it;
	}
	delete l;
	return sequence;
}

GWContactInstanceList GWContactList::instancesWithDn( const QString & dn )
{
	QObjectList * l = queryList( "GWContactInstance", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
	QObject *obj;
	GWContactInstanceList matches;
	while ( (obj = it.current()) != 0 )
	{
		++it;
		GWContactInstance * current = ::qt_cast<GWContactInstance *>( obj );
		if ( current->dn == dn )
			matches.append( current );
	}
	delete l;
	return matches;
}

void GWContactList::removeInstance( GWContactListItem * instance )
{
	delete instance;
}

void GWContactList::removeInstanceById( unsigned int id )
{
	QObjectList * l = queryList( "GWContactInstance", 0, false, true );
	QObjectListIt it( *l ); // iterate over the buttons
	QObject *obj;
	GWContactInstanceList matches;
	while ( (obj = it.current()) != 0 )
	{
		++it;
		GWContactInstance * current = ::qt_cast<GWContactInstance *>( obj );
		if ( current->id == id )
		{
			delete current;
			break;
		}
	}
	delete l;
}

void GWContactList::dump()
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	const QObjectList * l = children();
	if ( l && !l->isEmpty() )
	{
		QObjectListIt it( *l ); // iterate over the buttons
		QObject *obj;
		while ( (obj = it.current()) != 0 )
		{
			GWFolder * folder = ::qt_cast< GWFolder * >( obj );
			if ( folder )
				folder->dump( 1 );
			++it;
		}
	}
	else
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "  contact list is empty." << endl;
}

void GWContactList::clear()
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	const QObjectList * l = children();
	if ( l && !l->isEmpty() )
	{
		QObjectListIt it( *l );
		QObject *obj;
		while ( (obj = it.current()) != 0 )
		{
			delete obj;
			++it;
		}
	}
}

GWContactListItem::GWContactListItem( QObject * parent, unsigned int theId, unsigned int theSequence, const QString & theDisplayName ) :
	QObject( parent), id( theId ), sequence( theSequence ), displayName( theDisplayName )
{ }

GWFolder::GWFolder( QObject * parent, unsigned int theId,  unsigned int theSequence, const QString & theDisplayName ) :
	GWContactListItem( parent, theId, theSequence, theDisplayName )
{ }

void GWFolder::dump( unsigned int depth )
{
	QString s;
	s.fill( ' ', ++depth * 2 );
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << s <<"Folder " << displayName << " id: " << id << " contains: " << endl;
	const QObjectList * l = children();
	if ( l )
	{
		QObjectListIt it( *l ); // iterate over the buttons
		QObject *obj;
		while ( (obj = it.current()) != 0 )
		{
			++it;
			GWContactInstance * instance = ::qt_cast< GWContactInstance * >( obj );
			if (instance)
				instance->dump( depth );
			else
			{
				GWFolder * folder = ::qt_cast< GWFolder * >( obj );
				if ( folder )
					folder->dump( depth );
			}
		}
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << s << "  no contacts." << endl;
}

GWContactInstance::GWContactInstance( QObject * parent, unsigned int theId, unsigned int theSequence, const QString & theDisplayName, const QString & theDn ) :
	GWContactListItem( parent, theId, theSequence, theDisplayName ), dn( theDn )
{ }

void GWContactInstance::dump( unsigned int depth )
{
	QString s;
	s.fill( ' ', ++depth * 2 );
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << s << "Contact " << displayName << " id: " << id << " dn: " << dn << endl;
}
#include "gwcontactlist.moc"

