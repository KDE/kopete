
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

#include "kircengine.h"

#include <qtimer.h>

using namespace KIRC;

/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */

void Engine::bindNumericReplies()
{
	bind(1, this, SLOT(numericReply_001(const KIRC::Message &)), 1, 1);
//	bind(2, this, SLOT(numericReply_002(const KIRC::Message &)), 1, 1); // incomingConnectString
//	bind(3, this, SLOT(numericReply_003(const KIRC::Message &)), 1, 1); // incomingConnectString
	bind(4, this, SLOT(numericReply_004(const KIRC::Message &)), 5, 5);

//	bind(250, this, SLOT(numericReply_250(const KIRC::Message &)));
//	bind(251, this, SLOT(numericReply_251(const KIRC::Message &)));
//	bind(252, this, SLOT(numericReply_252(const KIRC::Message &)), 2, 2);
//	bind(253, this, SLOT(numericReply_253(const KIRC::Message &)), 2, 2);
//	bind(254, this, SLOT(numericReply_254(const KIRC::Message &)), 2, 2);
//	bind(255, this, SLOT(numericReply_255(const KIRC::Message &)), 1, 1); // incomingConnectString

//	bind(263, this, SLOT(numericReply_263(const KIRC::Message &))); // incomingServerLoadTooHigh
//	bind(265, this, SLOT(numericReply_265(const KIRC::Message &)));
//	bind(266, this, SLOT(numericReply_266(const KIRC::Message &)));

//	bind(301, this, SLOT(numericReply_301(const KIRC::Message &)), 2, 2); // incomingUserIsAway
//	bind(303, this, SLOT(numericReply_303(const KIRC::Message &)), 1, 1);
//	bind(305, Engine::IgnoreMethod );
//	bind(306, Engine::IgnoreMethod );
//	bind(307, this, SLOT(numericReply_307(const KIRC::Message &)), 2, 2); // incomingWhoIsIdentified
	bind(311, this, SLOT(numericReply_311(const KIRC::Message &)), 5, 5);
	bind(312, this, SLOT(numericReply_312(const KIRC::Message &)), 3, 3);
//	bind(313, this, SLOT(numericReply_313(const KIRC::Message &)), 2, 2); // incomingWhoIsOperator
	bind(314, this, SLOT(numericReply_314(const KIRC::Message &)), 5, 5);
//	bind(315, this, SLOT(numericReply_315(const KIRC::Message &)), 2, 2); //incomingEndOfWho
	bind(317, this, SLOT(numericReply_317(const KIRC::Message &)), 3, 4);
//	bind(318, this, SLOT(numericReply_318(const KIRC::Message &)), 2, 2); // incomingEndOfWhois
	bind(319, this, SLOT(numericReply_319(const KIRC::Message &)), 2, 2);
//	bind(320, this, SLOT(numericReply_320(const KIRC::Message &)), 2, 2); // incomingWhoIsIdentified
//	bind(321, Engine::IgnoreMethod );
	bind(322, this, SLOT(numericReply_322(const KIRC::Message &)), 3, 3);
//	bind(323, this, SLOT(numericReply_323(const KIRC::Message &)), 1, 1); // incomingEndOfList
	bind(324, this, SLOT(numericReply_324(const KIRC::Message &)), 2, 4);
	bind(328, this, SLOT(numericReply_328(const KIRC::Message &)), 2, 2);
	bind(329, this, SLOT(numericReply_329(const KIRC::Message &)), 3, 3);
	bind(331, this, SLOT(numericReply_331(const KIRC::Message &)), 2, 2);
	bind(332, this, SLOT(numericReply_332(const KIRC::Message &)), 2, 2);
	bind(333, this, SLOT(numericReply_333(const KIRC::Message &)), 4, 4);
	bind(352, this, SLOT(numericReply_352(const KIRC::Message &)), 5, 10);
	bind(353, this, SLOT(numericReply_353(const KIRC::Message &)), 3, 3);
//	bind(366, this, SLOT(numericReply_366(const KIRC::Message &)), 2, 2); // incomingEndOfNames
//	bind(369, this, SLOT(numericReply_369(const KIRC::Message &)), 2, 2); // incomingEndOfWhoWas
	bind(372, this, SLOT(numericReply_372(const KIRC::Message &)), 1, 1);
//	bind(375, Engine::IgnoreMethod );
//	bind(376, Engine::IgnoreMethod );

//	bind(401, this, SLOT(numericReply_401(const KIRC::Message &)), 2, 2); // incomingNoNickChan
//	bind(404, this, SLOT(numericReply_404(const KIRC::Message &)), 2, 2); // incomingCannotSendToChannel
//	bind(406, this, SLOT(numericReply_406(const KIRC::Message &)), 2, 2); // incomingWasNoNick
	bind(433, this, SLOT(numericReply_433(const KIRC::Message &)), 2, 2);
//	bind(442, this, SLOT(numericReply_442(const KIRC::Message &)), 2, 2); // incomingCannotSendToChannel
	bind(464, this, SLOT(numericReply_464(const KIRC::Message &)), 1, 1);
	bind(471, this, SLOT(numericReply_471(const KIRC::Message &)), 2, 2);
	bind(473, this, SLOT(numericReply_473(const KIRC::Message &)), 2, 2);
	bind(474, this, SLOT(numericReply_474(const KIRC::Message &)), 2, 2);
	bind(475, this, SLOT(numericReply_475(const KIRC::Message &)), 2, 2);
//	bind(477, this, SLOT(numericReply_477(const KIRC::Message &)), 2, 2);
}

