
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
	addIrcMethod("001",	&KIRC::numericReply_001,	1,	1);

	/* Gives information about the host, close to 004 (See case 004) in the form of:
	 * ":Your host is <servername>, running version <ver>" */
	addIrcMethod("002",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString,		1,	1));

	/* Gives the date that this server was created (useful for determining the uptime of the server) in the form of:
	 * "This server was created <date>" */
	addIrcMethod("003",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString,		1,	1));
	addIrcMethod("004",	&KIRC::numericReply_004,	5,	5);

	/* NOT IN RFC1459 NOR RFC2812
	 * Tells connections statistics about the server for the uptime activity:
	 * ":Highest connection count: <integer> (<integer> clients) (<integer> since server was (re)started)"
	 * This is not the only form of this message ignoring it for now.
	 * FIXME: Implement me */
	addIrcMethod("250",	new KIRCMethodFunctor_Ignore());

	/* Tells how many user there are on all the different servers in the form of:
	 * ":There are <integer> users and <integer> services on <integer> servers" */
	addIrcMethod("251",	new KIRCMethodFunctor_Ignore());

	/* Issues a number of operators on the server in the form of:
	 * "<integer> :operator(s) online" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("252",	&KIRC::numericReply_252,	2,	2);

	/* Tells how many unknown connections the server has in the form of:
	 * "<integer> :unknown connection(s)" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("253",	&KIRC::numericReply_253,	2,	2);

	/* Tells how many total channels there are on this network in the form of:
	 * "<integer> :channels formed" */
	// FIXME: send an integer instead of a QString
	addIrcMethod("254",	&KIRC::numericReply_254,	2,	2);

	/* Tells how many clients and servers *this* server handles in the form of:
	 * ":I have <integer> clients and <integer> servers" */
	addIrcMethod("255",	new KIRCMethodFunctor_S_Suffix<KIRC>(this, &KIRC::incomingConnectString, 1, 1));

	//Server is too busy
	addIrcMethod("263",	new KIRCMethodFunctor_Empty<KIRC>(this, &KIRC::incomingServerLoadTooHigh));

	/* NOT IN RFC2812
	 * Tells statistics about the current local server state:
	 * ":Current local  users: <integer>  Max: <integer>" */
	addIrcMethod("265",	new KIRCMethodFunctor_Ignore());

	/* Tells statistics about the current global(the whole irc server chain) server state:
	 * ":Current global users: <integer>  Max: <integer>" */
	addIrcMethod("266",	new KIRCMethodFunctor_Ignore());

	/* "<nick> :<away message>"
	 */
	addIrcMethod("301",	new KIRCMethodFunctor_SS_Suffix<KIRC,1>(this, &KIRC::incomingUserIsAway, 2, 2));

	addIrcMethod("303",	&KIRC::numericReply_303,	1,	1);

	/* ":You are no longer marked as being away"
	 */
	addIrcMethod("305",	new KIRCMethodFunctor_Ignore());

	/* ":You have been marked as being away"
	 */
	addIrcMethod("306",	new KIRCMethodFunctor_Ignore());

	/* Indicates that this user is identified with NICSERV on DALNET */
	addIrcMethod("307",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsIdentified, 2, 2));

	/* Show info about a user (part of a /whois) in the form of:
	 * "<nick> <user> <host> * :<real name>" */
	addIrcMethod("311",	&KIRC::numericReply_311,	5,	5);

	/* Show info about a server (part of a /whois) in the form of:
	 * "<nick> <server> :<server info>" */
	addIrcMethod("312",	&KIRC::numericReply_312,	3,	3);

	/* Show info about an operator (part of a /whois) in the form of:
	 * "<nick> :is an IRC operator" */
	addIrcMethod("313",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsOperator, 2, 2));

	/* Show WHOWAS Info
	 * "<nick> <user> <host> * :<real name>" */
	addIrcMethod("314",	&KIRC::numericReply_314,	5,	5);

	addIrcMethod("315",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWho, 2, 2));

	addIrcMethod("317",	&KIRC::numericReply_317,	3,	4);

	/* Receive end of WHOIS in the form of
	 * "<nick> :End of /WHOIS list" */
	addIrcMethod("318",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWhois, 2, 2));

	addIrcMethod("319",	&KIRC::numericReply_319,	2,	2);

	/* Indicates that this user is identified with NICSERV on FREENODE */
	addIrcMethod("320",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWhoIsIdentified, 2, 2));

	/* RFC1459: "<channel> :Users  Name" ("Channel :Users  Name")
	 * RFC2812: Obsolete. Not used. */
	addIrcMethod("321",	new KIRCMethodFunctor_Ignore());

	/* Received one channel from the LIST command.
	 * "<channel> <# visible> :<topic>" */
	addIrcMethod("322",	&KIRC::numericReply_322,	3,	3);

	/* End of the LIST command.
	 * ":End of LIST" */
	addIrcMethod("323",	new KIRCMethodFunctor_Empty<KIRC>(this, &KIRC::incomingEndOfList));

	addIrcMethod("324",	&KIRC::numericReply_324,	2,	4);
	addIrcMethod("328",	&KIRC::numericReply_328,	2,	2);
	addIrcMethod("329",	&KIRC::numericReply_329,	3,	3);
	addIrcMethod("331",	&KIRC::numericReply_331,	2,	2);
	addIrcMethod("332",	&KIRC::numericReply_332,	2,	2);
	addIrcMethod("333",	&KIRC::numericReply_333,	4,	4);

	//WHO Reply
	addIrcMethod("352",	&KIRC::numericReply_352,	5,	10);

	//NAMES list
	addIrcMethod("353",	&KIRC::numericReply_353,	3,	3);

	/* Gives a signal to indicate that the NAMES list has ended for a certain channel in the form of:
	 * "<channel> :End of NAMES list" */
	addIrcMethod("366",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfNames, 2, 2));

	/* End of WHOWAS Request */
	addIrcMethod("369",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingEndOfWhoWas, 2, 2));

	/* Part of the MOTD.
	 * ":- <text>" */
	addIrcMethod("372",	&KIRC::numericReply_372,	1,	1);

	/* Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
	 * ":- <server> Message of the day - "
	 */
	addIrcMethod("375",	&KIRC::numericReply_375,	1,	1);
