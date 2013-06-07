/*
    Kopete Yahoo Protocol
    Notifies about status changes of buddies

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

#include "statusnotifiertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KYahoo;

StatusNotifierTask::StatusNotifierTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

StatusNotifierTask::~StatusNotifierTask()
{

}

bool StatusNotifierTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;
	
	YMSGTransfer *t = static_cast<YMSGTransfer*>(transfer);

	if( t->service() == Yahoo::ServiceStealthOffline )
		parseStealthStatus( t );
	else if( t->service() == Yahoo::ServiceAuthorization )
		parseAuthorization( t );
	else
		parseStatus( t );

	return true;
}

bool StatusNotifierTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServiceLogon ||
		t->service() == Yahoo::ServiceLogoff ||
		t->service() == Yahoo::ServiceIsAway ||
		t->service() == Yahoo::ServiceIsBack ||
		t->service() == Yahoo::ServiceGameLogon ||
		t->service() == Yahoo::ServiceGameLogoff ||
		t->service() == Yahoo::ServiceIdAct ||
		t->service() == Yahoo::ServiceIddeAct ||
		t->service() == Yahoo::ServiceStatus ||
		t->service() == Yahoo::ServiceStealthOffline ||
		t->service() == Yahoo::ServiceAuthorization ||
		t->service() == Yahoo::ServiceBuddyStatus
	)
		return true;
	else
		return false;
}

void StatusNotifierTask::parseStatus( YMSGTransfer* t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	if( t->status() == Yahoo::StatusDisconnected && 
		t->service() == Yahoo::ServiceLogoff )
	{
		emit loginResponse( Yahoo::LoginDupl, QString() );
	}

	QString	myNick;		/* key = 1 */
	QString customError;	/* key = 16  */
	QString nick;		/* key = 7  */
	int state;		/* key = 10  */
	QString message;	/* key = 19  */
//	int flags;		/* key = 13  */
	int away;		/* key = 47  */
	int idle;		/* key = 137 */
	bool utf;		/* key = 97 */
	int pictureChecksum;	/* key = 192 */

	customError = t->firstParam( 16 );
	if( !customError.isEmpty() )
		client()->notifyError( i18n("An unknown error has occurred."), customError, Client::Warning );

	myNick = t->firstParam( 1 );
	
	for( int i = 0; i < t->paramCount( 7 ); ++i)
	{
		nick = t->nthParam( 7, i );
		state = t->nthParam( 10, i ).toInt();
//		flags = t->nthParamSeparated( 13, i, 7 ).toInt();
		away = t->nthParamSeparated( 47, i, 7 ).toInt();
		idle = t->nthParamSeparated( 137, i, 7 ).toInt();
		utf = t->nthParamSeparated( 97, i, 7 ).toInt() == 1;
		pictureChecksum = t->nthParamSeparated( 192, i, 7 ).toInt();
		if( utf )
			message = QString::fromUtf8( t->nthParamSeparated( 19, i, 7 ) );
		else
			message = t->nthParamSeparated( 19, i, 7 );

		if(state == 99 && message == "I'm on SMS")
			state = 10;

		if( t->service() == Yahoo::ServiceLogoff )
			emit statusChanged( nick, Yahoo::StatusOffline, QString(), 0, 0, 0 );
		else
			emit statusChanged( nick, state, message, away, idle, pictureChecksum );
	}
}

void StatusNotifierTask::parseAuthorization( YMSGTransfer* t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString nick;		/* key = 4  */	
	QString msg;		/* key = 14  */
	int state;		/* key = 13  */
	bool utf;		/* key = 97 */

	utf = t->firstParam( 97 ).toInt() == 1;
	nick = t->firstParam( 4 );
	if( utf )
		msg = QString::fromUtf8( t->firstParam( 14 ) );
	else
		msg = t->firstParam( 14 );
	state = t->firstParam( 13 ).toInt();

	if( state == 1 )
	{
		emit( authorizationAccepted( nick ) );
	}
	else if( state == 2 )
	{
		emit( authorizationRejected( nick, msg ) );
	}
	else	// This is a request
	{
		QString fname = t->firstParam( 216 );
		QString lname = t->firstParam( 254 );
		QString name;
		if( !fname.isEmpty() || !lname.isEmpty() )
			name = QString("%1 %2").arg(fname).arg(lname);

		kDebug(YAHOO_RAW_DEBUG) << "Emitting gotAuthorizationRequest( " << nick<< ", " << msg << ", " << name << " )";
		emit gotAuthorizationRequest( nick, msg, name );
	}
}

void StatusNotifierTask::parseStealthStatus( YMSGTransfer* t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString nick;		/* key = 7  */
	int state;		/* key = 31  */

	nick = t->firstParam( 7 );
	state = t->firstParam( 31 ).toInt();

	emit stealthStatusChanged( nick, ( state == 1 ) ? Yahoo::StealthActive : Yahoo::StealthNotActive );
}

#include "statusnotifiertask.moc"
