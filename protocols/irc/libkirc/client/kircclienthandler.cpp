/*
    kircclienteventhandler.cpp - IRC Client Message Handler

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

#include "kircclienthandler.moc"

#include "kircclientsocket.h"

#include "kirccontext.h"
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

class KIrc::ClientEventHandlerPrivate
{
public:
	KIrc::Context *context;
};

using namespace KIrc;

ClientEventHandler::ClientEventHandler(Context *context)
	: EventHandler(parent)
	, d_ptr(new ClientEventHandlerPrivate)
{
	Q_D(ClientEventHandler);

	d->context = context;
}

ClientEventHandler::~ClientEventHandler()
{
	delete d_ptr;
}

void ClientEventHandler::postEvent(QEvent *event)
{
//	post the message here ... still dunno where, socket ?
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, QString &text)
{
	Q_D(ClientEventHandler);

	Message msg = ev->message();
	from = d->context->entityFromName(msg.prefix());
	if (text.isEmpty())
		text = msg.suffix();

	if (from) {
//		postEvent(new TextEvent(eventId, from, to, text));
		return true;
	}
	return false;
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, QString &text)
{
	Q_D(ClientEventHandler);

	Message msg = ev->message();
	from = d->context->entityFromName(msg.prefix());
	to = d->context->entitiesFromNames(msg.argAt(1));
	if (text.isEmpty())
		text = msg.suffix();

	if (from && (to.count() > 0)) {
		postEvent(new TextEvent(eventId, from, to, text));
		return true;
	}
	return false;
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, Entity::List &victims, QString &text)
{
	Q_D(ClientEventHandler);

	Message msg = ev->message();
	from = d->context->entityFromName(msg.prefix());
	to = d->context->entitiesFromNames(msg.argAt(1));
	victims = d->context->entitiesFromNames(msg.argAt(2));
	if (text.isEmpty())
		text = msg.suffix();

	if (from && (to.count() > 0) && (victims.count() > 0)) {
		postEvent(new TextEvent(eventId, from, to, text));
		return true;
	}
	return false;
}

bool ClientEventHandler::postServerInfoEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Entity::Ptr &from; Entity::List &to;
	return postEvent(ev, "ServerInfo", from, to);
}

/*
void ClientEventHandler::postEvent(const MessageEvent *&msg, Event::MessageType messageType, const QString &message)
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

void ClientEventHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	receivedServerMessage(msg, msg.suffix());
}

void ClientEventHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QString &message)
{
//	emit receivedMessage(InfoMessage, msg.prefix(), Entity::List(), message);
}

// FIXME: Really handle this message
void ClientEventHandler::error(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::join(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(0, 0);

	Message msg = ev->message();
	Entity::Ptr from = d->context->entityFromName(msg.prefix());

#if 0
	if (msg.prefix() == QLatin1String("0"))
		// leave all the channels
	else
#endif
	{
		// For now lets forgot about the keys
		Entity::List channels = d->context->entitiesFromNames(msg.prefix());
		foreach(Entity::Ptr channel, channels)
		{
//			Event e;

//			channel->add(from);
		}
	}
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
void ClientEventHandler::kick(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Q_D(ClientEventHandler);

//	CHECK_ARGS(2, 2);

	Entity::Ptr from;
	Entity::List channels, users;

	if (postEvent(ev, "Kick", from, channels, users)) {
//		foreach(channel, channels)
//			channel->remove(users);
	}
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
void ClientEventHandler::mode(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Q_D(ClientEventHandler);

	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List to;
	QString modeDelta; // = msg.argAt(2); 

	if (postEvent(ev, "Mode", from, to, modeDelta)) {
//		to->setMode(modeDelta);
	}
}

/* Nick name of a user changed
 * "<nickname>"
 */
void ClientEventHandler::nick(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr from = d->context->entityFromName(msg.prefix());
//	QString newNick/oldNick = msg.arg(1);
*/
}

/* Do not support CTCP here, just do the simple message handling.
 */
void ClientEventHandler::notice(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List to;
	QString text;

	postEvent(ev, "Notice", from, to, text);
}

/* This signal emits when a user parts a channel
 * "<channel> *( "," <channel> ) [ <Part Message> ]"
 */
void ClientEventHandler::part(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List channels;
	QString text;

	if (postEvent(ev, "Part", from, channels, text)) {
//		foreach(channel, channels)
//			channel->remove(from);
	}
}

