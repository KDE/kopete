/*
    kircclientcommands.cpp - IRC Client Commands

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@wanadoo.fr>

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

#include "kirci18ntask.moc"

#include "kircclientsocket.h"

#include <kdebug.h>
#include <klocale.h>

#include <QDateTime>
#include <qfileinfo.h>
#include <qregexp.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*
class I18nTask::Private
{
public:
};
*/
using namespace KIrc;

I18nTask::I18nTask(QObject *parent)
	: Task(parent)
	, d(0)
{
}

I18nTask::~I18nTask()
{
//	delete d;
}

void I18nTask::postServerEvent(const Event *e, const QString &message)
{
//	event.setFrom(e->message().prefix());
//	event.setTo(e->message().socket().server());
//	event.setCc(e->message().socket().owner()); // ??

//	if (message->message().isEmpty())
//		e->setText(e->message().suffix());
//	else
//		e->setText(message);

//	post the message here ... still dunno where, socket ?
}

/*
void I18nTask::registerStandardCommands(CommandManager *cm)
{
	cm->registerCommand(ERROR,	this, SLOT(error(Event *&));
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
void I18nTask::error(Event * /*e*/)
{
//	e->client->close();
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
void I18nTask::join(Event *e)
{
/*
	return Event("Join", i18n("%1 has joined %2"), who, channel);
*/
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
void I18nTask::kick(Event *e)
{
/*
	return Event("Kick", i18n("%1 has kicked %2 from %3 (%4)", who, victims, channel, reason));
*/
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
void I18nTask::mode(Event *e)
{
/*
	QStringList args = e->message().argList();
	args.pop_front();

	Entity::Ptr fromEntity = e->message().entityFromPrefix();
	Entity::Ptr toEntity = e->message().entityFromArg(0)

	emit receivedMessage(
		Info,
		fromEntity,
		Entity::List::null,
		i18n(""));
*/
}

/* Nick name of a user changed
 * "<nickname>"
 */
void I18nTask::nick(Event *e)
{
/*
	return Event("Nick Changed", i18n("%1 is now knwon as %2"), oldnick, newnick);
*/
}

void I18nTask::notice(Event *e)
{
//	return Event("Notice", i18n(""), who, message);
}

/* This signal emits when a user parts a channel
 * "<channel> *( "," <channel> ) [ <Part Message> ]"
 */
void I18nTask::part(Event *e)
{
/*
	emit receivedMessage(
		PartMessage,
		e->message().entityFromPrefix(),
		e->message().entityFromArg(0),
		e->message().suffix());
*/
}

void I18nTask::ping(Event *e)
{
//	e->setCommand(PONG);
//	e->setArgs(e->message().rawArg(0));
//	e->setSuffix(e->message().rawSuffix());

//	e->client->writeMessage(reply);
}

void I18nTask::pong(Event * /*e*/)
{
}

void I18nTask::privmsg(Event *e)
{
	if (!e->message().suffix().isEmpty())
	{
/*
		emit receivedMessage(
			PrivateMessage,
			e->message().entityFromPrefix(),
			e->message().entityFromArg(0),
			e->message().suffix());
*/
	}
//#ifndef KIRC_STRICT
#if 0
	if (e->message().hasCtcpMessage())
	{
//		invokeCtcpCommandOfMessage(m_ctcpQueries, e);
	}
#endif
}

void I18nTask::quit(Event *e)
{
//	e->setText(i18n("%1 has quit: %2").arg(e->message().prefixEntity(), e->message().suffix()));
}

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
void I18nTask::topic(Event *e)
{
//	e->setText(i18n("%1 changed %2 topic to: %3").arg(e->message().prefixEntity(), e->message().arg(0), e->message().suffix()));
}


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void I18nTask::bindNumericReplies()
{
	bind(1, this, SLOT(numericReply_001(Event *)), 1, 1);
	bind(2, this, SLOT(numericReply_002(Event *)), 1, 1);
	bind(3, this, SLOT(numericReply_003(Event *)), 1, 1);
	bind(4, this, SLOT(numericReply_004(Event *)), 5, 5);
	bind(5, this, SLOT(numericReply_004(Event *)), 1, 1);

	bind(250, this, SLOT(numericReply_250(Event *)));
	bind(251, this, SLOT(numericReply_251(Event *)));
	bind(252, this, SLOT(numericReply_252(Event *)), 2, 2);
	bind(253, this, SLOT(numericReply_253(Event *)), 2, 2);
	bind(254, this, SLOT(numericReply_254(Event *)), 2, 2);
	bind(255, this, SLOT(numericReply_255(Event *)), 1, 1);

	bind(263, this, SLOT(numericReply_263(Event *)));
	bind(265, this, SLOT(numericReply_265(Event *)));
	bind(266, this, SLOT(numericReply_266(Event *)));

	bind(301, this, SLOT(numericReply_301(Event *)), 2, 2);
	bind(303, this, SLOT(numericReply_303(Event *)), 1, 1);
//	bind(305, this, SLOT(ignoreMessage(Event *)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(Event *)), 0, 0 );
	bind(307, this, SLOT(numericReply_307(Event *)), 1, 1);
	bind(311, this, SLOT(numericReply_311(Event *)), 5, 5);
	bind(312, this, SLOT(numericReply_312(Event *)), 3, 3);
	bind(313, this, SLOT(numericReply_313(Event *)), 2, 2);
	bind(314, this, SLOT(numericReply_314(Event *)), 5, 5);
	bind(315, this, SLOT(numericReply_315(Event *)), 2, 2);
	bind(317, this, SLOT(numericReply_317(Event *)), 3, 4);
	bind(318, this, SLOT(numericReply_318(Event *)), 2, 2);
	bind(319, this, SLOT(numericReply_319(Event *)), 2, 2);
	bind(320, this, SLOT(numericReply_320(Event *)), 2, 2);
//	bind(321, this, SLOT(ignoreMessage(Event *)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(Event *)), 3, 3);
	bind(323, this, SLOT(numericReply_323(Event *)), 1, 1);
	bind(324, this, SLOT(numericReply_324(Event *)), 2, 4);
	bind(328, this, SLOT(numericReply_328(Event *)), 2, 2);
	bind(329, this, SLOT(numericReply_329(Event *)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(Event *)), 3, 3); // ???
	bind(331, this, SLOT(numericReply_331(Event *)), 2, 2);
	bind(332, this, SLOT(numericReply_332(Event *)), 2, 2);
	bind(333, this, SLOT(numericReply_333(Event *)), 4, 4);
	bind(352, this, SLOT(numericReply_352(Event *)), 5, 10);
	bind(353, this, SLOT(numericReply_353(Event *)), 3, 3);
	bind(366, this, SLOT(numericReply_366(Event *)), 2, 2);
	bind(369, this, SLOT(numericReply_369(Event *)), 2, 2);
	bind(372, this, SLOT(numericReply_372(Event *)), 1, 1);
	bind(375, this, SLOT(ignoreMessage(Message&)), 0, 0 );
	bind(376, this, SLOT(ignoreMessage(Message&)), 0, 0 );

	bind(401, this, SLOT(numericReply_401(Event *)), 2, 2);
	bind(404, this, SLOT(numericReply_404(Event *)), 2, 2);
	bind(406, this, SLOT(numericReply_406(Event *)), 2, 2);
	bind(422, this, SLOT(numericReply_422(Event *)), 1, 1);
	bind(433, this, SLOT(numericReply_433(Event *)), 2, 2);
	bind(442, this, SLOT(numericReply_442(Event *)), 2, 2);
	bind(464, this, SLOT(numericReply_464(Event *)), 1, 1);
	bind(471, this, SLOT(numericReply_471(Event *)), 2, 2);
	bind(473, this, SLOT(numericReply_473(Event *)), 2, 2);
	bind(474, this, SLOT(numericReply_474(Event *)), 2, 2);
	bind(475, this, SLOT(numericReply_475(Event *)), 2, 2);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(postServerEvent(Event *)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void I18nTask::numericReply_001(Event *e)
{
	kDebug(14121) ;

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	postServerEvent(e);
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given information are close to 004.
 */
void I18nTask::numericReply_002(Event *e)
{
	postServerEvent(e);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void I18nTask::numericReply_003(Event *e)
{
	postServerEvent(e);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void I18nTask::numericReply_004(Event *e)
{
//	emit postServerEvent(e->message().arg(1),e->message().arg(2),e->message().arg(3),e->message().arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void I18nTask::numericReply_005(Event *e)
{
	postServerEvent(e);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void I18nTask::numericReply_250(Event *e)
{
	postServerEvent(e);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void I18nTask::numericReply_251(Event *e)
{
	postServerEvent(e);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void I18nTask::numericReply_252(Event *e)
{
	postServerEvent(e, i18np("There is 1 operator online.", "There are %1 operators online.", e->message().arg(1).toULong()));
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void I18nTask::numericReply_253(Event *e)
{
	postServerEvent(e, i18np("There is 1 unknown connection.", "There are %1 unknown connections.", e->message().arg(1).toULong()));
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void I18nTask::numericReply_254(Event *e)
{
	postServerEvent(e, i18np("There has been 1 channel formed.",
		"There have been %1 channels formed.",
		e->message().arg(1).toULong()));
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void I18nTask::numericReply_255(Event *e)
{
	postServerEvent(e);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void I18nTask::numericReply_263(Event *e)
{
	postServerEvent(e, i18n("Server was too busy to execute %1.", e->message().arg(1)));
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void I18nTask::numericReply_265(Event *e)
{
	postServerEvent(e);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void I18nTask::numericReply_266(Event *e)
{
	postServerEvent(e);
}

/* 301: "<nick> :<away message>"
 */
void I18nTask::numericReply_301(Event *e)
{
/*
	Entity entity = e->message().entityFromArg(1);
	entity->setAwayMessage(e->message().suffix);
	entity->setMode("+a");

	postServerEvent(e);
*/
}

/* 303: ":*1<nick> *(" " <nick> )"
 */
void I18nTask::numericReply_303(Event *e)
{
/*
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), e->message().suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).trimmed().isEmpty())
			emit incomingUserOnline(*it);
	}
*/
}

/* 305: ":You are no longer marked as being away"
 */
void I18nTask::numericReply_305(Event *e)
{
/*
	Entity::Ptr self = this->self();
	self->setAwayMessage(QString());
//	self->setModes("-a");
	postServerEvent(e, i18n("You are no longer marked as being away."));
*/
}


/* 306: ":You have been marked as being away"
 */
void I18nTask::numericReply_306(Event *e)
{
//	Entity::Ptr self = d->client->owner();
//	self->setModes("+a");
	postServerEvent(e, i18n("You have been marked as being away."));
}

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
void I18nTask::numericReply_307(Event *e)
{
	postServerEvent(e, i18n("%1 is a registered nick.", e->message().arg(1)));
}

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void I18nTask::numericReply_311(Event *e)
{
//	emit incomingWhoIsUser(e->message().arg(1), e->message().arg(2), e->message().arg(3), e->message().suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void I18nTask::numericReply_312(Event *e)
{
//	emit incomingWhoIsServer(e->message().arg(1), e->message().arg(2), e->message().suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void I18nTask::numericReply_313(Event *e)
{
	postServerEvent(e, i18n("%1 is an IRC operator.", e->message().arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void I18nTask::numericReply_314(Event *e)
{
//	emit incomingWhoWasUser(e->message().arg(1), e->message().arg(2), e->message().arg(3), e->message().suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
void I18nTask::numericReply_315(Event *e)
{
	postServerEvent(e);
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void I18nTask::numericReply_317(Event *e)
{
/*
	emit incomingWhoIsIdle(e->message().arg(1), e->message().arg(2).toULong());
	if (e->message().argsSize()==4)
		emit incomingSignOnTime(e->message().arg(1),e->message().arg(3).toULong());
*/
}

/* 318: "<nick>{<space><realname>} :End of /WHOIS list"
 * End of WHOIS for a given nick.
 */
void I18nTask::numericReply_318(Event *e)
{
	emit postServerEvent(e);
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void I18nTask::numericReply_319(Event *e)
{
//	emit incomingWhoIsChannels(e->message().arg(1), e->message().suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void I18nTask::numericReply_320(Event *e)
{
//	emit incomingWhoIsIdentified(e->message().arg(1));
}

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
void I18nTask::numericReply_322(Event *e)
{
//	emit incomingListedChan(e->message().arg(1), e->message().arg(2).toUInt(), e->message().suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void I18nTask::numericReply_323(Event *e)
{
	emit postServerEvent(e);
}

/* 324: "<channel> <mode> <mode params>"
 */
void I18nTask::numericReply_324(Event *e)
{
//	emit incomingChannelMode(e->message().arg(1), e->message().arg(2), e->message().arg(3));
}

/* 328:
 */
void I18nTask::numericReply_328(Event *e)
{
//	emit incomingChannelHomePage(e->message().arg(1), e->message().suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void I18nTask::numericReply_329(Event * /*e*/)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void I18nTask::numericReply_331(Event * /*e*/)
{
//	emit incomingExistingTopic(e->message().arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void I18nTask::numericReply_332(Event *e)
{
//	emit incomingExistingTopic(e->message().arg(1), e->message().suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void I18nTask::numericReply_333(Event *e)
{
/*
	QDateTime d;
	d.setTime_t( e->message().arg(3).toLong() );
	emit incomingTopicUser(e->message().arg(1), e->message().arg(2), d );
*/
}

/* 352:
 * WHO Reply
 */
void I18nTask::numericReply_352(Event *e)
{
/*
	QStringList suffix = QStringList::split( ' ', e->message().suffix() );

	emit incomingWhoReply(
		e->message().arg(5),
		e->message().arg(1),
		e->message().arg(2),
		e->message().arg(3),
		e->message().arg(4),
		e->message().arg(6)[0] != 'H',
		e->message().arg(7),
		e->message().suffix().section(' ', 0, 1 ).toUInt(),
		e->message().suffix().section(' ', 1 )
	);
*/
}


/* 353:
 * NAMES list
 */
void I18nTask::numericReply_353(Event *e)
{
//	emit incomingNamesList(e->message().arg(2), QStringList::split(' ', e->message().suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void I18nTask::numericReply_366(Event *e)
{
	emit postServerEvent(e);
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void I18nTask::numericReply_369(Event *e)
{
	emit postServerEvent(e);
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void I18nTask::numericReply_372(Event *e)
{
	#warning FIXME remove the "- " in front.
	postServerEvent(e);
}

/* 375: ":- <server> Event *of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */
void I18nTask::numericReply_375(Event *e)
{
	postServerEvent(e);
}

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void I18nTask::numericReply_376(Event *e)
{
	postServerEvent(e);
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void I18nTask::numericReply_401(Event *e)
{
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 404: "<channel name> :Cannot send to channel"
 */
void I18nTask::numericReply_404(Event *e)
{
	postServerEvent(e, i18n("You cannot send message to channel %1.", e->message().arg(1)));
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void I18nTask::numericReply_406(Event *e)
{
	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 422: ":MOTD File is missing"
 *
 * Server's MOTD file could not be opened by the server.
 */
void I18nTask::numericReply_422(Event *e)
{
	postServerEvent(e);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void I18nTask::numericReply_433(Event *e)
{
//	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
//		m_FailedNickOnLogin = true;
//		emit incomingFailedNickOnLogin(e->message().arg(1));
	}
//	else
//	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
//		emit incomingNickInUse(e->message().arg(1));
//	}
	postServerEvent(e, i18n("Nickname %1 is already in use.", e->message().arg(1)));
}

/* 442: "<channel> :You're not on that channel"
 */
void I18nTask::numericReply_442(Event *e)
{
	postServerEvent(e, i18n("You are not on channel %1.", e->message().arg(1)));
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void I18nTask::numericReply_464(Event *e)
{
	/* Server need pass.. Call disconnect*/
//	emit incomingFailedServerPassword();
	postServerEvent(e, i18n("Incorrect password."));
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void I18nTask::numericReply_471(Event *e)
{
	postServerEvent(e, i18n("Cannot join %1, channel is full.", e->message().arg(1)) );
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void I18nTask::numericReply_473(Event *e)
{
	postServerEvent(e, i18n("Cannot join %1, channel is invite only.", e->message().arg(1)) );
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void I18nTask::numericReply_474(Event *e)
{
	postServerEvent(e, i18n("Cannot join %1, you are banned from that channel.", e->message().arg(1)) );
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void I18nTask::numericReply_475(Event *e)
{
	postServerEvent(e, i18n("Cannot join %1, wrong channel key was given.", e->message().arg(1)) );
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void I18nTask::numericReply_477(Event *e)
// {
// 	emit incomingChannelNeedRegistration(e->message().arg(2), e->message().suffix());
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
void I18nTask::bindCtcp()
{
	bindCtcpQuery("ACTION",		this, SLOT(CtcpQuery_action(Event *&)),
		-1,	-1);
	bindCtcpQuery("CLIENTINFO",	this, SLOT(CtcpQuery_clientinfo(Event *&)),
		-1,	1);
	bindCtcpQuery("DCC",		this, SLOT(CtcpQuery_dcc(Event *&)),
		4,	5);
	bindCtcpQuery("FINGER",		this, SLOT(CtcpQuery_finger(Event *&)),
		-1,	0);
	bindCtcpQuery("PING",		this, SLOT(CtcpQuery_ping(Event *&)),
		1,	1);
	bindCtcpQuery("SOURCE",		this, SLOT(CtcpQuery_source(Event *&)),
		-1,	0);
	bindCtcpQuery("TIME",		this, SLOT(CtcpQuery_time(Event *&)),
		-1,	0);
	bindCtcpQuery("USERINFO",	this, SLOT(CtcpQuery_userinfo(Event *&)),
		-1,	0);
	bindCtcpQuery("VERSION",	this, SLOT(CtcpQuery_version(Event *&)),
		-1,	0);

	bindCtcpReply("ERRMSG",		this, SLOT(CtcpReply_erre(Event *&)),
		1,	-1);
	bindCtcpReply("PING",		this, SLOT(CtcpReply_ping(Event *&)),
		1,	1,	"");
	bindCtcpReply("VERSION",	this, SLOT(CtcpReply_version(Event *&)),
		-1,	-1,	"");
}
*/

void I18nTask::CtcpQuery_action(Event *e)
{
/*	QString target = e->message().arg(0);
	if (target[0] == '#' || target[0] == '!' || target[0] == '&')
		emit incomingAction(target, e, e->message().ctcpMessage().ctcpRaw());
	else
		emit incomingPrivAction(e, target, e->message().ctcpMessage().ctcpRaw());*/
}

/*
NO REPLY EXIST FOR THE CTCP ACTION COMMAND !
bool I18nTask::CtcpReply_action(Event *e)
{
}
*/

//	FIXME: the API can now answer to help commands.
void I18nTask::CtcpQuery_clientinfo(Event *e)
{
	QString clientinfo = QString::fromLatin1("The following commands are supported, but "
			"without sub-command help: VERSION, CLIENTINFO, USERINFO, TIME, SOURCE, PING,"
			"ACTION.");

//	writeCtcpReplyMessage(	e->message().prefix(), QString(),
//				e->message().ctcpMessage().command(), QString(), clientinfo);
}

void I18nTask::CtcpQuery_dcc(Event *e)
{
//	Event *&ctcpMsg = e->message().ctcpMessage();
	Event *ctcpMsg;

	QString dccCommand = ctcpMsg.arg(0).toUpper();

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
			kDebug(14120) << "Starting DCC chat window.";
//			TransferHandler::self()->createClient(
//				this, e->message().prefix(),
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
//		QFileInfo realfile(e->message().arg(1));
		QHostAddress address(ctcpMsg.arg(2).toUInt(&okayHost));
		unsigned int port = ctcpMsg.arg(3).toUInt(&okayPort);
		unsigned int size = ctcpMsg.arg(4).toUInt(&okaySize);
		if (okayHost && okayPort && okaySize)
		{
			kDebug(14120) << "Starting DCC send file transfert for file:" << ctcpMsg.arg(1);
//			TransferHandler::self()->createClient(
//				this, e->message().prefix(),
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
bool I18nTask::CtcpReply_dcc(Event *e)
{
}
*/

void I18nTask::CtcpReply_erre(Event * /*e*/)
{
	// should emit one signal
}

void I18nTask::CtcpQuery_finger(Event * /*e*/)
{
	// To be implemented
}

void I18nTask::CtcpQuery_ping(Event *e)
{
//	writeCtcpReplyMessage(	e->message().prefix(), QString(),
//				e->message().ctcpMessage().command(), e->message().ctcpMessage().arg(0));
}

void I18nTask::CtcpReply_ping(Event *e)
{
/*	timeval time;
	if (gettimeofday(&time, 0) == 0)
	{
		// FIXME: the time code is wrong for usec
		QString timeReply = QString::fromLatin1("%1.%2").arg(time->message().tv_sec).arg(time->message().tv_usec);
		double newTime = timeReply.toDouble();
		double oldTime = e->message().suffix().section(' ',0, 0).toDouble();
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

		emit incomingCtcpReply(QString::fromLatin1("PING"), e->message().prefix(), diffString);
	}
//	else
//		((MessageRedirector *)sender())->error("failed to get current time");*/
}

void I18nTask::CtcpQuery_source(Event *e)
{
//	writeCtcpReplyMessage(e->message().prefix(), QString(),
//			      e->message().ctcpMessage().command(), m_SourceString);
}

void I18nTask::CtcpQuery_time(Event *e)
{
//	writeCtcpReplyMessage(e->message().prefix(), QString(),
//			      e->message().ctcpMessage().command(), QDateTime::currentDateTime().toString(),
//			      QString(), false);
}

void I18nTask::CtcpQuery_userinfo(Event *e)
{
//	QString userinfo = m_UserString;

//	writeCtcpReplyMessage(e->message().prefix(), QString(),
//			      e->message().ctcpMessage().command(), QString(), userinfo);
}

void I18nTask::CtcpQuery_version(Event *e)
{
//	QString response = m_VersionString;

//	writeCtcpReplyMessage(e->message().prefix(),
//		e->message().ctcpMessage().command() + " " + response);
}

void I18nTask::CtcpReply_version(Event *e)
{
//	emit incomingCtcpReply(e->message().ctcpMessage().command(), e->message().prefix(), e->message().ctcpMessage().ctcpRaw());
}

#endif
