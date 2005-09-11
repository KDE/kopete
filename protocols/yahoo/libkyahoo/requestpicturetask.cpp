/*
    Kopete Yahoo Protocol
    Request a Picture of a Buddy

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

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
#include <qstring.h>

RequestPictureTask::RequestPictureTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

RequestPictureTask::~RequestPictureTask()
{
}

void RequestPictureTask::onGo()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServicePicture);
	t->setId( client()->sessionID() );
	t->setParam( 4, client()->userId());
	t->setParam( 5, m_target );
	t->setParam( 13, QString::fromLatin1("1") );
	send( t );
	
	setSuccess( true );
}

void RequestPictureTask::setTarget( const QString &target )
{
	m_target = target;
}
