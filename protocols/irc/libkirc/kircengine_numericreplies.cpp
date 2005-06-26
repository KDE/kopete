/*
    kircnumericreplies.cpp - IRC Client

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

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

#include <kdebug.h>
#include <klocale.h>

#include <qdatetime.h>

using namespace KIRC;

/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */

void Engine::bindNumericReplies()
{
	bind(1, this, SLOT(numericReply_001(KIRC::Message &)), 1, 1);
	bind(2, this, SLOT(numericReply_002(KIRC::Message &)), 1, 1);
	bind(3, this, SLOT(numericReply_003(KIRC::Message &)), 1, 1);
	bind(4, this, SLOT(numericReply_004(KIRC::Message &)), 5, 5);
	bind(5, this, SLOT(numericReply_004(KIRC::Message &)), 1, 1);

	bind(250, this, SLOT(numericReply_250(KIRC::Message &)));
	bind(251, this, SLOT(numericReply_251(KIRC::Message &)));
	bind(252, this, SLOT(numericReply_252(KIRC::Message &)), 2, 2);
	bind(253, this, SLOT(numericReply_253(KIRC::Message &)), 2, 2);
	bind(254, this, SLOT(numericReply_254(KIRC::Message &)), 2, 2);
	bind(255, this, SLOT(numericReply_255(KIRC::Message &)), 1, 1);

	bind(263, this, SLOT(numericReply_263(KIRC::Message &))); // incomingServerLoadTooHigh
	bind(265, this, SLOT(numericReply_265(KIRC::Message &)));
	bind(266, this, SLOT(numericReply_266(KIRC::Message &)));

	bind(301, this, SLOT(numericReply_301(KIRC::Message &)), 2, 2);
	bind(303, this, SLOT(numericReply_303(KIRC::Message &)), 1, 1);
	bind(305, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 ); // You are no longer marked as away
	bind(306, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 ); // You are marked as away
	bind(307, this, SLOT(numericReply_307(KIRC::Message &)), 1, 1);
	bind(311, this, SLOT(numericReply_311(KIRC::Message &)), 5, 5);
	bind(312, this, SLOT(numericReply_312(KIRC::Message &)), 3, 3);
	bind(313, this, SLOT(numericReply_313(KIRC::Message &)), 2, 2);
	bind(314, this, SLOT(numericReply_314(KIRC::Message &)), 5, 5);
	bind(315, this, SLOT(numericReply_315(KIRC::Message &)), 2, 2);
	bind(317, this, SLOT(numericReply_317(KIRC::Message &)), 3, 4);
	bind(318, this, SLOT(numericReply_318(KIRC::Message &)), 2, 2);
	bind(319, this, SLOT(numericReply_319(KIRC::Message &)), 2, 2);
	bind(320, this, SLOT(numericReply_320(KIRC::Message &)), 2, 2);
	bind(321, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(KIRC::Message &)), 3, 3);
	bind(323, this, SLOT(numericReply_323(KIRC::Message &)), 1, 1);
	bind(324, this, SLOT(numericReply_324(KIRC::Message &)), 2, 4);
	bind(328, this, SLOT(numericReply_328(KIRC::Message &)), 2, 2);
	bind(329, this, SLOT(numericReply_329(KIRC::Message &)), 3, 3);
	bind(330, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0); // ???
	bind(331, this, SLOT(numericReply_331(KIRC::Message &)), 2, 2);
	bind(332, this, SLOT(numericReply_332(KIRC::Message &)), 2, 2);
	bind(333, this, SLOT(numericReply_333(KIRC::Message &)), 4, 4);
	bind(352, this, SLOT(numericReply_352(KIRC::Message &)), 5, 10);
	bind(353, this, SLOT(numericReply_353(KIRC::Message &)), 3, 3);
	bind(366, this, SLOT(numericReply_366(KIRC::Message &)), 2, 2);
	bind(369, this, SLOT(numericReply_369(KIRC::Message &)), 2, 2);
	bind(372, this, SLOT(numericReply_372(KIRC::Message &)), 1, 1);
	bind(375, this, SLOT(ignoreMessage(KIRC::Message&)), 0, 0 );
	bind(376, this, SLOT(ignoreMessage(KIRC::Message&)), 0, 0 );

	bind(401, this, SLOT(numericReply_401(KIRC::Message &)), 2, 2); // incomingNoNickChan
