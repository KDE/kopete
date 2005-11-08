/*
    kircclientcommands.cpp - IRC Client Commands

    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>
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

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "kircclientcommands.moc"

#include "kircclient.h"
#include "kircmessage.h"
#include "kirctransferhandler.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
/*
class KIRC::ClientCommands::Private
{
public:
};
*/
using namespace KIRC;

ClientCommands::ClientCommands(QObject *parent)
	: QObject(parent)
	, d(0)
{
}

ClientCommands::~ClientCommands()
{
//	delete d;
}

void ClientCommands::receivedServerMessage(KIRC::Message msg)
{
	receivedServerMessage(msg, msg.suffix());
}

void ClientCommands::receivedServerMessage(KIRC::Message msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), Entity::List(), message);
}
/*
void ClientCommands::registerStandardCommands(CommandManager *cm)
{
	cm->registerCommand(ERROR,	this, SLOT(error(KIRC::Message &));
//		setMinMax(0, 0);

	bind(JOIN,	this, SLOT(join(KIRC::Message)),	0, 1);

	bind(KICK,	this, SLOT(kick(KIRC::Message)),	2, 2);

	bind(MODE,	this, SLOT(mode(KIRC::Message)),	1, 1);

	bind(NICK,	this, SLOT(nick(KIRC::Message)),	0, 0);

	bind(NOTICE,	this, SLOT(notice(KIRC::Message)),	1, 1);

	bind(PART,	this, SLOT(part(KIRC::Message)),	1, 1);

	bind(PING,	this, SLOT(ping(KIRC::Message)),	0, 0);

	bind(PONG,	this, SLOT(pong(KIRC::Message)),	0, 0);

	bind(PRIVMSG,	this, SLOT(privmsg(KIRC::Message)),	1, 1);

	bind(QUIT,	this, SLOT(quit(KIRC::Message)),	0, 0);

//	bind(SQUIT,	this, SLOT(squit(KIRC::Message)),	1, 1);

	bind(TOPIC,	this, SLOT(topic(KIRC::Message)),	1, 1);
}
*/

