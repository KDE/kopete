/*
    kopetestatusaction.cpp - Kopete Status Action

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
#include "kopetestatusaction.h"

#include <kopeteonlinestatus.h>
#include <kopetestatusitems.h>

#include "kopetestatusrootaction.h"


namespace Kopete {

StatusAction::StatusAction( Status::Status *status, StatusRootAction* rootAction, QObject * parent )
	: KAction( parent ), mStatus(status), mRootAction(rootAction)
{
	connect( this, SIGNAL(triggered(bool)), this, SLOT(triggered()) );
	connect( mStatus, SIGNAL(changed()), this, SLOT(changed()) );
	init();
}

void StatusAction::init()
{
	if ( mRootAction->filter() == StatusRootAction::UseCategory )
	{
		this->setIcon( Kopete::OnlineStatusManager::pixmapForCategory( mStatus->category() ) );
		this->setText( mStatus->title() );
	}
	else
	{
		this->setIcon( KIcon(mRootAction->onlineStatus().iconFor( mRootAction->account() )) );
		this->setText( mStatus->title() );
	}
	
	this->setData( mStatus->uid() );
	this->setToolTip( mStatus->message() );
}

void StatusAction::triggered()
{
	mRootAction->changeStatus( mStatus );
}

void StatusAction::changed()
{

}

}

#include "kopetestatusaction.moc"