//	bind(404, this, SLOT(numericReply_404(KIRC::Message &)), 2, 2); // incomingCannotSendToChannel
	bind(406, this, SLOT(numericReply_406(KIRC::Message &)), 2, 2); // incomingWasNoNick
	bind(422, this, SLOT(numericReply_422(KIRC::Message &)), 1, 1);
	bind(433, this, SLOT(numericReply_433(KIRC::Message &)), 2, 2);
//	bind(442, this, SLOT(numericReply_442(KIRC::Message &)), 2, 2); // incomingCannotSendToChannel
	bind(464, this, SLOT(numericReply_464(KIRC::Message &)), 1, 1);
	bind(471, this, SLOT(numericReply_471(KIRC::Message &)), 2, 2);
	bind(473, this, SLOT(numericReply_473(KIRC::Message &)), 2, 2);
	bind(474, this, SLOT(numericReply_474(KIRC::Message &)), 2, 2);
	bind(475, this, SLOT(numericReply_475(KIRC::Message &)), 2, 2);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(KIRC::Message&)),0,0);
}

/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void Engine::numericReply_001(Message &msg)
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
	receivedServerMessage(msg);

	setConnectionState(Connected);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given informations are close to 004.
 */
void Engine::numericReply_002(Message &msg)
{
	receivedServerMessage(msg);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void Engine::numericReply_003(Message &msg)
{
	receivedServerMessage(msg);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void Engine::numericReply_004(Message &msg)
{
//	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void Engine::numericReply_005(Message &msg)
{
	receivedServerMessage(msg);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void Engine::numericReply_250(Message &msg)
{
	receivedServerMessage(msg);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void Engine::numericReply_251(Message &msg)
{
	receivedServerMessage(msg);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void Engine::numericReply_252(Message &msg)
{
	receivedServerMessage(msg, i18n("There are %1 operators online").arg(msg.arg(1)));
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void Engine::numericReply_253(Message &msg)
{
	receivedServerMessage(msg, i18n("There are %1 unknown connections").arg(msg.arg(1)));
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void Engine::numericReply_254(Message &msg)
{
	receivedServerMessage(msg, i18n("There are %1 channel formed").arg(msg.arg(1)));
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void Engine::numericReply_255(Message &msg)
{
	receivedServerMessage(msg);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void Engine::numericReply_263(Message &)
{
	emit incomingServerLoadTooHigh();
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void Engine::numericReply_265(Message &msg)
{
	receivedServerMessage(msg);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void Engine::numericReply_266(Message &msg)
{
	receivedServerMessage(msg);
}

/* 301: "<nick> :<away message>"
 */
void Engine::numericReply_301(Message &msg)
{
	emit incomingUserIsAway(msg.arg(1), msg.suffix());
}

/* 303: ":*1<nick> *(" " <nick> )"
 */
void Engine::numericReply_303(Message &msg)
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
// void Engine::numericReply_305(Message &msg)
// {
// }


/* 306: ":You have been marked as being away"
 */
// void Engine::numericReply_306(Message &msg)
// {
// }

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
void Engine::numericReply_307(Message &msg)
{
//	emit incomingWhoiIsUserNickIsRegistered(msg.arg(1));
}

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void Engine::numericReply_311(Message &msg)
{
	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void Engine::numericReply_312(Message &msg)
{
	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void Engine::numericReply_313(Message &msg)
{
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void Engine::numericReply_314(Message &msg)
{
	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

void Engine::numericReply_315(Message &msg)
{
	emit incomingEndOfWho(msg.arg(1));
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void Engine::numericReply_317(Message &msg)
{
	emit incomingWhoIsIdle(msg.arg(1), msg.arg(2).toULong());
	if (msg.argsSize()==4)
		emit incomingSignOnTime(msg.arg(1),msg.arg(3).toULong());
}

/* 318: "<nick>{<space><realname>} :End of /WHOIS list"
 * End of WHOIS for a given nick.
 */
void Engine::numericReply_318(Message &msg)
{
	emit incomingEndOfWhois(msg.arg(1));
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void Engine::numericReply_319(Message &msg)
{
	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void Engine::numericReply_320(Message &msg)
{
	emit incomingWhoIsIdentified(msg.arg(1));
}

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
void Engine::numericReply_322(Message &msg)
{
	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void Engine::numericReply_323(Message &msg)
{
	emit receivedServerMessage(msg);
}

/* 324: "<channel> <mode> <mode params>"
 */
void Engine::numericReply_324(Message &msg)
{
	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328: "<channel> <mode> <mode params>"
 */
void Engine::numericReply_328(Message &msg)
{
	kdDebug(14120) << k_funcinfo << endl;
	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void Engine::numericReply_329( Message &)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void Engine::numericReply_331( Message &)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void Engine::numericReply_332(Message &msg)
{
	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void Engine::numericReply_333( Message &msg )
{
/*
	QDateTime d;
	d.setTime_t( msg.arg(3).toLong() );
	emit incomingTopicUser(msg.arg(1), msg.arg(2), d );
*/
}

/* 352:
 * WHO Reply
 */
void Engine::numericReply_352(Message &msg)
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
		msg.suffix().section(' ', 0, 1 ).toUInt(),
		msg.suffix().section(' ', 1 )
	);
}


/* 353:
 * NAMES list
 */
void Engine::numericReply_353(Message &msg)
{
	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void Engine::numericReply_366(Message &msg)
{
	emit incomingEndOfNames(msg.arg(1));
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void Engine::numericReply_369(Message &msg)
{
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void Engine::numericReply_372(Message &msg)
{
	emit incomingMotd(msg.suffix());
}

/* 375: ":- <server> Message of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void Engine::numericReply_376(Message &msg)
{
//	emit incomingMotd(msg.suffix());
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void Engine::numericReply_401(Message &msg)
{
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void Engine::numericReply_406(Message &msg)
{
	#warning FIXME 406 MEANS *NEVER*, even before
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 422: ":MOTD File is missing"
 *
 * Server's MOTD file could not be opened by the server.
 */
void Engine::numericReply_422(Message &msg)
{
	receivedServerMessage(msg);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void Engine::numericReply_433(Message &msg)
{
//	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
		m_FailedNickOnLogin = true;
		emit incomingFailedNickOnLogin(msg.arg(1));
	}
//	else
//	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
//		emit incomingNickInUse(msg.arg(1));
//	}
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void Engine::numericReply_464(Message &/*msg*/)
{
	/* Server need pass.. Call disconnect*/
	emit incomingFailedServerPassword();
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void Engine::numericReply_471(Message &msg)
{
	emit incomingFailedChanFull(msg.arg(1));
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void Engine::numericReply_473(Message &msg)
{
	emit incomingFailedChanInvite(msg.arg(1));
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void Engine::numericReply_474(Message &msg)
{
	emit incomingFailedChanBanned(msg.arg(1));
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void Engine::numericReply_475(Message &msg)
{
	emit incomingFailedChankey(msg.arg(1));
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void Engine::numericReply_477(Message &msg)
// {
// 	emit incomingChannelNeedRegistration(msg.arg(2), msg.suffix());
// }

/* 478: "<channel> <char> :Channel list is full"
 */

/* 481: ":Permission Denied- You're not an IRC operator"
 */

/* 482: "<channel> :You're not channel operator"
 */

/* 483: ":You can't kill a server!"
 */

/* 484: ":Your connection is restricted!"
 */

/* 485: ":You're not the original channel operator"
 */

/* 491: ":No O-lines for your host"
 */

/* 501: ":Unknown MODE flag"
 */

/* 502: ":Cannot change mode for other users"
 */
