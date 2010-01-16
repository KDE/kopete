/*
    Kopete Yahoo Protocol
    Request a Picture of a Buddy

    Copyright (c) 2005-2006 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "requestpicturetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"

#include <kdebug.h>

RequestPictureTask::RequestPictureTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

RequestPictureTask::~RequestPictureTask()
{
}

void RequestPictureTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePicture);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit());
	t->setParam( 5, m_target.toLocal8Bit() );
	t->setParam( 13, "1" );
	send( t );
	
	setSuccess();
}

void RequestPictureTask::setTarget( const QString &target )
{
	m_target = target;
}

#include "requestpicturetask.moc"

