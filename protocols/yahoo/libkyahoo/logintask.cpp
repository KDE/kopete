/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

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

#include <qstring.h>

#include "logintask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"

LoginTask::LoginTask(Task* parent) : Task(parent)
{
	mState = InitialState;
}

LoginTask::~LoginTask()
{

}

bool LoginTask::take(Transfer* transfer)
{
	switch (mState)
	{
		case (InitialState):
			kdDebug(14180) << k_funcinfo << " - ERROR - take called while in initial state" << endl;
			return false;
		break;
		case (SentVerify):
			sendAuth(transfer);
			return true;
		break;
		case (SentAuth):
			sendAuthResp(transfer);
			return true;
		break;
		case (SentAuthResp):
			kdDebug(14180) << k_funcinfo << " - ERROR - take called while in SentAuthResp state" << endl;
			return true;
		break;
		default:
		return false;
		break;
	}
	return false;
}

bool LoginTask::forMe(Transfer* transfer) const
{
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	switch (mState)
	{
		case (InitialState):
			//there shouldn't be a incoming transfer for this task at this state
			return false;
		break;
		case (SentVerify):
			if ( t->service() == Yahoo::ServiceVerify )
			return true;
		break;
		case (SentAuth):
			if ( t->service() == Yahoo::ServiceAuth )
			return true;
		break;
		default:
			return false;
		break;
	}
	return false;
}

void LoginTask::onGo()
{
	/* initial state, we have to send a ServiceVerify */
	if (mState == InitialState)
		sendVerify();
	else
		kdDebug(14180) << k_funcinfo << " - ERROR - OnGo called and not initial state" << endl;
	//emit finished();
}

void LoginTask::sendVerify()
{
	/* send a ServiceVerify */
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceVerify);
	//t->setParam("1", client()->userId());
	send( t );
	mState = SentVerify;	
}

void LoginTask::sendAuth(Transfer* transfer)
{
	kdDebug(14180) << k_funcinfo << endl;
	// transfer is the verify ack transfer, no useful data in it.
	Q_UNUSED(transfer);
	
	/* got ServiceVerify ACK, send a ServiceAuth with username */
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceAuth);
	t->setParam("1", client()->userId());
	send(t);
	mState = SentAuth;
}

void LoginTask::sendAuthResp(Transfer* transfer)
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
	{
		setSuccess(false);
		return;
	}
	
	
	
}