/*
    kircclientcommands.cpp - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircclientcommands.moc"

#include "kircclientsocket.h"

#include "kircevent.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*
class KIrc::ClientCommandsPrivate
{
public:
};
*/
using namespace KIrc;

ClientCommands::ClientCommands(QObject *parent)
	: KIrc::EventHandler(parent)
	, d(0)
{
}

ClientCommands::~ClientCommands()
{
//	delete d;
}
/*
void ClientCommands::postEvent(const Message &msg, Message::Type messageType, const QString &message)
{
	Event event;
//	event.setType(messageType);
//	event.setFrom(msg.prefix());
//	event.setCc(msg.socket().owner()); // server instead ?

	if (message.isEmpty())
		event.setText(msg.suffix());
	else
		event.setText(message);

//	post the message here ... still dunno where, socket ?
}
*/
void ClientCommands::postErrorEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::ErrorMessage, message);
}

void ClientCommands::postInfoEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::InfoMessage, message);
}

void ClientCommands::postMOTDEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::MOTDMessage, message);
}

void ClientCommands::receivedServerMessage(const Message &msg)
{
	receivedServerMessage(msg, msg.suffix());
}

void ClientCommands::receivedServerMessage(const Message &msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), Entity::List(), message);
}
/*
void ClientCommands::registerStandardCommands(CommandManager *cm)
{
	cm->registerCommand(ERROR,	this, SLOT(error(Message &));
//		setMinMax(0, 0);

	bind(JOIN,	this, SLOT(join(Message)),	0, 1);

	bind(KICK,	this, SLOT(kick(Message)),	2, 2);

	bind(MODE,	this, SLOT(mode(Message)),	1, 1);

	bind(NICK,	this, SLOT(nick(Message)),	0, 0);

	bind(NOTICE,	this, SLOT(notice(Message)),	1, 1);

	bind(PART,	this, SLOT(part(Message)),	1, 1);

	bind(PING,	this, SLOT(ping(Message)),	0, 0);

	bind(PONG,	this, SLOT(pong(Message)),	0, 0);

	bind(PRIVMSG,	this, SLOT(privmsg(Message)),	1, 1);

	bind(QUIT,	this, SLOT(quit(Message)),	0, 0);

//	bind(SQUIT,	this, SLOT(squit(Message)),	1, 1);

	bind(TOPIC,	this, SLOT(topic(Message)),	1, 1);
}
*/