// FIXME: Really handle this message
void ClientCommands::error(Message /*msg*/)
{
//	msg->client->close();
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
void ClientCommands::join(Message msg)
{
/*
	if (msg.argsSize()==1)
		emit incomingJoinedChannel(msg.arg(0), msg.prefix());
	else
		emit incomingJoinedChannel(msg.suffix(), msg.prefix());

	emit receivedMessage(
		JoinMessage,
		fromEntity,
		toEntity,
		i18n(""));
*/
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
void ClientCommands::kick(Message msg)
{
/*
	emit incomingKick(msg.arg(0), msg.prefix(), msg.arg(1), msg.suffix());
	emit receivedMessage(
		PartMessage,
		fromEntity,
		toEntity,
		i18n(""));
*/
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
void ClientCommands::mode(Message msg)
{
	QStringList args = msg.argList();
	args.pop_front();
/*
	Entity::Ptr fromEntity = msg.entityFromPrefix();
	Entity::Ptr toEntity = msg.entityFromArg(0)

	emit receivedMessage(
		Info,
		fromEntity,
		KIRC::Entity::List::null,
		i18n(""));

	toEntity->setModes(args.join(" "));
*/
}

/* Nick name of a user changed
 * "<nickname>"
 */
void ClientCommands::nick(Message msg)
{
/*
	// FIXME: Find better i18n strings

	QString message;

	if (oldNick.lower() == m_Nickname.lower())
	{
		m_Nickname = msg.suffix();
		message = i18n("Your nick has changed from %1 to %2");
	}
	else
		message = i18n("User nick has changed from %1 to %2");

	emit receivedMessage(
		InfoMessage,
		msg.entityFromPrefix(),
		KIRC::Entity::List::null,
		message);

	fromEntity->rename();
*/
}

void ClientCommands::notice(Message msg)
{
	if (!msg.suffix().isEmpty())
	{
/*
		emit receivedMessage(
			NoticeMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0), // shoul allways return myself
			msg.suffix()
		);
*/
	}

//	if(msg.hasCtcpMessage())
//		invokeCtcpCommandOfMessage(m_ctcpReplies, msg);
}

/* This signal emits when a user parts a channel
 * "<channel> *( "," <channel> ) [ <Part Message> ]"
 */
void ClientCommands::part(Message msg)
{
/*
	emit receivedMessage(
		PartMessage,
		msg.entityFromPrefix(),
		msg.entityFromArg(0),
		msg.suffix());
*/
}

void ClientCommands::ping(Message imsg)
{
	Message msg;
	msg.setCommand(PONG);
//	msg.setArgs(imsg.rawArg(0));
	msg.setSuffix(imsg.rawSuffix());

//	msg->client->writeMessage(msg);
}

void ClientCommands::pong(Message /*msg*/)
{
}

void ClientCommands::privmsg(Message msg)
{
/*
	if (!msg.suffix().isEmpty())
	{
		emit receivedMessage(
			PrivateMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0),
			msg.suffix());
	}

	if (msg.hasCtcpMessage())
	{
		invokeCtcpCommandOfMessage(m_ctcpQueries, msg);
	}
*/
}

void ClientCommands::quit(Message msg)
{
/*
	emit receivedMessage(
		QuitMessage,
		msg.prefixEntity(),
		m_server,
		msg.suffix());
*/
}

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
void ClientCommands::topic(Message msg)
{
/*
	emit incomingTopicChange(msg.arg(0), msg.prefix(), msg.suffix());
	emit receivedMessage(
		QuitMessage,
		msg.prefixEntity(),
		m_server,
		msg.suffix());
*/
}


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void ClientCommands::bindNumericReplies()
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

	bind(263, this, SLOT(numericReply_263(KIRC::Message &)));
	bind(265, this, SLOT(numericReply_265(KIRC::Message &)));
	bind(266, this, SLOT(numericReply_266(KIRC::Message &)));

	bind(301, this, SLOT(numericReply_301(KIRC::Message &)), 2, 2);
	bind(303, this, SLOT(numericReply_303(KIRC::Message &)), 1, 1);
//	bind(305, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 );
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
//	bind(321, this, SLOT(ignoreMessage(KIRC::Message &)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(KIRC::Message &)), 3, 3);
	bind(323, this, SLOT(numericReply_323(KIRC::Message &)), 1, 1);
	bind(324, this, SLOT(numericReply_324(KIRC::Message &)), 2, 4);
	bind(328, this, SLOT(numericReply_328(KIRC::Message &)), 2, 2);
	bind(329, this, SLOT(numericReply_329(KIRC::Message &)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(KIRC::Message &)), 3, 3); // ???
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

	bind(401, this, SLOT(numericReply_401(KIRC::Message &)), 2, 2);
	bind(404, this, SLOT(numericReply_404(KIRC::Message &)), 2, 2);
	bind(406, this, SLOT(numericReply_406(KIRC::Message &)), 2, 2);
	bind(422, this, SLOT(numericReply_422(KIRC::Message &)), 1, 1);
	bind(433, this, SLOT(numericReply_433(KIRC::Message &)), 2, 2);
	bind(442, this, SLOT(numericReply_442(KIRC::Message &)), 2, 2);
	bind(464, this, SLOT(numericReply_464(KIRC::Message &)), 1, 1);
	bind(471, this, SLOT(numericReply_471(KIRC::Message &)), 2, 2);
	bind(473, this, SLOT(numericReply_473(KIRC::Message &)), 2, 2);
	bind(474, this, SLOT(numericReply_474(KIRC::Message &)), 2, 2);
	bind(475, this, SLOT(numericReply_475(KIRC::Message &)), 2, 2);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(KIRC::Message&)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void ClientCommands::numericReply_001(Message msg)
{
	kdDebug(14121) << k_funcinfo << endl;

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	receivedServerMessage(msg);

//	msg->client->setConnectionState(KIRC::Socket::Open);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given informations are close to 004.
 */
void ClientCommands::numericReply_002(Message msg)
{
	receivedServerMessage(msg);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void ClientCommands::numericReply_003(Message msg)
{
	receivedServerMessage(msg);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void ClientCommands::numericReply_004(Message msg)
{
//	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void ClientCommands::numericReply_005(Message msg)
{
	receivedServerMessage(msg);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientCommands::numericReply_250(Message msg)
{
	receivedServerMessage(msg);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void ClientCommands::numericReply_251(Message msg)
{
	receivedServerMessage(msg);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void ClientCommands::numericReply_252(Message msg)
{
	receivedServerMessage(msg, i18n("There are %1 operators online.").arg(msg.arg(1)));
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void ClientCommands::numericReply_253(Message msg)
{
	receivedServerMessage(msg, i18n("There are %1 unknown connections.").arg(msg.arg(1)));
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void ClientCommands::numericReply_254(Message msg)
{
	receivedServerMessage(msg, i18n("There are %1 channel formed.").arg(msg.arg(1)));
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void ClientCommands::numericReply_255(Message msg)
{
	receivedServerMessage(msg);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void ClientCommands::numericReply_263(Message msg)
{
	receivedServerMessage(msg, i18n("Server was too busy to execute %1.").arg(msg.arg(1)));
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void ClientCommands::numericReply_265(Message msg)
{
	receivedServerMessage(msg);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void ClientCommands::numericReply_266(Message msg)
{
	receivedServerMessage(msg);
}

/* 301: "<nick> :<away message>"
 */
void ClientCommands::numericReply_301(Message msg)
{
/*
	Entity entity = msg.entityFromArg(1);
	entity->setAwayMessage(msg.suffix);
	entity->setMode("+a");

	receivedServerMessage(msg);
*/
}

/* 303: ":*1<nick> *(" " <nick> )"
 */
void ClientCommands::numericReply_303(Message msg)
{
/*
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).stripWhiteSpace().isEmpty())
			emit incomingUserOnline(*it);
	}
*/
}

/* 305: ":You are no longer marked as being away"
 */
void ClientCommands::numericReply_305(Message msg)
{
/*
	Entity::Ptr self = this->self();
	self->setAwayMessage(QString::null);
//	self->setModes("-a");
	receivedServerMessage(msg, i18n("You are no longer marked as being away."));
*/
}


/* 306: ":You have been marked as being away"
 */
void ClientCommands::numericReply_306(Message msg)
{
//	Entity::Ptr self = d->client->owner();
//	self->setModes("+a");
	receivedServerMessage(msg, i18n("You have been marked as being away."));
}

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
void ClientCommands::numericReply_307(Message msg)
{
	receivedServerMessage(msg, i18n("%1 is a registered nick.").arg(msg.arg(1)));
}

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_311(Message msg)
{
//	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void ClientCommands::numericReply_312(Message msg)
{
//	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void ClientCommands::numericReply_313(Message msg)
{
	receivedServerMessage(msg, i18n("%1 is an IRC operator.").arg(msg.arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void ClientCommands::numericReply_314(Message msg)
{
//	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
void ClientCommands::numericReply_315(Message msg)
{
	receivedServerMessage(msg);
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_317(Message msg)
{
/*
	emit incomingWhoIsIdle(msg.arg(1), msg.arg(2).toULong());
	if (msg.argsSize()==4)
		emit incomingSignOnTime(msg.arg(1),msg.arg(3).toULong());
*/
}

/* 318: "<nick>{<space><realname>} :End of /WHOIS list"
 * End of WHOIS for a given nick.
 */
void ClientCommands::numericReply_318(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_319(Message msg)
{
//	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void ClientCommands::numericReply_320(Message msg)
{
//	emit incomingWhoIsIdentified(msg.arg(1));
}

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
void ClientCommands::numericReply_322(Message msg)
{
//	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void ClientCommands::numericReply_323(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 324: "<channel> <mode> <mode params>"
 */
void ClientCommands::numericReply_324(Message msg)
{
//	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328: "<channel> <mode> <mode params>"
 */
void ClientCommands::numericReply_328(Message msg)
{
//	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientCommands::numericReply_329(Message /*msg*/)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void ClientCommands::numericReply_331(Message /*msg*/)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void ClientCommands::numericReply_332(Message msg)
{
//	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void ClientCommands::numericReply_333(Message msg)
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
void ClientCommands::numericReply_352(Message msg)
{
/*
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
*/
}


/* 353:
 * NAMES list
 */
void ClientCommands::numericReply_353(Message msg)
{
//	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void ClientCommands::numericReply_366(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void ClientCommands::numericReply_369(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void ClientCommands::numericReply_372(Message msg)
{
	#warning FIXME remove the "- " in front.
	receivedServerMessage(msg);
}

/* 375: ":- <server> Message of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void ClientCommands::numericReply_376(Message msg)
{
	receivedServerMessage(msg);
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void ClientCommands::numericReply_401(Message msg)
{
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 404: "<channel name> :Cannot send to channel"
 */
void ClientCommands::numericReply_404(Message msg)
{
	receivedServerMessage(msg, i18n("You cannot send message to channel %2.").arg(msg.arg(1)));
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void ClientCommands::numericReply_406(Message msg)
{
	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 422: ":MOTD File is missing"
 *
 * Server's MOTD file could not be opened by the server.
 */
void ClientCommands::numericReply_422(Message msg)
{
	receivedServerMessage(msg);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void ClientCommands::numericReply_433(Message msg)
{
//	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
//		m_FailedNickOnLogin = true;
//		emit incomingFailedNickOnLogin(msg.arg(1));
	}
//	else
//	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
//		emit incomingNickInUse(msg.arg(1));
//	}
}

/* 442: "<channel> :You're not on that channel"
 */
void ClientCommands::numericReply_442(Message msg)
{
	receivedServerMessage(msg, i18n("You are not on channel %1.").arg(msg.arg(1)));
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void ClientCommands::numericReply_464(Message /*msg*/)
{
	/* Server need pass.. Call disconnect*/
//	emit incomingFailedServerPassword();
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void ClientCommands::numericReply_471(Message msg)
{
	receivedServerMessage(msg, i18n("Cannot join %1, channel is full.").arg(msg.arg(1)) );
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void ClientCommands::numericReply_473(Message msg)
{
	receivedServerMessage(msg, i18n("Cannot join %1, channel is invite only.").arg(msg.arg(1)) );
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void ClientCommands::numericReply_474(Message msg)
{
	receivedServerMessage(msg, i18n("Cannot join %1, you are banned from that channel.").arg(msg.arg(1)) );
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void ClientCommands::numericReply_475(Message msg)
{
	receivedServerMessage(msg, i18n("Cannot join %1, wrong channel key was given.").arg(msg.arg(1)) );
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void ClientCommands::numericReply_477(Message msg)
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

#ifndef KIRC_STRICT

/*
void ClientCommands::bindCtcp()
{
	bindCtcpQuery("ACTION",		this, SLOT(CtcpQuery_action(KIRC::Message &)),
		-1,	-1);
	bindCtcpQuery("CLIENTINFO",	this, SLOT(CtcpQuery_clientinfo(KIRC::Message &)),
		-1,	1);
	bindCtcpQuery("DCC",		this, SLOT(CtcpQuery_dcc(KIRC::Message &)),
		4,	5);
	bindCtcpQuery("FINGER",		this, SLOT(CtcpQuery_finger(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("PING",		this, SLOT(CtcpQuery_ping(KIRC::Message &)),
		1,	1);
	bindCtcpQuery("SOURCE",		this, SLOT(CtcpQuery_source(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("TIME",		this, SLOT(CtcpQuery_time(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("USERINFO",	this, SLOT(CtcpQuery_userinfo(KIRC::Message &)),
		-1,	0);
	bindCtcpQuery("VERSION",	this, SLOT(CtcpQuery_version(KIRC::Message &)),
		-1,	0);

	bindCtcpReply("ERRMSG",		this, SLOT(CtcpReply_errmsg(KIRC::Message &)),
		1,	-1);
	bindCtcpReply("PING",		this, SLOT(CtcpReply_ping(KIRC::Message &)),
		1,	1,	"");
	bindCtcpReply("VERSION",	this, SLOT(CtcpReply_version(KIRC::Message &)),
		-1,	-1,	"");
}
*/

void ClientCommands::CtcpQuery_action(Message msg)
{
/*	QString target = msg.arg(0);
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(target, msg, msg.ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(msg, target, msg.ctcpMessage().ctcpRaw());*/
}

/*
NO REPLY EXIST FOR THE CTCP ACTION COMMAND !
bool ClientCommands::CtcpReply_action(Message msg)
{
}
*/

//	FIXME: the API can now answer to help commands.
void ClientCommands::CtcpQuery_clientinfo(Message msg)
{
	QString clientinfo = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), QString::null, clientinfo);
}

void ClientCommands::CtcpQuery_dcc(Message msg)
{
//	Message &ctcpMsg = msg.ctcpMessage();
	Message ctcpMsg;

	QString dccCommand = ctcpMsg.arg(0).upper();

	if (dccCommand == QString::fromLatin1("CHAT"))
	{
//		if(ctcpMsg.argsSize()!=4) return false;

		/* DCC CHAT type longip port
		 *
		 *  type   = Either Chat or Talk, but almost always Chat these days
		 *  longip = 32-bit Internet address of originator's machine
		 *  port   = Port on which the originator is waitng for a DCC chat
		 */
		bool okayHost, okayPort;
		// should ctctMsg.arg(1) be tested?
		QHostAddress address(ctcpMsg.arg(2).toUInt(&okayHost));
		unsigned int port = ctcpMsg.arg(3).toUInt(&okayPort);
		if (okayHost && okayPort)
		{
			kdDebug(14120) << "Starting DCC chat window." << endl;
//			TransferHandler::self()->createClient(
//				this, msg.prefix(),
//				address, port,
//				Transfer::Chat );
		}
	}
	else if (dccCommand == QString::fromLatin1("SEND"))
	{
//		if(ctcpMsg.argsSize()!=5) return false;

		/* DCC SEND (filename) (longip) (port) (filesize)
		 *
		 *  filename = Name of file being sent
		 *  longip   = 32-bit Internet address of originator's machine
		 *  port     = Port on which the originator is waiitng for a DCC chat
		 *  filesize = Size of file being sent
		 */
		bool okayHost, okayPort, okaySize;
//		QFileInfo realfile(msg.arg(1));
		QHostAddress address(ctcpMsg.arg(2).toUInt(&okayHost));
		unsigned int port = ctcpMsg.arg(3).toUInt(&okayPort);
		unsigned int size = ctcpMsg.arg(4).toUInt(&okaySize);
		if (okayHost && okayPort && okaySize)
		{
			kdDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1) << endl;
//			TransferHandler::self()->createClient(
//				this, msg.prefix(),
//				address, port,
//				Transfer::FileIncoming,
//				ctcpMsg.arg(1), size );
		}
	}
//	else
//		((MessageRedirector *)sender())->error("Unknow dcc command");
}

/*
NO REPLY EXIST FOR THE CTCP DCC COMMAND !
bool ClientCommands::CtcpReply_dcc(Message msg)
{
}
*/

void ClientCommands::CtcpReply_errmsg(Message /*msg*/)
{
	// should emit one signal
}

void ClientCommands::CtcpQuery_finger( Message /*msg*/)
{
	// To be implemented
}

void ClientCommands::CtcpQuery_ping(Message msg)
{
//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), msg.ctcpMessage().arg(0));
}

void ClientCommands::CtcpReply_ping(Message msg)
{
/*	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		// FIXME: the time code is wrong for usec
		QString timeReply = QString::fromLatin1("%1.%2").arg(time.tv_sec).arg(time.tv_usec);
		double newTime = timeReply.toDouble();
		double oldTime = msg.suffix().section(' ',0, 0).toDouble();
		double difference = newTime - oldTime;
		QString diffString;

		if (difference < 1)
		{
			diffString = QString::number(difference);
			diffString.remove((diffString.find('.') -1), 2);
			diffString.truncate(3);
			diffString.append("milliseconds");
		}
		else
		{
			diffString = QString::number(difference);
			QString seconds = diffString.section('.', 0, 0);
			QString millSec = diffString.section('.', 1, 1);
			millSec.remove(millSec.find('.'), 1);
			millSec.truncate(3);
			diffString = QString::fromLatin1("%1 seconds, %2 milliseconds").arg(seconds).arg(millSec);
		}

		emit incomingCtcpReply(QString::fromLatin1("PING"), msg.prefix(), diffString);
	}
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");*/
}

void ClientCommands::CtcpQuery_source(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), m_SourceString);
}

void ClientCommands::CtcpQuery_time(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QDateTime::currentDateTime().toString(),
//			      QString::null, false);
}

void ClientCommands::CtcpQuery_userinfo(Message msg)
{
//	QString userinfo = m_UserString;

//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QString::null, userinfo);
}

void ClientCommands::CtcpQuery_version(Message msg)
{
//	QString response = m_VersionString;

//	writeCtcpReplyMessage(msg.prefix(),
//		msg.ctcpMessage().command() + " " + response);
}

void ClientCommands::CtcpReply_version(Message msg)
{
//	emit incomingCtcpReply(msg.ctcpMessage().command(), msg.prefix(), msg.ctcpMessage().ctcpRaw());
}

#endif // KIRC_STRICT
