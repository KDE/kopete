
/*
    kircnumericreplies.cpp - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>

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

#include "kircfunctors.h"

#include "kirc.h"

void KIRC::registerNumericReplies()
{
//	Note: Numeric replies always have the current nick or *(when undefined) as first argmuent.
	addNumericIrcMethod( 1, &KIRC::numericReply_001, 1, 1);

	/* Gives information about the host, close to 004 (See case 004) in the form of:
	 * ":Your host is <servername>, running version <ver>" */
	addNumericIrcMethod( 2, new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString, 1, 1));

	/* Gives the date that this server was created (useful for determining the uptime of the server) in the form of:
	 * "This server was created <date>" */
	addNumericIrcMethod( 3, new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString, 1, 1));

	/* Gives information about the servername, version, available modes, etc in the form of:
	 * "<servername> <version> <available user modes> <available channel modes>"
	 */
	addNumericIrcMethod( 4, &KIRC::numericReply_004,	5,	5);

	/* NOT IN RFC1459 NOR RFC2812
	 * Tells connections statistics about the server for the uptime activity:
	 * ":Highest connection count: <integer> (<integer> clients) (<integer> since server was (re)started)"
	 * This is not the only form of this message ignoring it for now.
	 * FIXME: Implement me */
	addNumericIrcMethod( 250, KIRC::IgnoreMethod );

	/* Tells how many user there are on all the different servers in the form of:
	 * ":There are <integer> users and <integer> services on <integer> servers" */
	addNumericIrcMethod( 251, KIRC::IgnoreMethod );

	/* Issues a number of operators on the server in the form of:
	 * "<integer> :operator(s) online" */
	// FIXME: send an integer instead of a QString
	addNumericIrcMethod( 252, &KIRC::numericReply_252,	2,	2);

	/* Tells how many unknown connections the server has in the form of:
	 * "<integer> :unknown connection(s)" */
	// FIXME: send an integer instead of a QString
	addNumericIrcMethod( 253, &KIRC::numericReply_253,	2,	2);

	/* Tells how many total channels there are on this network in the form of:
	 * "<integer> :channels formed" */
	// FIXME: send an integer instead of a QString
	addNumericIrcMethod( 254, &KIRC::numericReply_254,	2,	2);

	/* Tells how many clients and servers *this* server handles in the form of:
	 * ":I have <integer> clients and <integer> servers" */
	addNumericIrcMethod( 255, new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString, 1, 1));

	//Server is too busy
	addNumericIrcMethod( 263, new KIRCMethodFunctor_Empty<KIRC>(this, &KIRC::incomingServerLoadTooHigh));

	/* NOT IN RFC2812
	 * Tells statistics about the current local server state:
	 * ":Current local  users: <integer>  Max: <integer>" */
	addNumericIrcMethod( 265, KIRC::IgnoreMethod );

	/* Tells statistics about the current global(the whole irc server chain) server state:
	 * ":Current global users: <integer>  Max: <integer>" */
	addNumericIrcMethod( 266, KIRC::IgnoreMethod );

	/* "<nick> :<away message>"
	 */
	addNumericIrcMethod( 301, new KIRCMethodFunctor_SS_Suffix<KIRC,1>(this, &KIRC::incomingUserIsAway, 2, 2));

	addNumericIrcMethod( 303, &KIRC::numericReply_303,	1,	1);

	/* ":You are no longer marked as being away"
	 */
	addNumericIrcMethod( 305, KIRC::IgnoreMethod );

	/* ":You have been marked as being away"
	 */
	addNumericIrcMethod( 306, KIRC::IgnoreMethod );

	/* DALNET: Indicates that this user is identified with NICSERV. */
	addNumericIrcMethod( 307, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsIdentified, 2, 2));

	/* Show info about a user (part of a /whois) in the form of:
	 * "<nick> <user> <host> * :<real name>" */
	addNumericIrcMethod( 311, &KIRC::numericReply_311,	5,	5);

	/* Show info about a server (part of a /whois) in the form of:
	 * "<nick> <server> :<server info>" */
	addNumericIrcMethod( 312, &KIRC::numericReply_312,	3,	3);

	/* Show info about an operator (part of a /whois) in the form of:
	 * "<nick> :is an IRC operator" */
	addNumericIrcMethod( 313, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsOperator, 2, 2));

	/* Show WHOWAS Info
	 * "<nick> <user> <host> * :<real name>" */
	addNumericIrcMethod( 314, &KIRC::numericReply_314,	5,	5);

	addNumericIrcMethod( 315, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWho, 2, 2));

	addNumericIrcMethod( 317, &KIRC::numericReply_317,	3,	4);

	/* Receive end of WHOIS in the form of
	 * "<nick> :End of /WHOIS list" */
	addNumericIrcMethod( 318, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWhois, 2, 2));

	addNumericIrcMethod( 319, &KIRC::numericReply_319,	2,	2);

	/* Indicates that this user is identified with NICSERV on FREENODE */
	addNumericIrcMethod( 320, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsIdentified, 2, 2));

	/* RFC1459: "<channel> :Users  Name" ("Channel :Users  Name")
	 * RFC2812: Obsolete. Not used. */
	addNumericIrcMethod( 321, KIRC::IgnoreMethod );

	/* Received one channel from the LIST command.
	 * "<channel> <# visible> :<topic>" */
	addNumericIrcMethod( 322, &KIRC::numericReply_322,	3,	3);

	/* End of the LIST command.
	 * ":End of LIST" */
	addNumericIrcMethod( 323, new KIRCMethodFunctor_Empty<KIRC>(this, &KIRC::incomingEndOfList));

	addNumericIrcMethod( 324, &KIRC::numericReply_324,	2,	4);

	addNumericIrcMethod( 328, &KIRC::numericReply_328,	2,	2);

	addNumericIrcMethod( 329, &KIRC::numericReply_329,	3,	3);

	addNumericIrcMethod( 331, &KIRC::numericReply_331,	2,	2);

	addNumericIrcMethod( 332, &KIRC::numericReply_332,	2,	2);

	addNumericIrcMethod( 333, &KIRC::numericReply_333,	4,	4);

	//WHO Reply
	addNumericIrcMethod( 352, &KIRC::numericReply_352,	5,	10);

	//NAMES list
	addNumericIrcMethod( 353, &KIRC::numericReply_353,	3,	3);

	/* Gives a signal to indicate that the NAMES list has ended for a certain channel in the form of:
	 * "<channel> :End of NAMES list" */
	addNumericIrcMethod( 366, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfNames, 2, 2));

	/* End of WHOWAS Request */
	addNumericIrcMethod( 369, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWhoWas, 2, 2));

	/* Part of the MOTD.
	 * ":- <text>" */
	addNumericIrcMethod( 372, &KIRC::numericReply_372,	1,	1);

	/* Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
	 * ":- <server> Message of the day - "
	 */
	addNumericIrcMethod( 375, KIRC::IgnoreMethod );

	/* End of the motd.
	 * ":End of MOTD command" */
	addNumericIrcMethod( 376, KIRC::IgnoreMethod );

	/* Gives a signal to indicate that the command issued failed because the person not being on IRC in the for of:
	 * "<nickname> :No such nick/channel"
	 *  - Used to indicate the nickname parameter supplied to a command is currently unused. */
	addNumericIrcMethod( 401, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingNoNickChan, 2, 2));

	addNumericIrcMethod( 404, new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Like case 401, but when there *was* no such nickname
	 * "<nickname> :There was no such nickname" */
	addNumericIrcMethod( 406, new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWasNoNick, 2, 2));

	addNumericIrcMethod( 433, &KIRC::numericReply_433,	2,	2);

	addNumericIrcMethod( 442, new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Bad server password
	 * ":Password Incorrect"
	 */
	addNumericIrcMethod( 464, &KIRC::numericReply_464,	1,	1);

	/* Channel is Full */
	addNumericIrcMethod( 471, &KIRC::numericReply_471,	2,	2);

	/* Invite Only */
	addNumericIrcMethod( 473, &KIRC::numericReply_473,	2,	2);

	/* Banned */
	addNumericIrcMethod( 474, &KIRC::numericReply_474,	2,	2);

	/* Wrong Chan-key */
	addNumericIrcMethod( 475, &KIRC::numericReply_475,	2,	2);

	/* DALNET:
	 * "<channel> :You need a registered nick to join that channel."
	 */
//	addNumericIrcMethod( 477, &KIRC::numericReply_477,	2,	2);
}

