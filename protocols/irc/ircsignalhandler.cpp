
/*
    ircsignalhandler.cpp - Maps signals from the IRC engine to contacts

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircusercontact.h"
#include "ircchannelcontact.h"
#include "ircsignalhandler.h"

#include "kircengine.h"

IRCSignalHandler::IRCSignalHandler(IRCContactManager *m)
	: QObject(m),
	  manager(m)
{
	KIRC::Engine *m_engine = static_cast<IRCAccount*>( manager->mySelf()->account() )->engine();

	//Channel Connections to ourself
	QObject::connect(m_engine, SIGNAL(incomingNamesList(const QString &, const QStringList &)),
		this, SLOT(slotNamesList(const QString &, const QStringList &)));

	QObject::connect(m_engine, SIGNAL(incomingEndOfNames(const QString &)),
		this, SLOT(slotEndOfNames(const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingTopicUser(const QString &, const QString &, const QDateTime &)),
		this, SLOT(slotTopicUser(const QString&,const QString&,const QDateTime&)));

	//Channel String mappings
	map<IRCChannelContact>( m, SIGNAL(incomingFailedChankey(const QString &)),
		&IRCChannelContact::failedChankey );

	map<IRCChannelContact>( m, SIGNAL(incomingFailedChanFull(const QString &)),
		&IRCChannelContact::failedChanInvite );

	map<IRCChannelContact>( m, SIGNAL(incomingFailedChanInvite(const QString &)),
		&IRCChannelContact::failedChanInvite );

	map<IRCChannelContact>( m, SIGNAL(incomingFailedChanBanned(const QString &)),
		&IRCChannelContact::failedChanBanned );

	mapSingle<IRCChannelContact>( m, SIGNAL(incomingJoinedChannel(const QString &, const QString &)),
		&IRCChannelContact::userJoinedChannel );

	mapSingle<IRCChannelContact>( m, SIGNAL(incomingExistingTopic(const QString &, const QString &)),
		&IRCChannelContact::channelTopic );

	mapSingle<IRCChannelContact>( m, SIGNAL(incomingChannelHomePage(const QString &, const QString &)),
		&IRCChannelContact::channelHomePage );

	mapDouble<IRCChannelContact>( m,
		SIGNAL(incomingPartedChannel(const QString &, const QString &,const QString &)),
		&IRCChannelContact::userPartedChannel );

	mapDouble<IRCChannelContact>( m,
		SIGNAL(incomingTopicChange(const QString &, const QString &,const QString &)),
		&IRCChannelContact::topicChanged );

	mapDouble<IRCChannelContact>( m,
		SIGNAL(incomingChannelModeChange(const QString &, const QString &,const QString &)),
		&IRCChannelContact::incomingModeChange );

	mapDouble<IRCChannelContact>( m,
		SIGNAL(incomingChannelMode(const QString &, const QString &,const QString &)),
		&IRCChannelContact::incomingChannelMode );

	mapTriple<IRCChannelContact>( m,
		SIGNAL(incomingKick(const QString &, const QString &,const QString &,const QString &)),
		&IRCChannelContact::userKicked );

	//User connections to ourself
	QObject::connect(m_engine, SIGNAL(incomingWhoIsIdle(const QString &, unsigned long )),
			this, SLOT(slotNewWhoIsIdle(const QString &, unsigned long )));

	QObject::connect(m_engine, SIGNAL(incomingWhoReply(const QString &, const QString &, const QString &,
		const QString &, const QString &, bool, const QString &, uint, const QString & )),
		this, SLOT( slotNewWhoReply(const QString &, const QString &, const QString &, const QString &,
		const QString &, bool, const QString &, uint, const QString &)));

	//User signal mappings
	map<IRCUserContact>( m, SIGNAL(incomingUserOnline( const QString & )), &IRCUserContact::userOnline );

	map<IRCUserContact>( m, SIGNAL(incomingWhoIsOperator( const QString & )), &IRCUserContact::newWhoIsOperator );

	map<IRCUserContact>( m, SIGNAL(incomingWhoIsIdentified( const QString & )), &IRCUserContact::newWhoIsIdentified );

	map<IRCUserContact>( m, SIGNAL(incomingEndOfWhois( const QString & )), &IRCUserContact::whoIsComplete );

	map<IRCUserContact>( m, SIGNAL(incomingEndOfWhoWas( const QString & )), &IRCUserContact::whoWasComplete );

	mapSingle<IRCUserContact>( m, SIGNAL(incomingUserIsAway( const QString &, const QString & )),
		&IRCUserContact::incomingUserIsAway );

	mapSingle<IRCUserContact>( m, SIGNAL(incomingWhoIsChannels( const QString &, const QString & )),
		&IRCUserContact::newWhoIsChannels );

	mapDouble<IRCUserContact>( m,
		SIGNAL(incomingWhoIsServer(const QString &, const QString &, const QString &)),
		&IRCUserContact::newWhoIsServer );

	mapDouble<IRCUserContact>( m,
		SIGNAL(incomingPrivAction(const QString &, const QString &, const QString &)),
		&IRCUserContact::newAction );

	mapDouble<IRCChannelContact>( m,
		SIGNAL(incomingAction(const QString &, const QString &, const QString &)),
		&IRCChannelContact::newAction );

	mapTriple<IRCUserContact>( m,
		SIGNAL(incomingWhoIsUser(const QString &, const QString &, const QString &, const QString &)),
		&IRCUserContact::newWhoIsUser );

	mapTriple<IRCUserContact>( m,
		SIGNAL(incomingWhoWasUser(const QString &, const QString &, const QString &, const QString &)),
		&IRCUserContact::newWhoIsUser );
}

IRCSignalHandler::~IRCSignalHandler()
{
	//Delete our mapping pointers
	for( QValueList<IRCSignalMappingBase*>::iterator it = mappings.begin(); it != mappings.end(); ++it )
		delete *it;
}

void IRCSignalHandler::slotNamesList( const QString &chan, const QStringList &list )
{
	IRCChannelContact *c = manager->existChannel( chan );
	if( c )
		c->namesList( list );
}

void IRCSignalHandler::slotEndOfNames( const QString &chan )
{
	IRCChannelContact *c = manager->existChannel( chan );
	if ( c )
		c->endOfNames();
}

void IRCSignalHandler::slotTopicUser(const QString &chan, const QString &user,const QDateTime &time)
{
	IRCChannelContact *c = manager->existChannel( chan );
	if( c )
		c->topicUser( user, time );
}

void IRCSignalHandler::slotNewWhoIsIdle(const QString &nick, unsigned long val )
{
	IRCUserContact *c = manager->findUser( nick );
	if( c )
		c->newWhoIsIdle( val );
}

void IRCSignalHandler::slotNewWhoReply(const QString &nick, const QString &arg1, const QString &arg2,
	const QString &arg3, const QString &arg4, bool arg5, const QString &arg6, uint arg7, const QString &arg8 )
{
	IRCUserContact *c = manager->findUser( nick );
	if( c )
		c->newWhoReply( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 );
}

#include "ircsignalhandler.moc"
