/*
    Kopete Yahoo Protocol
    Handles conferences

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

#include "conferencetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>
#include <kdebug.h>

ConferenceTask::ConferenceTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

ConferenceTask::~ConferenceTask()
{
}

bool ConferenceTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	YMSGTransfer *t = 0L;
	t = static_cast<YMSGTransfer*>(transfer);
	
 	if( t->service() == Yahoo::ServiceConfInvite ||
		t->service() == Yahoo::ServiceConfAddInvite)
 		parseInvitation( t );
	else if( t->service() == Yahoo::ServiceConfMsg )
		parseMessage( t );
	else if( t->service() == Yahoo::ServiceConfLogon )
		parseUserJoined( t );
	else if( t->service() == Yahoo::ServiceConfLogoff )
		parseUserLeft( t );
	else if( t->service() == Yahoo::ServiceConfDecline )
		parseUserDeclined( t );

	return true;
}

bool ConferenceTask::forMe( const Transfer* transfer ) const
{
	const YMSGTransfer *t = 0L;
	t = dynamic_cast<const YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceConfInvite ||
		t->service() == Yahoo::ServiceConfLogon ||
		t->service() == Yahoo::ServiceConfDecline ||
		t->service() == Yahoo::ServiceConfLogoff ||
		t->service() == Yahoo::ServiceConfAddInvite ||
		t->service() == Yahoo::ServiceConfMsg )	
		return true;
	else
		return false;
}

void ConferenceTask::parseInvitation( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;
	
	int i = 0;
	QString who = t->firstParam( 50 );
	QString room = t->firstParam( 57 );
	bool utf = QString( t->firstParam( 97 ) ).toInt() == 1;
	QString msg;
	if( utf )
		msg = QString::fromUtf8( t->firstParam( 58 ) );
	else
		msg = t->firstParam( 58 );

	QStringList members;
	for( i = 0; i < t->paramCount( 52 ); ++i )
		members.append( t->nthParam( 52, i ) );
	for( i = 0; i < t->paramCount( 53 ); ++i )
		members.append( t->nthParam( 53, i ) );
	if( who == client()->userId() )
		return;

	if( !who.isEmpty() && !room.isEmpty() )
		emit gotInvite( who, room, msg, members );
}

void ConferenceTask::parseMessage( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString room = t->firstParam( 57 );
	QString from = t->firstParam( 3 );
	bool utf = QString( t->firstParam( 97 ) ).toInt() == 1;
	QString msg;
	if( utf )
		msg = QString::fromUtf8( t->firstParam( 14 ) );
	else
		msg = t->firstParam( 14 );

	if( !msg.isEmpty() )
		emit gotMessage( from, room, msg ); 
}

void ConferenceTask::parseUserJoined( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString room = t->firstParam( 57 );
	QString who = t->firstParam( 53 );

	if( !who.isEmpty() && !room.isEmpty() )
		emit userJoined( who, room );
}

void ConferenceTask::parseUserLeft( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString room = t->firstParam( 57 );
	QString who = t->firstParam( 56 );

	if( !who.isEmpty() && !room.isEmpty() )
		emit userLeft( who, room );
}

void ConferenceTask::parseUserDeclined( YMSGTransfer *t )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	QString room = t->firstParam( 57 );
	QString who = t->firstParam( 54 );
	QString msg = t->firstParam( 14 );

	if( !who.isEmpty() && !room.isEmpty() )
		emit userDeclined( who, room, msg );
}

void ConferenceTask::inviteConference( const QString &room, const QStringList &members, const QString &msg )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfInvite);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 50, client()->userId().toLocal8Bit() );
	t->setParam( 57, room.toLocal8Bit() );
	t->setParam( 58, msg.toLocal8Bit() );
	t->setParam( 97, 1 );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
		t->setParam( 52, (*it).toLocal8Bit() );
	t->setParam( 13, "0" );

	send( t );
}

void ConferenceTask::addInvite( const QString &room, const QStringList &who, const QStringList &members, const QString &msg )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfAddInvite);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );

	QString whoList = who.first();
	for( int i = 1; i < who.size(); i++ )
		whoList += QString(",%1").arg( who[i] );
	t->setParam( 51, whoList.toLocal8Bit() );

	t->setParam( 57, room.toLocal8Bit() );
	t->setParam( 58, msg.toLocal8Bit() );
	t->setParam( 97, 1 );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
	{
		t->setParam( 52, (*it).toLocal8Bit() );
		t->setParam( 53, (*it).toLocal8Bit() );	// Note: this field should only be set if the buddy has already joined the conference, but no harm is done this way
	}
	t->setParam( 13, "0" );

	send( t );
}

void ConferenceTask::joinConference( const QString &room, const QStringList &members )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfLogon);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
		t->setParam( 3, (*it).toLocal8Bit() );
	t->setParam( 57, room.toLocal8Bit() );

	send( t );
}

void ConferenceTask::declineConference( const QString &room, const QStringList &members, const QString &msg )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfDecline);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
		t->setParam( 3, (*it).toLocal8Bit() );
	t->setParam( 57, room.toLocal8Bit() );	
	t->setParam( 14, msg.toUtf8() );
	t->setParam( 97, 1 );

	send( t );
}
void ConferenceTask::leaveConference( const QString &room, const QStringList &members )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfLogoff);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
		t->setParam( 3, (*it).toLocal8Bit() );
	t->setParam( 57, room.toLocal8Bit() );

	send( t );
}

void ConferenceTask::sendMessage( const QString &room, const QStringList &members, const QString &msg )
{
	kDebug(YAHOO_RAW_DEBUG) ;

	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceConfMsg);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	for( QStringList::const_iterator it = members.begin(); it != members.end(); ++it )
		t->setParam( 53, (*it).toLocal8Bit() );
	t->setParam( 57, room.toLocal8Bit() );
	t->setParam( 14, msg.toUtf8() );
	t->setParam( 97, 1 );

	send( t );
}
#include "conferencetask.moc"