bool KIRC::numericReply_001(const KIRCMessage &msg)
{
	/* Gives a welcome message in the form of:
	 * "Welcome to the Internet Relay Network <nick>!<user>@<host>"
	 */
	if (m_FailedNickOnLogin == true)
	{
		// this is if we had a "Nickname in use" message when connecting and we set another nick.
		// This signal emits that the nick was accepted and we are now logged in
		emit successfullyChangedNick(m_Nickname, m_PendingNick);
		m_Nickname = m_PendingNick;
		m_FailedNickOnLogin = false;
	}

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	emit incomingConnectString(msg.suffix());
	setStatus(Connected);
	emit connectedToServer();

	return true;
}

bool KIRC::numericReply_004(const KIRCMessage &msg)
{
	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
	return true;
}

bool KIRC::numericReply_252(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_253(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_254(const KIRCMessage &msg)
{
	emit incomingConnectString( msg.toString() );
	return true;
}

bool KIRC::numericReply_265(const KIRCMessage &)
{
	return true;
}

bool KIRC::numericReply_266(const KIRCMessage &)
{
	return true;
}

bool KIRC::numericReply_303(const KIRCMessage &msg)
{
	/* ":*1<nick> *(" " <nick> )"
	 */
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).stripWhiteSpace().isEmpty())
			emit incomingUserOnline(*it);
	}
	return true;
}
/*
bool KIRC::numericReply_305(const KIRCMessage &msg)
{
	return true;
}

bool KIRC::numericReply_306(const KIRCMessage &msg)
{
	return true;
}
*/
bool KIRC::numericReply_311(const KIRCMessage &msg)
{
	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
	return true;
}

