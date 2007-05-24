/*
    kircclientcommands.cpp - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2006 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircclienttask.moc"

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

// For now lets define it to be empty
#define CHECK_ARGS(min, max)

/*
class ClientTask::Private
{
public:
};
*/
using namespace KIrc;

ClientTask::ClientTask(QObject *parent)
	: SimpleTask(parent)
	, d(0)
{
}

ClientTask::~ClientTask()
{
//	delete d;
}

/*
void ClientTask::postEvent(const Message &msg, Event::MessageType messageType, const QString &message)
{
	Event event;
	event.setType(messageType);
//	event.setFrom(msg.prefix());
//	event.setCc(msg.socket().owner()); // server instead ?

	if (message.isEmpty())
		event.setMessage(msg.suffix());
	else
		event.setMessage(message);

//	post the message here ... still dunno where, socket ?
}
*/
void ClientTask::postErrorEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::ErrorMessage, message);
}

void ClientTask::postInfoEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::InfoMessage, message);
}

void ClientTask::postMOTDEvent(const Message &msg, const QString &message)
{
//	postEvent(msg, Event::MOTDMessage, message);
}

void ClientTask::receivedServerMessage(Message msg)
{
	receivedServerMessage(msg, msg.suffix());
}

void ClientTask::receivedServerMessage(Message msg, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), Entity::List(), message);
}

