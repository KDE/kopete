/*
    gwcontactlist.cpp - Kopete GroupWise Protocol

    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwcontactlist.h"
#include <qobject.h>

#include <kdebug.h>

#include "gwerror.h" //debug area

GWContactList::GWContactList( QObject * parent )
 : QObject( parent ), rootFolder( new GWFolder( this, 0, 0, QString() ) )
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
	GWContactInstance * contact = 0;
	foreach ( GWFolder *folder, findChildren<GWFolder *>() )
	{
		if ( folder && folder->id == parent )
		{
			contact = new GWContactInstance( folder, id, sequence, displayName, dn );
			break;
		}
	}
	return contact;
}

GWFolder * GWContactList::findFolderById( unsigned int id )
{
	GWFolder * folder = 0;
	foreach ( GWFolder *candidate, findChildren<GWFolder *>() )
	{
		if ( candidate->id == id )
		{
			folder = candidate;
			break;
		}
	}
	return folder;
}

GWFolder * GWContactList::findFolderByName( const QString & displayName )
{
	GWFolder * folder = 0;
	foreach ( GWFolder *candidate, findChildren<GWFolder *>() )
	{
		if ( candidate->displayName == displayName )
		{
			folder = candidate;
			break;
		}
	}
	return folder;
}

int GWContactList::maxSequenceNumber()
{
	unsigned int sequence = 0;
	foreach ( GWFolder *current, findChildren<GWFolder *>() )
	{
		sequence = qMax( sequence, current->sequence );
	}
	return sequence;
}

GWContactInstanceList GWContactList::instancesWithDn( const QString & dn )
{
	GWContactInstanceList matches;
	foreach ( GWContactInstance * current, findChildren<GWContactInstance *>() )
	{
		if ( current->dn == dn )
			matches.append( current );
	}
	return matches;
}

void GWContactList::removeInstance( GWContactListItem * instance )
{
	delete instance;
}

void GWContactList::removeInstanceById( unsigned int id )
{
	GWContactInstanceList matches;
	foreach ( GWContactInstance * current, findChildren<GWContactInstance *>() )
	{
		if ( current->id == id )
		{
			delete current;
			break;
		}
	}
}

void GWContactList::dump()
{
	kDebug() ;
	foreach ( GWFolder * folder, findChildren<GWFolder *>() )
	{
		if ( folder )
			folder->dump( 1 );
	}
}

void GWContactList::clear()
{
	kDebug() ;
	foreach ( QObject *obj, children() )
	{
		delete obj;
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
	kDebug() << s <<"Folder " << displayName << " id: " << id << " contains: ";
	foreach ( QObject *obj, children() )
	{
		GWContactInstance * instance = qobject_cast< GWContactInstance * >( obj );
		if (instance)
			instance->dump( depth );
		else
		{
			GWFolder * folder = qobject_cast< GWFolder * >( obj );
			if ( folder )
				folder->dump( depth );
		}
	}
}

GWContactInstance::GWContactInstance( QObject * parent, unsigned int theId, unsigned int theSequence, const QString & theDisplayName, const QString & theDn ) :
	GWContactListItem( parent, theId, theSequence, theDisplayName ), dn( theDn )
{ }

void GWContactInstance::dump( unsigned int depth )
{
	QString s;
	s.fill( ' ', ++depth * 2 );
	kDebug() << s << "Contact " << displayName << " id: " << id << " dn: " << dn;
}
#include "gwcontactlist.moc"

