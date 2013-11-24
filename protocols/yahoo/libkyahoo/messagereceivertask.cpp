/*
    Kopete Yahoo Protocol
    Receive Messages

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "messagereceivertask.h"

#include <qstring.h>

#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <kdebug.h>

using namespace KYahoo;

MessageReceiverTask::MessageReceiverTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

MessageReceiverTask::~MessageReceiverTask()
{
}

bool MessageReceiverTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;
	
	if( t->service() == Yahoo::ServiceNotify )
	{
		parseNotify( t );
	}
	else
	{
		if( t->service() == Yahoo::ServiceAnimatedAudibleIcon )
		{
			parseAnimatedAudibleIcon( t );
		}
		else
		{
			parseMessage( t );
		}
	}

	return true;
}

bool MessageReceiverTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceMessage ||
		t->service() == Yahoo::ServiceGameMsg ||
		t->service() == Yahoo::ServiceSysMessage ||
		t->service() == Yahoo::ServiceNotify ||
		t->service() == Yahoo::ServiceAnimatedAudibleIcon )	
		return true;
	else
		return false;
}

void MessageReceiverTask::parseMessage( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	int cnt = t->paramCount( 5 );
	for( int i = 0; i < cnt; ++i )
	{
		QString to = t->nthParam( 5, i );
		QString timestamp = t->nthParamSeparated( 15, i, 4 );
		QString utf8 = t->nthParamSeparated( 97, i, 4 );
		QString from = t->nthParamSeparated( 1, i, 4 ).isEmpty() ? t->nthParam( 4, i ) : t->nthParamSeparated( 1, i, 4 );
		QString msg = t->nthParamSeparated( 14, i, 4 );
		QString sysmsg = t->nthParamSeparated( 16, i, 4 );

		// The arrangement of the key->value pairs is different when there is only one message in the packet.
		// Separating by key "5" (sender) doesn't work in that case, because the "1" and "4" keys are sent before the "5" key
		if( cnt == 1 )
			from = t->firstParam( 1 ).isEmpty() ? t->firstParam( 4 ) : t->firstParam( 1 );

		if( !sysmsg.isEmpty() )
		{
			client()->notifyError( "Server message received: ", sysmsg, Client::Error );
			continue;
		}
	
		if( msg.isEmpty() )
		{
			kDebug(YAHOO_RAW_DEBUG) << "Got a empty message. Dropped.";
			continue;
		}
	
		if( utf8.startsWith( '1' ) )
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
}

void MessageReceiverTask::parseAnimatedAudibleIcon( YMSGTransfer *t )
{
	// added by michaelacole
	kDebug(YAHOO_RAW_DEBUG) ;

	int cnt = t->paramCount( 5 );
	for( int i = 0; i < cnt; ++i )
	{
		QString to = t->nthParam( 5, i );
		QString from = t->nthParamSeparated( 1, i, 4 ).isEmpty() ? t->nthParam( 4, i ) : t->nthParamSeparated( 1, i, 4 );
		QString msg = t->nthParamSeparated( 231, i, 4 );
		QString msg2 = t->nthParamSeparated( 230, i, 4 );
		QString utf8 = t->nthParamSeparated( 97, i, 4 );
		QString timestamp = t->nthParamSeparated( 15, i, 4 );

		// The arrangement of the key->value pairs is different when there is only one message in the packet.
		// Separating by key "5" (sender) doesn't work in that case, because the "1" and "4" keys are sent before the "5" key
		if( cnt == 1 )
			from = t->firstParam( 1 ).isEmpty() ? t->firstParam( 4 ) : t->firstParam( 1 );

		if( msg.isEmpty() )
		{
			kDebug(YAHOO_RAW_DEBUG) << "Got a empty message. Dropped.";
			continue;
		}

		if( utf8.startsWith( '1' ) )
		{
			msg = QString::fromUtf8( msg.toLatin1() );
		}
		
		msg = "ANIMATED AUDIBLE SENT TO YOU WITH TEXT " + msg;
		emit gotIm( from, msg, timestamp.toLong(), 0);
		msg2 = "http://us.dl1.yimg.com//download.yahoo.com/dl/aud/"+msg2.mid(5,2)+"/" + msg2 + ".swf";
		emit gotIm( from, msg2,timestamp.toLong(), 0);

	}
}

void MessageReceiverTask::parseNotify( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

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
		if( ind.startsWith(' ') )
		{
			kDebug(YAHOO_RAW_DEBUG) << "Got a WebcamInvitation.";
			emit gotWebcamInvite( from );
		}
		else
		{
			kDebug(YAHOO_RAW_DEBUG) << "Got a WebcamRequest-Response: " << ind.toInt();
		}
	}
}

#include "messagereceivertask.moc"