// FIXME: Really handle this message
void ClientTask::error(Message /*msg*/)
{
	CHECK_ARGS(0, 0);

//	msg->client->close();
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
void ClientTask::join(Message msg)
{
	CHECK_ARGS(0, 0);

/*
	Entity::Ptr who = msg.entityFromPrefix();

#if 0
	if (msg.prefix() == QLatin1String("0"))
		// leave all the channels
	else
#endif
	{
		// For now lets forgot about the keys
		Entity::List channels = msg.entitiesFromNames(msg.prefix())
		foreach(Entity::Ptr channel, channelNames)
		{
			Event e;

			who->join(channel);
		}
	}
*/
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
void ClientTask::kick(Message msg)
{
	CHECK_ARGS(2, 2);

/*
	Entity::Ptr who = msg.entityFromPrefix();
	Entity::List channel = msg.entitiesFromNames(msg.arg(0));
	Entity::List names = msg.entitiesFromNames(msg.arg(1));
	QString reason = msg.suffix(); // who.codec

	emit incomingKick(msg.arg(0), msg.prefix(), msg.arg(1), msg.suffix());
*/
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
void ClientTask::mode(Message msg)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr who = msg.entityFromPrefix();
	Entity::Ptr entity = msg.entityFromArg(0);
	QString modeDelta = msg.arg(1); 

	entity->setMode(modeDelta);
*/
}

/* Nick name of a user changed
 * "<nickname>"
 */
void ClientTask::nick(Message msg)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr who = msg.entityFromPrefix();
//	QString newNick/oldNick = msg.arg(0);
*/
}

void ClientTask::notice(Message msg)
{
	CHECK_ARGS(1, 1);

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
void ClientTask::part(Message msg)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr who = msg.entityFromPrefix();
	Entity::List channels = msg.entitiesFromArg(0);

	who->part(channels);
*/
}

void ClientTask::ping(Message msg)
{
	CHECK_ARGS(0, 0);

	Message reply;
//	reply.setCommand(PONG);
//	reply.setArgs(msg.rawArg(0));
	reply.setSuffix(msg.rawSuffix());

//	msg->client->writeMessage(reply);
}

void ClientTask::pong(Message /*msg*/)
{
	CHECK_ARGS(0, 0);


}

void ClientTask::privmsg(Message msg)
{
	CHECK_ARGS(1, 1);

#warning decode the ctcp commands here.
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

void ClientTask::quit(Message msg)
{
	CHECK_ARGS(0, 0);

/*
	Entity::Ptr who = msg.entityFromPrefix();
	QString message = msg.suffix();

//	who->quit();

	emit receivedMessage(
		QuitMessage,
		msg.prefixEntity(),
		m_server,
		msg.suffix());
*/
}

/*
void CLientCommands::squit(Message msg)
{
	CHECK_ARGS(1, 1);
}
*/

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
void ClientTask::topic(Message msg)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr who = msg.entityFromPrefix();
	Entity::Ptr channel = msg.entityFromArg(0);
	QString newTopic = msg.suffix();

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
void ClientTask::bindNumericReplies()
{
	bind(263, this, SLOT(numericReply_263(Message &)));
	bind(265, this, SLOT(numericReply_265(Message &)));
	bind(266, this, SLOT(numericReply_266(Message &)));

//	bind(305, this, SLOT(ignoreMessage(Message &)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(Message &)), 0, 0 );
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
	bind(352, this, SLOT(numericReply_352(Message &)), 5, 10);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(Message&)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void ClientTask::numericReply_001(Message msg)
{
	CHECK_ARGS(1, 1);

	kDebug(14121) << k_funcinfo << endl;

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	receivedServerMessage(msg);

//	msg->client->setConnectionState(Socket::Open);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given information are close to 004.
 */
void ClientTask::numericReply_002(Message msg)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(msg);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void ClientTask::numericReply_003(Message msg)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(msg);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void ClientTask::numericReply_004(Message msg)
{
	CHECK_ARGS(5, 5);

//	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void ClientTask::numericReply_005(Message msg)
{
//	CHECK_ARGS(?, ?);

	receivedServerMessage(msg);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientTask::numericReply_250(Message msg)
{
//	CHECK_ARGS(1, 1);

	postInfoEvent(msg);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void ClientTask::numericReply_251(Message msg)
{
//	CHECK_ARGS(1, 1);

	postInfoEvent(msg);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void ClientTask::numericReply_252(Message msg)
{
	CHECK_ARGS(2, 2);

	postInfoEvent(msg, i18np("There is 1 operator online.", "There are %s operators online.", msg.arg(1).toULong()));
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void ClientTask::numericReply_253(Message msg)
{
	CHECK_ARGS(2, 2);

	postInfoEvent(msg, i18np("There is 1 unknown connection.", "There are %s unknown connections.", msg.arg(1).toULong()));
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void ClientTask::numericReply_254(Message msg)
{
	CHECK_ARGS(2, 2);

	postInfoEvent(msg, i18np("There is 1 channel formed.", "There are %s channel formed.", msg.arg(1).toULong()));
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void ClientTask::numericReply_255(Message msg)
{
	CHECK_ARGS(1, 1);

	postInfoEvent(msg);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void ClientTask::numericReply_263(Message msg)
{
	receivedServerMessage(msg, i18n("Server was too busy to execute %1.", msg.arg(1)));
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void ClientTask::numericReply_265(Message msg)
{
	postInfoEvent(msg);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void ClientTask::numericReply_266(Message msg)
{
	postInfoEvent(msg);
}

/* 301: "<nick> :<away message>"
 */
void ClientTask::numericReply_301(Message msg)
{
	CHECK_ARGS(2, 2);

/*
	Entity entity = msg.entityFromArg(1);
	entity->setAwayMessage(msg.suffix);
	entity->setMode("+a");

	receivedServerMessage(msg);
*/
}

/* 303: ":*1<nick> *(" " <nick> )"
 */
void ClientTask::numericReply_303(Message msg)
{
	CHECK_ARGS(1, 1);

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
void ClientTask::numericReply_305(Message msg)
{
/*
	Entity::Ptr self = this->self();
	self->setAwayMessage(QString::null);
//	self->setModes("-a");
	postInfoEvent(msg, i18n("You are no longer marked as being away."));
*/
}


/* 306: ":You have been marked as being away"
 */
void ClientTask::numericReply_306(Message msg)
{
//	Entity::Ptr self = d->client->owner();
//	self->setModes("+a");
	postInfoEvent(msg, i18n("You have been marked as being away."));
}

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
void ClientTask::numericReply_307(Message msg)
{
	CHECK_ARGS(1, 1);

	postErrorEvent(msg, i18n("%1 is a registered nick.", msg.arg(1)));
}

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void ClientTask::numericReply_311(Message msg)
{
	CHECK_ARGS(5, 5);

//	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void ClientTask::numericReply_312(Message msg)
{
//	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void ClientTask::numericReply_313(Message msg)
{
	postInfoEvent(msg, i18n("%1 is an IRC operator.", msg.arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void ClientTask::numericReply_314(Message msg)
{
//	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
void ClientTask::numericReply_315(Message msg)
{
	receivedServerMessage(msg);
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void ClientTask::numericReply_317(Message msg)
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
void ClientTask::numericReply_318(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void ClientTask::numericReply_319(Message msg)
{
//	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void ClientTask::numericReply_320(Message msg)
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
void ClientTask::numericReply_322(Message msg)
{
//	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void ClientTask::numericReply_323(Message msg)
{
	emit receivedServerMessage(msg);
}

/* 324: "<channel> <mode> <mode params>"
 */
void ClientTask::numericReply_324(Message msg)
{
//	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328:
 */
void ClientTask::numericReply_328(Message msg)
{
//	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientTask::numericReply_329(Message /*msg*/)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void ClientTask::numericReply_331(Message /*msg*/)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void ClientTask::numericReply_332(Message msg)
{
//	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void ClientTask::numericReply_333(Message msg)
{
	CHECK_ARGS(4, 4);

/*
	QDateTime d;
	d.setTime_t( msg.arg(3).toLong() );
	emit incomingTopicUser(msg.arg(1), msg.arg(2), d );
*/
}

/* 352:
 * WHO Reply
 */
void ClientTask::numericReply_352(Message msg)
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
void ClientTask::numericReply_353(Message msg)
{
	CHECK_ARGS(3, 3);

//	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void ClientTask::numericReply_366(Message msg)
{
	CHECK_ARGS(2, 2);

	emit receivedServerMessage(msg);
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void ClientTask::numericReply_369(Message msg)
{
	CHECK_ARGS(2, 2);

	emit receivedServerMessage(msg);
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void ClientTask::numericReply_372(Message msg)
{
	CHECK_ARGS(1, 1);

	#warning FIXME remove the "- " in front.
	postMOTDEvent(msg);
}

/* 375: ":- <server> Message of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */
void ClientTask::numericReply_375(Message msg)
{
	CHECK_ARGS(1, 1);

	postMOTDEvent(msg);
}

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void ClientTask::numericReply_376(Message msg)
{
	CHECK_ARGS(1, 1);

	postMOTDEvent(msg);
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void ClientTask::numericReply_401(Message msg)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 404: "<channel name> :Cannot send to channel"
 */
void ClientTask::numericReply_404(Message msg)
{
	CHECK_ARGS(2, 2);

	postErrorEvent(msg, i18n("You cannot send message to channel %1.", msg.arg(1)));
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void ClientTask::numericReply_406(Message msg)
{
	CHECK_ARGS(2, 2);

	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 422: ":MOTD File is missing"
 *
 * Server's MOTD file could not be opened by the server.
 */
void ClientTask::numericReply_422(Message msg)
{
	CHECK_ARGS(1, 1);

	postErrorEvent(msg);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void ClientTask::numericReply_433(Message msg)
{
	CHECK_ARGS(2, 2);

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
	postErrorEvent(msg, i18n("Nickname %1 is already in use.", msg.arg(1)));
}

/* 442: "<channel> :You're not on that channel"
 */
void ClientTask::numericReply_442(Message msg)
{
	CHECK_ARGS(2, 2);

	postErrorEvent(msg, i18n("You are not on channel %1.", msg.arg(1)));
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void ClientTask::numericReply_464(Message msg)
{
	CHECK_ARGS(1, 1)

	/* Server need pass.. Call disconnect*/
//	emit incomingFailedServerPassword();
	postErrorEvent(msg, i18n("Incorrect password."));
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void ClientTask::numericReply_471(Message msg)
{
	CHECK_ARGS(2, 2)

	postErrorEvent(msg, i18n("Cannot join %1, channel is full.", msg.arg(1)) );
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void ClientTask::numericReply_473(Message msg)
{
	CHECK_ARGS(2, 2)

	postErrorEvent(msg, i18n("Cannot join %1, channel is invite only.", msg.arg(1)) );
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void ClientTask::numericReply_474(Message msg)
{
	CHECK_ARGS(2, 2)

	postErrorEvent(msg, i18n("Cannot join %1, you are banned from that channel.", msg.arg(1)) );
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void ClientTask::numericReply_475(Message msg)
{
	CHECK_ARGS(2, 2)

	postErrorEvent(msg, i18n("Cannot join %1, wrong channel key was given.", msg.arg(1)) );
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void ClientTask::numericReply_477(Message msg)
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

#if 0
//#ifndef KIRC_STRICT

/*
void ClientTask::bindCtcp()
{
	bindCtcpQuery("ACTION",		this, SLOT(CtcpQuery_action(Message &)),
		-1,	-1);
	bindCtcpQuery("CLIENTINFO",	this, SLOT(CtcpQuery_clientinfo(Message &)),
		-1,	1);
	bindCtcpQuery("DCC",		this, SLOT(CtcpQuery_dcc(Message &)),
		4,	5);
	bindCtcpQuery("FINGER",		this, SLOT(CtcpQuery_finger(Message &)),
		-1,	0);
	bindCtcpQuery("PING",		this, SLOT(CtcpQuery_ping(Message &)),
		1,	1);
	bindCtcpQuery("SOURCE",		this, SLOT(CtcpQuery_source(Message &)),
		-1,	0);
	bindCtcpQuery("TIME",		this, SLOT(CtcpQuery_time(Message &)),
		-1,	0);
	bindCtcpQuery("USERINFO",	this, SLOT(CtcpQuery_userinfo(Message &)),
		-1,	0);
	bindCtcpQuery("VERSION",	this, SLOT(CtcpQuery_version(Message &)),
		-1,	0);

	bindCtcpReply("ERRMSG",		this, SLOT(CtcpReply_errmsg(Message &)),
		1,	-1);
	bindCtcpReply("PING",		this, SLOT(CtcpReply_ping(Message &)),
		1,	1,	"");
	bindCtcpReply("VERSION",	this, SLOT(CtcpReply_version(Message &)),
		-1,	-1,	"");
}
*/

void ClientTask::CtcpQuery_action(Message msg)
{
/*	QString target = msg.arg(0);
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(target, msg, msg.ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(msg, target, msg.ctcpMessage().ctcpRaw());*/
}

/*
NO REPLY EXIST FOR THE CTCP ACTION COMMAND !
bool ClientTask::CtcpReply_action(Message msg)
{
}
*/

//	FIXME: the API can now answer to help commands.
void ClientTask::CtcpQuery_clientinfo(Message msg)
{
	QString clientinfo = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), QString::null, clientinfo);
}

void ClientTask::CtcpQuery_dcc(Message msg)
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
			kDebug(14120) << "Starting DCC chat window." << endl;
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
			kDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1) << endl;
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
bool ClientTask::CtcpReply_dcc(Message msg)
{
}
*/

void ClientTask::CtcpReply_errmsg(Message /*msg*/)
{
	// should emit one signal
}

void ClientTask::CtcpQuery_finger( Message /*msg*/)
{
	// To be implemented
}

void ClientTask::CtcpQuery_ping(Message msg)
{
//	writeCtcpReplyMessage(	msg.prefix(), QString::null,
//				msg.ctcpMessage().command(), msg.ctcpMessage().arg(0));
}

void ClientTask::CtcpReply_ping(Message msg)
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

void ClientTask::CtcpQuery_source(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), m_SourceString);
}

void ClientTask::CtcpQuery_time(Message msg)
{
//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QDateTime::currentDateTime().toString(),
//			      QString::null, false);
}

void ClientTask::CtcpQuery_userinfo(Message msg)
{
//	QString userinfo = m_UserString;

//	writeCtcpReplyMessage(msg.prefix(), QString::null,
//			      msg.ctcpMessage().command(), QString::null, userinfo);
}

void ClientTask::CtcpQuery_version(Message msg)
{
//	QString response = m_VersionString;

//	writeCtcpReplyMessage(msg.prefix(),
//		msg.ctcpMessage().command() + " " + response);
}

void ClientTask::CtcpReply_version(Message msg)
{
//	emit incomingCtcpReply(msg.ctcpMessage().command(), msg.prefix(), msg.ctcpMessage().ctcpRaw());
}

#endif