//	addIrcMethod("375",	KIRCMethodFunctor_S_Suffix(this, &KIRC::incomingStartOfMotd,	1,	1));

	/* End of the motd.
	 * ":End of MOTD command" */
	addIrcMethod("376",	&KIRC::numericReply_376,	1,	1);

	/* Gives a signal to indicate that the command issued failed because the person not being on IRC in the for of:
	 * "<nickname> :No such nick/channel"
	 *  - Used to indicate the nickname parameter supplied to a command is currently unused. */
	addIrcMethod("401",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingNoNickChan, 2, 2));

	addIrcMethod("404",	new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Like case 401, but when there *was* no such nickname
	 * "<nickname> :There was no such nickname" */
	addIrcMethod("406",	new KIRCMethodFunctor_S<KIRC, 1>(this, &KIRC::incomingWasNoNick, 2, 2));

	addIrcMethod("433",	&KIRC::numericReply_433,	2,	2);

	addIrcMethod("442",	new KIRCMethodFunctor_SS_Suffix<KIRC, 1>(this, &KIRC::incomingCannotSendToChannel, 2, 2));

	/* Bad server password */
	addIrcMethod("464",	&KIRC::numericReply_464,	1,	1);
	/* Channel is Full */
	addIrcMethod("471",	&KIRC::numericReply_471,	2,	2);
	/* Invite Only */
	addIrcMethod("473",	&KIRC::numericReply_473,	2,	2);
	/* Banned */
	addIrcMethod("474",	&KIRC::numericReply_474,	2,	2);
	/* Wrong Chan-key */
	addIrcMethod("475",	&KIRC::numericReply_475,	2,	2);
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
	/* Gives information about the servername, version, available modes, etc in the form of:
	 * "<servername> <version> <available user modes> <available channel modes>"
	 */
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
	m_motdBuffer.append( msg.suffix() );
	return true;
}

bool KIRC::numericReply_375(const KIRCMessage &)
{
	m_motdBuffer.clear();
	return true;
}

bool KIRC::numericReply_376(const KIRCMessage &)
{
	emit incomingMotd(m_motdBuffer);
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
