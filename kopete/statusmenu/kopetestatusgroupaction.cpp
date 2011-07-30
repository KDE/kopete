/*
    kopetestatusgroupaction.cpp - Kopete Status Group Action

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
#include "kopetestatusgroupaction.h"

#include <kopetestatusitems.h>
#include <kopeteonlinestatus.h>

#include "kopetestatusrootaction.h"
#include "kopetestatusaction.h"

namespace Kopete
{

StatusGroupAction::StatusGroupAction( Status::StatusGroup *group, StatusRootAction* rootAction, QObject * parent )
	: KActionMenu( parent ), mStatusGroup(group), mRootAction(rootAction)
{
	init();

	connect( mStatusGroup, SIGNAL(changed()), this, SLOT(changed()) );
	connect( mStatusGroup, SIGNAL(childRemoved(Kopete::Status::StatusItem*)),
	         this, SLOT(childRemoved(Kopete::Status::StatusItem*)) );
	connect( mStatusGroup, SIGNAL(childInserted(int,Kopete::Status::StatusItem*)),
	         this, SLOT(childInserted(int,Kopete::Status::StatusItem*)) );

	foreach( Kopete::Status::StatusItem* child, mStatusGroup->childList() )
		insertChild( 0, child );
}

int StatusGroupAction::childCount() const
{
	return mChildMap.count();
}

void StatusGroupAction::init()
{
	this->setText( mStatusGroup->title() );

	if ( mRootAction->filter() == StatusRootAction::UseCategory )
		this->setIcon( Kopete::OnlineStatusManager::pixmapForCategory( mStatusGroup->category() ) );
	else
		this->setIcon( mRootAction->onlineStatus().iconFor( mRootAction->account() ) );
}

void StatusGroupAction::changed()
{
	init();
}

void StatusGroupAction::childInserted( int i, Kopete::Status::StatusItem* child )
{
	Status::StatusItem* before = mStatusGroup->child( i + 1 );
	if ( before )
		insertChild( mChildMap.value( before, 0 ), child );
	else
		insertChild( 0, child );
}

void StatusGroupAction::insertChild( QAction * before, Status::StatusItem* child )
{
	if ( child->isGroup() )
	{
		Kopete::Status::StatusGroup *group = qobject_cast<Kopete::Status::StatusGroup*>(child);
		StatusGroupAction *groupAction = new StatusGroupAction( group, mRootAction, this );
		if ( groupAction->childCount() == 0 )
		{
			delete groupAction;
			return;
		}

		mChildMap.insert( group, groupAction );
		this->insertAction( before, groupAction );
	}
	else
	{
		if ( mRootAction->filter() == StatusRootAction::UseStatusAndCategory
		     && child->category() != 0 && (child->category() & mRootAction->category()) == 0 )
			return;
		
		Kopete::Status::Status* status = qobject_cast<Kopete::Status::Status*>(child);
		StatusAction *action = new StatusAction( status, mRootAction, this );
		mChildMap.insert( status, action );
		this->insertAction( before, action );
	}
}

void StatusGroupAction::childRemoved( Kopete::Status::StatusItem* item )
{
	QAction *action = mChildMap.value( item );
	this->removeAction( action );
	mChildMap.remove( item );
	delete action;
}

}

#include "kopetestatusgroupaction.moc"