void ClientEventHandler::ping(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(0, 0);

#if 0
	MessageEvent *reply;
//	reply.setCommand(PONG);
//	reply.setArgs(msg.rawArg(0));
	reply.setSuffix(msg.rawSuffix());

//	msg->client->writeMessage(reply);
#endif
}

void ClientEventHandler::pong(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(0, 0);

}

/* Do not support CTCP here, just do the simple message handling.
 */
void ClientEventHandler::privmsg(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List to;
	QString text;

	postEvent(ev, "PrivMsg", from, to, text);
}

void ClientEventHandler::quit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(0, 0);

	Entity::Ptr from;
	QString message;

	if (postEvent(ev, "Quit", from, message)) {
//		d->context->remove(from);
	}
}

/*
void CLientCommands::squit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);
}
*/

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
void ClientEventHandler::topic(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List channel;
	QString topic;

	if (postEvent(ev, "Topic", from, channel, topic)) {
//		channel->set(Topic, topic);
	}
}


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void ClientEventHandler::bindNumericReplies()
{
	bind(263, this, SLOT(numericReply_263(MessageEvent *&)));
	bind(265, this, SLOT(numericReply_265(MessageEvent *&)));
	bind(266, this, SLOT(numericReply_266(MessageEvent *&)));

//	bind(305, this, SLOT(ignoreMessage(MessageEvent *&)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(MessageEvent *&)), 0, 0 );
	bind(312, this, SLOT(numericReply_312(MessageEvent *&)), 3, 3);
	bind(313, this, SLOT(numericReply_313(MessageEvent *&)), 2, 2);
	bind(314, this, SLOT(numericReply_314(MessageEvent *&)), 5, 5);
	bind(315, this, SLOT(numericReply_315(MessageEvent *&)), 2, 2);
	bind(317, this, SLOT(numericReply_317(MessageEvent *&)), 3, 4);
	bind(318, this, SLOT(numericReply_318(MessageEvent *&)), 2, 2);
	bind(319, this, SLOT(numericReply_319(MessageEvent *&)), 2, 2);
	bind(320, this, SLOT(numericReply_320(MessageEvent *&)), 2, 2);
//	bind(321, this, SLOT(ignoreMessage(MessageEvent *&)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(MessageEvent *&)), 3, 3);
	bind(323, this, SLOT(numericReply_323(MessageEvent *&)), 1, 1);
	bind(324, this, SLOT(numericReply_324(MessageEvent *&)), 2, 4);
	bind(328, this, SLOT(numericReply_328(MessageEvent *&)), 2, 2);
	bind(329, this, SLOT(numericReply_329(MessageEvent *&)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(MessageEvent *&)), 3, 3); // ???
	bind(331, this, SLOT(numericReply_331(MessageEvent *&)), 2, 2);
	bind(332, this, SLOT(numericReply_332(MessageEvent *&)), 2, 2);
	bind(352, this, SLOT(numericReply_352(MessageEvent *&)), 5, 10);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(Message&)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
void ClientEventHandler::numericReply_001(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

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
void ClientEventHandler::numericReply_002(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(msg);
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
void ClientEventHandler::numericReply_003(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(msg);
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
void ClientEventHandler::numericReply_004(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(5, 5);

//	emit incomingHostInfo(msg.arg(1),msg.arg(2),msg.arg(3),msg.arg(4));
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
void ClientEventHandler::numericReply_005(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(?, ?);

//	receivedServerMessage(msg);
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientEventHandler::numericReply_250(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(1, 1);

	postServerInfoEvent(ev);
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
void ClientEventHandler::numericReply_251(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(1, 1);

	postServerInfoEvent(ev);
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
void ClientEventHandler::numericReply_252(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(2, 2);

	bool ok = false;
	Entity::Ptr from; Entity::Ptr to;
	QString text = i18np("There is %1 operator online.", "There are %1 operators online.", ev->message().argAt(1).toULong(&ok));

	if (ok)
		postEvent(ev, "OperatorsOnline", from, to, text);
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
void ClientEventHandler::numericReply_253(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	bool ok = false;
	Entity::Ptr from; Entity::List to;
	QString text = i18np("There is %1 unknown connection.", "There are %1 unknown connections.", ev->message().argAt(1).toULong(&ok));

	if (ok)
		postEvent(ev, "UnkownConnections", from, to, text);
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
void ClientEventHandler::numericReply_254(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	bool ok = false;
	Entity::Ptr from; Entity::List to;
	QString text = i18np("There has been %1 channel formed.", "There have been %1 channels formed.", ev->message().argAt(1).toULong(&ok));

	if (ok)
		postEvent(ev, "ChannelsFormed", from, to, text);	
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
void ClientEventHandler::numericReply_255(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	postServerInfoEvent(ev);
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
void ClientEventHandler::numericReply_263(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Entity::Ptr from; Entity::List to;
	QString text = i18n("Server was too busy to execute %1.", ev->message().argAt(1));

	postEvent(ev, "ServerTooBusy", from, to, text);
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
void ClientEventHandler::numericReply_265(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	postServerInfoEvent(ev);
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
void ClientEventHandler::numericReply_266(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	postServerInfoEvent(ev);
}

/* 301: "<nick> :<away message>"
 */
void ClientEventHandler::numericReply_301(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_303(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_305(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_306(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Entity::Ptr from; Entity::List to;

	if (postEvent(ev, "YouAreAway", from, to, i18n("You have been marked as being away."))) {
//		self->setModes("+a");
	}
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
void ClientEventHandler::numericReply_319(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoIsChannels(msg.arg(1), msg.suffix());
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
void ClientEventHandler::numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_322(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingListedChan(msg.arg(1), msg.arg(2).toUInt(), msg.suffix());
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
void ClientEventHandler::numericReply_323(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(msg);
}

/* 324: "<channel> <mode> <mode params>"
 */
void ClientEventHandler::numericReply_324(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelMode(msg.arg(1), msg.arg(2), msg.arg(3));
}

/* 328:
 */
void ClientEventHandler::numericReply_328(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelHomePage(msg.arg(1), msg.suffix());
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
void ClientEventHandler::numericReply_329(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
void ClientEventHandler::numericReply_331(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(msg.arg(1), suffix);
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
void ClientEventHandler::numericReply_332(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(msg.arg(1), msg.suffix());
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
void ClientEventHandler::numericReply_333(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_352(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_353(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(3, 3);

//	emit incomingNamesList(msg.arg(2), QStringList::split(' ', msg.suffix()));
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
void ClientEventHandler::numericReply_366(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(msg);
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
void ClientEventHandler::numericReply_369(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(msg);
}

/* 372: ":- <text>"
 * Part of the MOTD.
 */
void ClientEventHandler::numericReply_372(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	#warning FIXME remove the "- " in front.
//	postMOTDEvent(msg);
}

/* 375: ":- <server> MessageEvent *of the day - "
 * Beginging the motd. This isn't emitted because the MOTD is sent out line by line.
 */
void ClientEventHandler::numericReply_375(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	postMOTDEvent(msg);
}

/* 376: ":End of MOTD command"
 * End of the motd.
 */
void ClientEventHandler::numericReply_376(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	postMOTDEvent(msg);
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
void ClientEventHandler::numericReply_401(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
}

/* 404: "<channel name> :Cannot send to channel"
 */
void ClientEventHandler::numericReply_404(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(msg, i18n("You cannot send message to channel %1.", msg.arg(2)));
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
void ClientEventHandler::numericReply_406(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientEventHandler::numericReply_422(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	postErrorEvent(msg);
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
void ClientEventHandler::numericReply_433(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
//	postErrorEvent(msg, i18n("Nickname %1 is already in use.", msg.arg(1)));
}

/* 442: "<channel> :You're not on that channel"
 */
void ClientEventHandler::numericReply_442(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(msg, i18n("You are not on channel %1.", msg.arg(1)));
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
void ClientEventHandler::numericReply_464(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1)

	/* Server need pass.. Call disconnect*/
	Entity::Ptr from; Entity::List to;
	QString text = i18n("Password Incorect");

//	postEvent(ev, "PasswordIncorrect", from, to, text);

//	emit incomingFailedServerPassword();
}

/* 465: ":You are banned from this server"
 */

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
void ClientEventHandler::numericReply_471(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(msg, i18n("Cannot join %1, channel is full.", msg.arg(2)) );
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
void ClientEventHandler::numericReply_473(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(msg, i18n("Cannot join %1, channel is invite only.", msg.arg(2)) );
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
void ClientEventHandler::numericReply_474(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(msg, i18n("Cannot join %1, you are banned from that channel.", msg.arg(2)) );
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
void ClientEventHandler::numericReply_475(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(msg, i18n("Cannot join %1, wrong channel key was given.", msg.arg(2)) );
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void ClientEventHandler::numericReply_477(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