// FIXME: Really handle this message
void ClientCommands::error(const Message &/*msg*/)
{
//	msg->client->close();
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
void ClientCommands::join(const Message &msg)
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
void ClientCommands::kick(const Message &msg)
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
void ClientCommands::mode(const Message &msg)
{
/*
	QStringList args = msg.argList();
	args.pop_front();

	Entity::Ptr fromEntity = msg.entityFromPrefix();
	Entity::Ptr toEntity = msg.entityFromArg(0)

	emit receivedMessage(
		Info,
		fromEntity,
		Entity::List::null,
		i18n(""));

	toEntity->setModes(args.join(" "));
*/
}

/* Nick name of a user changed
 * "<nickname>"
 */
void ClientCommands::nick(const Message &msg)
{
/*
	// FIXME: Find better i18n strings

	QString message;

	if (oldNick.toLower() == m_Nickname.toLower())
	{
		m_Nickname = msg.suffix();
		message = i18n("Your nick has changed from %1 to %2");
	}
	else
		message = i18n("User nick has changed from %1 to %2");

	emit receivedMessage(
		InfoMessage,
		msg.entityFromPrefix(),
		Entity::List::null,
		message);

	fromEntity->rename();
*/
}

void ClientCommands::notice(const Message &msg)
{
	if (!msg.suffix().isEmpty())
	{
/*
		emit receivedMessage(
			NoticeMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0), // should always return myself
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
void ClientCommands::part(const Message &msg)
{
/*
	emit receivedMessage(
		PartMessage,
		msg.entityFromPrefix(),
		msg.entityFromArg(0),
		msg.suffix());
*/
}

void ClientCommands::ping(const Message &msg)
{
/*
	Message reply;
	reply.setCommand(PONG);
//	reply.setArgs(msg.rawArg(0));
	reply.setSuffix(msg.rawSuffix());

//	msg->client->writeMessage(reply);
*/
}

void ClientCommands::pong(const Message &/*msg*/)
{
}

void ClientCommands::privmsg(const Message &msg)
{
	if (!msg.suffix().isEmpty())
	{
/*
		emit receivedMessage(
			PrivateMessage,
			msg.entityFromPrefix(),
			msg.entityFromArg(0),
			msg.suffix());
*/
	}
#if 0
	if (msg.hasCtcpMessage())
	{
//		invokeCtcpCommandOfMessage(m_ctcpQueries, msg);
	}
#endif
}

void ClientCommands::quit(const Message &msg)
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
void ClientCommands::topic(const Message &msg)
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
	bind(1, this, SLOT(numericReply_001(Message &)), 1, 1);
	bind(2, this, SLOT(numericReply_002(Message &)), 1, 1);
	bind(3, this, SLOT(numericReply_003(Message &)), 1, 1);
	bind(4, this, SLOT(numericReply_004(Message &)), 5, 5);
	bind(5, this, SLOT(numericReply_004(Message &)), 1, 1);

	bind(250, this, SLOT(numericReply_250(Message &)));
	bind(251, this, SLOT(numericReply_251(Message &)));
	bind(252, this, SLOT(numericReply_252(Message &)), 2, 2);
	bind(253, this, SLOT(numericReply_253(Message &)), 2, 2);
	bind(254, this, SLOT(numericReply_254(Message &)), 2, 2);
	bind(255, this, SLOT(numericReply_255(Message &)), 1, 1);

	bind(263, this, SLOT(numericReply_263(Message &)));
	bind(265, this, SLOT(numericReply_265(Message &)));
	bind(266, this, SLOT(numericReply_266(Message &)));

	bind(301, this, SLOT(numericReply_301(Message &)), 2, 2);
	bind(303, this, SLOT(numericReply_303(Message &)), 1, 1);
//	bind(305, this, SLOT(ignoreMessage(Message &)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(Message &)), 0, 0 );
	bind(307, this, SLOT(numericReply_307(Message &)), 1, 1);
	bind(311, this, SLOT(numericReply_311(Message &)), 5, 5);
	bind(312, this, SLOT(numericReply_312(Message &)), 3, 3);
	bind(313, this, SLOT(numericReply_313(Message &)), 2, 2);
	bind(314, this, SLOT(numericReply_314(Message &)), 5, 5);
	bind(315, this, SLOT(numericReply_315(Message &)), 2, 2);
	bind(317, this, SLOT(numericReply_317(Message &)), 3, 4);
	bind(318, this, SLOT(numericReply_318(Message &)), 2, 2);
	bind(319, this, SLOT(numericReply_319(Message &)), 2, 2);
	bind(320, this, SLOT(numericReply_320(Message &)), 2, 2);
//	bind(321, this, SLOT(ignoreMessage(Message &)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(Message &)), 3, 3);
	bind(323, this, SLOT(numericReply_323(Message &)), 1, 1);
	bind(324, this, SLOT(numericReply_324(Message &)), 2, 4);
	bind(328, this, SLOT(numericReply_328(Message &)), 2, 2);
	bind(329, this, SLOT(numericReply_329(Message &)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(Message &)), 3, 3); // ???
	bind(331, this, SLOT(numericReply_331(Message &)), 2, 2);
	bind(332, this, SLOT(numericReply_332(Message &)), 2, 2);
	bind(333, this, SLOT(numericReply_333(Message &)), 4, 4);
	bind(352, this, SLOT(numericReply_352(Message &)), 5, 10);
	bind(353, this, SLOT(numericReply_353(Message &)), 3, 3);
	bind(366, this, SLOT(numericReply_366(Message &)), 2, 2);
	bind(369, this, SLOT(numericReply_369(Message &)), 2, 2);
	bind(372, this, SLOT(numericReply_372(Message &)), 1, 1);
	bind(375, this, SLOT(ignoreMessage(Message&)), 0, 0 );
	bind(376, this, SLOT(ignoreMessage(Message&)), 0, 0 );

	bind(401, this, SLOT(numericReply_401(Message &)), 2, 2);
	bind(404, this, SLOT(numericReply_404(Message &)), 2, 2);
	bind(406, this, SLOT(numericReply_406(Message &)), 2, 2);
	bind(422, this, SLOT(numericReply_422(Message &)), 1, 1);
	bind(433, this, SLOT(numericReply_433(Message &)), 2, 2);
	bind(442, this, SLOT(numericReply_442(Message &)), 2, 2);
	bind(464, this, SLOT(numericReply_464(Message &)), 1, 1);
	bind(471, this, SLOT(numericReply_471(Message &)), 2, 2);
	bind(473, this, SLOT(numericReply_473(Message &)), 2, 2);
	bind(474, this, SLOT(numericReply_474(Message &)), 2, 2);
	bind(475, this, SLOT(numericReply_475(Message &)), 2, 2);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(Message&)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void ClientCommands::numericReply_001(const Message &msg)
{
	kDebug(14121) ;

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	receivedServerMessage(msg);

//	msg->client->setConnectionState(Socket::Open);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given information are close to 004.
 */
void ClientCommands::numericReply_002(const Message &msg)
{
	receivedServerMessage(msg);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void ClientCommands::numericReply_003(const Message &msg)
{
	receivedServerMessage(msg);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void ClientCommands::numericReply_004(const Message &msg)
{
//	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void ClientCommands::numericReply_005(const Message &msg)
{
	receivedServerMessage(msg);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientCommands::numericReply_250(const Message &msg)
{
	postInfoEvent(msg);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void ClientCommands::numericReply_251(const Message &msg)
{
	postInfoEvent(msg);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void ClientCommands::numericReply_252(const Message &msg)
{
//	postInfoEvent(msg, i18np("There is 1 operator online.", "There are %s operators online.", msg.argAt(1).toULong()));
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void ClientCommands::numericReply_253(const Message &msg)
{
//	postInfoEvent(msg, i18np("There is 1 unknown connection.", "There are %s unknown connections.", msg.argAt(1).toULong()));
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void ClientCommands::numericReply_254(const Message &msg)
{
	postInfoEvent(msg, i18np("There has been 1 channel formed.",
		"There have been %1 channel formed.",
		msg.argAt(1).toULong()));
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void ClientCommands::numericReply_255(const Message &msg)
{
	postInfoEvent(msg);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void ClientCommands::numericReply_263(const Message &msg)
{
//	receivedServerMessage(msg, i18n("Server was too busy to execute %1.", msg.arg(1)));
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void ClientCommands::numericReply_265(const Message &msg)
{
	postInfoEvent(msg);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void ClientCommands::numericReply_266(const Message &msg)
{
	postInfoEvent(msg);
}

/* 301: "<nick> :<away message>"
 */
void ClientCommands::numericReply_301(const Message &msg)
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
void ClientCommands::numericReply_303(const Message &msg)
{
/*
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), msg.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).trimmed().isEmpty())
			emit incomingUserOnline(*it);
	}
*/
}

/* 305: ":You are no longer marked as being away"
 */
void ClientCommands::numericReply_305(const Message &msg)
{
/*
	Entity::Ptr self = this->self();
	self->setAwayMessage(QString());
//	self->setModes("-a");
	postInfoEvent(msg, i18n("You are no longer marked as being away."));
*/
}


/* 306: ":You have been marked as being away"
 */
void ClientCommands::numericReply_306(const Message &msg)
{
//	Entity::Ptr self = d->client->owner();
//	self->setModes("+a");
	postInfoEvent(msg, i18n("You have been marked as being away."));
}

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
void ClientCommands::numericReply_307(const Message &msg)
{
//	postErrorEvent(msg, i18n("%1 is a registered nick.", msg.arg(1)));
}

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_311(const Message &msg)
{
//	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void ClientCommands::numericReply_312(const Message &msg)
{
//	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void ClientCommands::numericReply_313(const Message &msg)
{
//	postInfoEvent(msg, i18n("%1 is an IRC operator.", msg.arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void ClientCommands::numericReply_314(const Message &msg)
{
//	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
void ClientCommands::numericReply_315(const Message &msg)
{
	receivedServerMessage(msg);
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_317(const Message &msg)
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
void ClientCommands::numericReply_318(const Message &msg)
{
	emit receivedServerMessage(msg);
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void ClientCommands::numericReply_319(const Message &msg)
{
//	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void ClientCommands::numericReply_320(const Message &msg)
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
void ClientCommands::numericReply_322(const Message &msg)
{
//	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void ClientCommands::numericReply_323(const Message &msg)
{
	emit receivedServerMessage(msg);
}

/* 324: "<channel> <mode> <mode params>"
 */
void ClientCommands::numericReply_324(const Message &msg)
{
//	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328:
 */
void ClientCommands::numericReply_328(const Message &msg)
{
//	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientCommands::numericReply_329(const Message &/*msg*/)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void ClientCommands::numericReply_331(const Message &/*msg*/)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void ClientCommands::numericReply_332(const Message &msg)
{
//	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void ClientCommands::numericReply_333(const Message &msg)
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
void ClientCommands::numericReply_352(const Message &msg)
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
void ClientCommands::numericReply_353(const Message &msg)
{
//	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void ClientCommands::numericReply_366(const Message &msg)
{
	emit receivedServerMessage(msg);
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void ClientCommands::numericReply_369(const Message &msg)
{
	emit receivedServerMessage(msg);
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void ClientCommands::numericReply_372(const Message &msg)
{
#ifdef __GNUC__
	#warning FIXME remove the "- " in front.
#endif
	postMOTDEvent(msg);
}

/* 375: ":- <server> Message of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */
void ClientCommands::numericReply_375(const Message &msg)
{
	postMOTDEvent(msg);
}

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void ClientCommands::numericReply_376(const Message &msg)
{
	postMOTDEvent(msg);
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void ClientCommands::numericReply_401(const Message &msg)
{
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 404: "<channel name> :Cannot send to channel"
 */
void ClientCommands::numericReply_404(const Message &msg)
{
//	postErrorEvent(msg, i18n("You cannot send message to channel %1.", msg.arg(1)));
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void ClientCommands::numericReply_406(const Message &msg)
{
#ifdef __GNUC__
	#warning FIXME 406 MEANS *NEVER*, unlike 401
#endif
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 422: ":MOTD File is missing"
 *
 * Server's MOTD file could not be opened by the server.
 */
void ClientCommands::numericReply_422(const Message &msg)
{
	postErrorEvent(msg);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void ClientCommands::numericReply_433(const Message &msg)
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
//	postErrorEvent(msg, i18n("Nickname %1 is already in use.", msg.arg(1)));
}

/* 442: "<channel> :You're not on that channel"
 */
void ClientCommands::numericReply_442(const Message &msg)
{
//	postErrorEvent(msg, i18n("You are not on channel %1.", msg.arg(1)));
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void ClientCommands::numericReply_464(const Message &msg)
{
	/* Server need pass.. Call disconnect*/
//	emit incomingFailedServerPassword();
//	postErrorEvent(msg, i18n("Incorrect password."));
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void ClientCommands::numericReply_471(const Message &msg)
{
//	postErrorEvent(msg, i18n("Cannot join %1, channel is full.", msg.arg(1)) );
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void ClientCommands::numericReply_473(const Message &msg)
{
//	postErrorEvent(msg, i18n("Cannot join %1, channel is invite only.", msg.arg(1)) );
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void ClientCommands::numericReply_474(const Message &msg)
{
//	postErrorEvent(msg, i18n("Cannot join %1, you are banned from that channel.", msg.arg(1)) );
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void ClientCommands::numericReply_475(const Message &msg)
{
//	postErrorEvent(msg, i18n("Cannot join %1, wrong channel key was given.", msg.arg(1)) );
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

