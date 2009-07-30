/*
    kopetestatusitems.cpp - Kopete Status Items

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetestatusitems.h"

#include <QtCore/QUuid>

namespace Kopete {

namespace Status {

StatusItem::StatusItem()
	: QObject(), mParentItem(0)
{
	mUid = QUuid::createUuid().toString();
}

StatusItem::StatusItem( const QString& uid )
	: mParentItem(0)
{
	mUid = uid;
}

void StatusItem::setCategory( OnlineStatusManager::Categories category )
{
	mCategory = category;
	emit changed();
}

void StatusItem::setTitle( const QString& title )
{
	mTitle = title;
	emit changed();
}

StatusGroup *StatusItem::parentGroup() const
{
	return qobject_cast<StatusGroup*>(parent());
}

int StatusItem::index() const
{
	if ( parent() )
		return parentGroup()->indexOf( const_cast<StatusItem*>(this) );
	
	return 0;
}

/****************************************************/
/****************************************************/

StatusGroup::StatusGroup()
	: StatusItem()
{
}

StatusGroup::StatusGroup( const QString& uid )
	: StatusItem( uid )
{
}

void StatusGroup::insertChild( int i, StatusItem *item )
{
	item->setParent( this );
	connect( item, SIGNAL(destroyed(QObject*)), this, SLOT(childDestroyed(QObject*)) );
	mChildItems.insert( i, item );
	emit childInserted( i, item );
}

void StatusGroup::appendChild( Kopete::Status::StatusItem *item )
{
	insertChild( mChildItems.size(), item );
}

void StatusGroup::removeChild( Kopete::Status::StatusItem *item )
{
	item->setParent( 0 );
	disconnect( item, 0, this, 0 );
	mChildItems.removeAll( item );
	emit childRemoved( item );
}

StatusItem* StatusGroup::copy() const
{
	StatusGroup* newGroup = new StatusGroup( uid() );
	newGroup->setTitle( title() );
	newGroup->setCategory( category() );
	
	foreach( StatusItem* item, mChildItems )
		newGroup->appendChild( item->copy() );

	return newGroup;
}

void StatusGroup::childDestroyed( QObject *object )
{
	StatusItem *item = static_cast<StatusItem*>(object);
	mChildItems.removeAll( item );
	emit childRemoved( item );
}

/****************************************************/
/****************************************************/

Status::Status()
	: StatusItem()
{
}

Status::Status( const QString& uid )
	: StatusItem( uid )
{
}

void Status::setMessage( const QString& message )
{
	mMessage = message;
	emit changed();
}

StatusItem* Status::copy() const
{
	Status* newStatus = new Status( uid() );
	newStatus->setTitle( title() );
	newStatus->setCategory( category() );
	newStatus->setMessage( message() );
	return newStatus;
}

}

}

#include "kopetestatusitems.moc"