bool KIRC::numericReply_312(const KIRCMessage &msg)
{
	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
	return true;
}

bool KIRC::numericReply_314(const KIRCMessage &msg)
{
	emit incomingWhoWasUser( msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix() );
	return true;
}

bool KIRC::numericReply_317(const KIRCMessage &msg)
{
	/* RFC say: "<nick> <integer> :seconds idle"
	 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
	 * Show info about someone who is idle (part of a /whois) in the form of:
	 */
	emit incomingWhoIsIdle(msg.arg(1), msg.arg(2).toULong());
	if (msg.argsSize()==4)
		emit incomingSignOnTime(msg.arg(1),msg.arg(3).toULong());
	return true;
}

bool KIRC::numericReply_319(const KIRCMessage &msg)
{
	/* Show info a channel a user is logged in (part of a /whois) in the form of:
	 * "<nick> :{[@|+]<channel><space>}"
	 */
	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
	return true;
}

bool KIRC::numericReply_322(const KIRCMessage &msg)
{
	/* "<channel> <# visible> :<topic>"
	 */
	//kdDebug(14120) << k_funcinfo << "Listed " << msg.arg(1) << endl;

	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
	return true;
}

bool KIRC::numericReply_324(const KIRCMessage &msg)
{
	/* "<channel> <mode> <mode params>"
	 */
	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
	return true;
}

bool KIRC::numericReply_328(const KIRCMessage &msg)
{
	/* "<channel> <mode> <mode params>"
	 */
	kdDebug(14120) << k_funcinfo << endl;
	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
	return true;
}

bool KIRC::numericReply_329( const KIRCMessage & /* msg */ )
{
	/* NOT IN RFC1459 NOR RFC2812
	 * "%s %lu"
	 * FIXME: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
	 */
	return true;
}

bool KIRC::numericReply_331( const KIRCMessage & /* msg */ )
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :No topic is set"
	 */
//	emit incomingExistingTopic(msg.arg(1), suffix);
	return true;
}

bool KIRC::numericReply_332(const KIRCMessage &msg)
{
	/* Gives the existing topic for a channel after a join.
	 * "<channel> :<topic>"
	 */
	emit incomingExistingTopic(msg.arg(1), msg.suffix());
	return true;
}

bool KIRC::numericReply_333( const KIRCMessage &msg )
{
	/* Gives the nickname and time who changed the topic
	 */
	 kdDebug(14120) << k_funcinfo << endl;
	 QDateTime d;
	 d.setTime_t( msg.arg(3).toLong() );
	 emit incomingTopicUser( msg.arg(1), msg.arg(2), d );
	 return true;
}

bool KIRC::numericReply_352(const KIRCMessage &msg)
{
	QStringList suffix = QStringList::split( ' ', msg.suffix() );

	emit incomingWhoReply(
		msg.arg(5),
		msg.arg(1),
		msg.arg(2),
		msg.arg(3),
		msg.arg(4),
		msg.arg(6)[0] != 'H',
		msg.arg(7),
		suffix[0].toUInt(),
		suffix[1]
	);

	return true;
}

bool KIRC::numericReply_353(const KIRCMessage &msg)
{
	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
	return true;
}

bool KIRC::numericReply_372(const KIRCMessage &msg)
{
	emit incomingMotd( msg.suffix() );
	return true;
}

bool KIRC::numericReply_433(const KIRCMessage &msg)
{
	/* "<nick> :Nickname is already in use"
	 * Tells us that our nickname is already in use.
	 */
	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
		m_FailedNickOnLogin = true;
		emit incomingFailedNickOnLogin(msg.arg(1));
	}
	else
	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
		emit incomingNickInUse(msg.arg(1));
	}


	return true;
}

bool KIRC::numericReply_464(const KIRCMessage &/*msg*/)
{
	/* Server need pass.. Call disconnect */
	emit incomingFailedServerPassword();
	return true;
}

bool KIRC::numericReply_471(const KIRCMessage &msg)
{
	emit incomingFailedChanFull(msg.arg(1));
	return true;
}

bool KIRC::numericReply_473(const KIRCMessage &msg)
{
	emit incomingFailedChanInvite(msg.arg(1));
	return true;
}

bool KIRC::numericReply_474(const KIRCMessage &msg)
{
	emit incomingFailedChanBanned(msg.arg(1));
	return true;
}

bool KIRC::numericReply_475(const KIRCMessage &msg)
{
	emit incomingFailedChankey(msg.arg(1));
	return true;
}
/*
bool KIRC::numericReply_477(const KIRCMessage &msg)
{
	emit incomingChannelNeedRegistration(msg.arg(2), msg.suffix());
	return true;
}
*/