/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void Engine::numericReply_001(const Message &msg)
{
	kdDebug(14121) << k_funcinfo << endl;

	if (m_FailedNickOnLogin)
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
	m_connectTimer->stop();

	setStatus(Connected);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given informations are close to 004.
 */
// void Engine::numericReply_002(const Message &msg)
// {
// }

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
// void Engine::numericReply_003(const Message &msg)
// {
// }

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void Engine::numericReply_004(const Message &msg)
{
	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void Engine::numericReply_252(const Message &msg)
{
	// FIXME: send an integer instead of a QString
	emit incomingConnectString( msg.toString() );
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void Engine::numericReply_253(const Message &msg)
{
	// FIXME: send an integer instead of a QString
	emit incomingConnectString( msg.toString() );
}

/* Tells how many total channels there are on this network in the form of:
 * "<integer> :channels formed" */
void Engine::numericReply_254(const Message &msg)
{
	// FIXME: send an integer instead of a QString
	emit incomingConnectString( msg.toString() );
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
// void Engine::numericReply_255(const Message &)
// {
// }

/* 263:
 * Server is too busy.
 */

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
// void Engine::numericReply_265(const Message &)
// {
// }

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
// void Engine::numericReply_266(const Message &)
// {
// }

/* 301: "<nick> :<away message>"
 */
// void Engine::numericReply_301(const Message &)
// {
// }

/* 303: ":*1<nick> *(" " <nick> )"
 */
void Engine::numericReply_303(const Message &msg)
{
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).stripWhiteSpace().isEmpty())
			emit incomingUserOnline(*it);
	}
}

/* 305: ":You are no longer marked as being away"
 */
// void Engine::numericReply_305(const Message &msg)
// {
// }


/* 306: ":You have been marked as being away"
 */
// void Engine::numericReply_306(const Message &msg)
// {
// }

/* 307:
 * DALNET: Indicates that this user is identified with NICSERV.
 */
// void Engine::numericReply_307(const Message &msg)
// {
// }

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void Engine::numericReply_311(const Message &msg)
{
	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void Engine::numericReply_312(const Message &msg)
{
	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void Engine::numericReply_314(const Message &msg)
{
	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

void Engine::numericReply_317(const Message &msg)
{
	/* RFC say: "<nick> <integer> :seconds idle"
	 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
	 * Show info about someone who is idle (part of a /whois) in the form of:
	 */
	emit incomingWhoIsIdle(msg.arg(1), msg.arg(2).toULong());
	if (msg.argsSize()==4)
		emit incomingSignOnTime(msg.arg(1),msg.arg(3).toULong());
}

/* 318: "<nick> :End of /WHOIS list"
 * End of WHOIS for a given nick.
 */
// void Engine::numericReply_218(const Message &)
// {
// }

void Engine::numericReply_319(const Message &msg)
{
	/* Show info a channel a user is logged in (part of a /whois) in the form of:
	 * "<nick> :{[@|+]<channel><space>}"
	 */
	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
void Engine::numericReply_322(const Message &msg)
{
	//kdDebug(14120) << k_funcinfo << "Listed " << msg.arg(1) << endl;

	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
// void Engine::numericReply_323(const Message &)
// {
// }

/* 324: "<channel> <mode> <mode params>"
 */
void Engine::numericReply_324(const Message &msg)
{
	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328: "<channel> <mode> <mode params>"
 */
void Engine::numericReply_328(const Message &msg)
{
	kdDebug(14120) << k_funcinfo << endl;
	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void Engine::numericReply_329( const Message &)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void Engine::numericReply_331( const Message &)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void Engine::numericReply_332(const Message &msg)
{
	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void Engine::numericReply_333( const Message &msg )
{
	kdDebug(14120) << k_funcinfo << endl;
	QDateTime d;
	d.setTime_t( msg.arg(3).toLong() );
	emit incomingTopicUser( msg.arg(1), msg.arg(2), d );
}

/* 352:
 * WHO Reply
 */
void Engine::numericReply_352(const Message &msg)
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
}


/* 353:
 * NAMES list
 */
void Engine::numericReply_353(const Message &msg)
{
	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */

/* 369:
 * End of WHOWAS Request
 */

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void Engine::numericReply_372(const Message &msg)
{
	emit incomingMotd( msg.suffix() );
}

/* 375: ":- <server> Message of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */

/* 376: ":End of MOTD command"
 * End of the motd.
 */

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void Engine::numericReply_433(const Message &msg)
{
	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
		m_FailedNickOnLogin = true;
		m_connectTimer->stop();
		emit incomingFailedNickOnLogin(msg.arg(1));
	}
	else
	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
		emit incomingNickInUse(msg.arg(1));
	}
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void Engine::numericReply_464(const Message &/*msg*/)
{
	/* Server need pass.. Call disconnect*/
	emit incomingFailedServerPassword();
}

/* 471:
 * Channel is Full
 */
void Engine::numericReply_471(const Message &msg)
{
	emit incomingFailedChanFull(msg.arg(1));
}

/* 473:
 * Invite Only.
 */
void Engine::numericReply_473(const Message &msg)
{
	emit incomingFailedChanInvite(msg.arg(1));
}

/* 474:
 * Banned.
 */
void Engine::numericReply_474(const Message &msg)
{
	emit incomingFailedChanBanned(msg.arg(1));
}

/* 475:
 * Wrong Chan-key.
 */
void Engine::numericReply_475(const Message &msg)
{
	emit incomingFailedChankey(msg.arg(1));
}

/* 477: "<channel> :You need a registered nick to join that channel."
 * Available on DALNET servers only ?
 */
// void Engine::numericReply_477(const Message &msg)
// {
// 	emit incomingChannelNeedRegistration(msg.arg(2), msg.suffix());
// }
