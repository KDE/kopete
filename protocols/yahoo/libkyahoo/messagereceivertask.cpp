/*
    Kopete Yahoo Protocol
    Receive Messages

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

#include <qstring.h>

#include "messagereceivertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>

MessageReceiverTask::MessageReceiverTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
}

MessageReceiverTask::~MessageReceiverTask()
{
}

bool MessageReceiverTask::take( Transfer* transfer )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
	if( t->service() == Yahoo::ServiceNotify )
		parseNotify( t );
	else
		parseMessage( t );

	return true;
}

bool MessageReceiverTask::forMe( Transfer* transfer ) const
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;
	
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceMessage ||
		t->service() == Yahoo::ServiceGameMsg ||
		t->service() == Yahoo::ServiceSysMessage ||
		t->service() == Yahoo::ServiceNotify )	
		return true;
	else
		return false;
}

void MessageReceiverTask::parseMessage( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString to = t->firstParam( 5 );
	QString timestamp = t->firstParam( 15 );
	QString utf8 = t->firstParam( 97 );
	QString from = t->firstParam( 1 ).isEmpty() ? t->firstParam( 4 ) : t->firstParam( 1 );
	QString msg = t->firstParam( 14 );
	QString sysmsg = t->firstParam( 16 );

	if( msg.isEmpty() )
	{
		kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Got a empty message. Dropped." << endl;
		return;
	}

	if( utf8.startsWith( "1" ) )
		msg = QString::fromUtf8( msg.toLatin1() );

	if( t->service() == Yahoo::ServiceSysMessage )
		emit systemMessage( sysmsg );
	else
	{	
		if( msg.startsWith( "<ding>" ) )
			emit gotBuzz( from, timestamp.toLong() );
		else
			emit gotIm( from, msg, timestamp.toLong(), 0);
	}
}

void MessageReceiverTask::parseNotify( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << endl;

	QString from = t->firstParam( 4 );
	//QString to = t->firstParam( 5 );
	QString type = t->firstParam( 49 );
	QString stat = t->firstParam( 13 );
	QString ind = t->firstParam( 14 );

	if( type.startsWith( "TYPING" ) )
		emit gotTypingNotify( from, stat.toInt() );
	else if( type.startsWith( "GAME" ) )
		;
	else if( type.startsWith( "WEBCAMINVITE" ) )
	{
		if( ind.startsWith(" ") )
		{
			kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Got a WebcamInvitation." << endl;
			emit gotWebcamInvite( from );
		}
		else
		{
			kDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Got a WebcamRequest-Response: " << ind.toInt() << endl;
		}
	}
}

#include "messagereceivertask.moc"
