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


#include "kircclientsocket.h"

#include "kircclientmotdhandler.h"

#include "kirccontext.h"
#include "kircentity.h"
#include "kircevent.h"
#include "kirchandler_p.h"

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
	: public KIrc::HandlerPrivate
{
public:
	KIrc::ClientMotdHandler *motdHandler;
};

using namespace KIrc;

ClientEventHandler::ClientEventHandler(QObject* parent)
	: Handler(new ClientEventHandlerPrivate, parent)
{
	Q_D(ClientEventHandler);
	d->motdHandler = new KIrc::ClientMotdHandler(this);
}

ClientEventHandler::~ClientEventHandler()
{
}

KIrc::Handler::Handled ClientEventHandler::onMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	if ( Handler::onMessage(context, message, socket) == NotHandled )
	{
		KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket*>( socket );
		KIrc::TextEvent *event=new KIrc::TextEvent( "NOTHANDLED", client->server(), client->owner(), message.toLine() );
		context->postEvent( event );
	}
	return CoreHandled;
}

#if 0
void ClientEventHandler::postEvent(QEvent *event)
{
//	post the message here ... still dunno where, socket ?
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, QString &text)
{
	Q_D(ClientEventHandler);

	Message message = ev->message();
	from = d->context->entityFromName(message.prefix());
	if (text.isEmpty())
		text = message.suffix();

	if (from) {
//		postEvent(new TextEvent(eventId, from, to, text));
		return true;
	}
	return false;
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, QString &text)
{
	Q_D(ClientEventHandler);

	Message message = ev->message();
	from = d->context->entityFromName(message.prefix());
	to = d->context->entitiesFromNames(message.argAt(1));
	if (text.isEmpty())
		text = message.suffix();

	if (from && (to.count() > 0)) {
		postEvent(new TextEvent(eventId, from, to, text));
		return true;
	}
	return false;
}

bool ClientEventHandler::postEvent(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QByteArray &eventId, Entity::Ptr &from, Entity::List &to, Entity::List &victims, QString &text)
{
	Q_D(ClientEventHandler);

	Message message = ev->message();
	from = d->context->entityFromName(message.prefix());
	to = d->context->entitiesFromNames(message.argAt(1));
	victims = d->context->entitiesFromNames(message.argAt(2));
	if (text.isEmpty())
		text = message.suffix();

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
void ClientEventHandler::postEvent(const MessageEvent *&message, Event::MessageType messageType, const QString &message)
{
	Event event;
	event.setType(messageType);
//	event.setFrom(message.prefix());
//	event.setCc(message.socket().owner()); // server instead ?

	if (message.isEmpty())
		event.setMessage(message.suffix());
	else
		event.setMessage(message);

//	post the message here ... still dunno where, socket ?
}
*/

void ClientEventHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket, const QString &message)
{
}

#endif

void ClientEventHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket*>( socket );
	KIrc::TextEvent *event=new KIrc::TextEvent( "ServerInfo", client->server(), client->owner(), message.suffix() );
	context->postEvent( event );
}


// FIXME: Really handle this message
KIrc::Handler::Handled ClientEventHandler::ERROR(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(0, 0);

//	message->client->close();
	return KIrc::Handler::NotHandled;
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
KIrc::Handler::Handled ClientEventHandler::MODE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);
/*
	CHECK_ARGS(1, 1);

	Entity::Ptr from;
	Entity::List to;
	QString modeDelta; // = message.argAt(2);

	if (postEvent(ev, "Mode", from, to, modeDelta)) {
//		to->setMode(modeDelta);
	}
*/
	return KIrc::Handler::NotHandled;
}

/* Nick name of a user changed
 * "<nickname>"
 */
KIrc::Handler::Handled ClientEventHandler::NICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

/*
	Entity::Ptr from = d->context->entityFromName(message.prefix());
//	QString newNick/oldNick = message.arg(1);
*/
	return KIrc::Handler::NotHandled;
}

/* Do not support CTCP here, just do the simple message handling.
 */
KIrc::Handler::Handled ClientEventHandler::NOTICE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	Entity::Ptr from; Entity::List to;
	QString text;

//	postEvent(ev, "Notice", from, to, text);
	return KIrc::Handler::NotHandled;
}

/* Do not support CTCP here, just do the simple message handling.
 */
KIrc::Handler::Handled ClientEventHandler::PRIVMSG(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(1, 1);

	kDebug( 14120 )<<"privmsg: "<<message.suffix();

	KIrc::TextEvent *event=new KIrc::TextEvent( "PRIVMSG",
												 context->entityFromName( message.prefix() ) ,
												 context->entitiesFromNames( message.argAt( 1 ) ),
												 message.suffix()
											  );

	context->postEvent( event );

	return KIrc::Handler::CoreHandled;
}

KIrc::Handler::Handled ClientEventHandler::QUIT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);
/*
	CHECK_ARGS(0, 0);

	Entity::Ptr from;
	QString message;

	if (postEvent(ev, "Quit", from, message)) {
//		d->context->remove(from);
	}
*/
	return KIrc::Handler::NotHandled;
}

/*
KIrc::Handler::Handled CLientCommands::squit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);
}
*/


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void ClientEventHandler::bindNumericReplies()
{
	bind(263, this, SLOT(numericReply_263(MessageEvent*&)));
	bind(265, this, SLOT(numericReply_265(MessageEvent*&)));
	bind(266, this, SLOT(numericReply_266(MessageEvent*&)));

//	bind(305, this, SLOT(ignoreMessage(MessageEvent*&)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(MessageEvent*&)), 0, 0 );
	bind(312, this, SLOT(numericReply_312(MessageEvent*&)), 3, 3);
	bind(313, this, SLOT(numericReply_313(MessageEvent*&)), 2, 2);
	bind(314, this, SLOT(numericReply_314(MessageEvent*&)), 5, 5);
	bind(315, this, SLOT(numericReply_315(MessageEvent*&)), 2, 2);
	bind(317, this, SLOT(numericReply_317(MessageEvent*&)), 3, 4);
	bind(318, this, SLOT(numericReply_318(MessageEvent*&)), 2, 2);
	bind(319, this, SLOT(numericReply_319(MessageEvent*&)), 2, 2);
	bind(320, this, SLOT(numericReply_320(MessageEvent*&)), 2, 2);
//	bind(321, this, SLOT(ignoreMessage(MessageEvent*&)), 0, 0 );
	bind(322, this, SLOT(numericReply_322(MessageEvent*&)), 3, 3);
	bind(323, this, SLOT(numericReply_323(MessageEvent*&)), 1, 1);
	bind(324, this, SLOT(numericReply_324(MessageEvent*&)), 2, 4);
	bind(328, this, SLOT(numericReply_328(MessageEvent*&)), 2, 2);
	bind(329, this, SLOT(numericReply_329(MessageEvent*&)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(MessageEvent*&)), 3, 3); // ???
	bind(331, this, SLOT(numericReply_331(MessageEvent*&)), 2, 2);
	bind(332, this, SLOT(numericReply_332(MessageEvent*&)), 2, 2);
	bind(352, this, SLOT(numericReply_352(MessageEvent*&)), 5, 10);

	//Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
	bind(477, this, SLOT(receivedServerMessage(Message&)),0,0);
}
*/
/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_001(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	/* At this point we are connected and the server is ready for us to being taking commands
	 * although the MOTD comes *after* this.
	 */
	static_cast<KIrc::ClientSocket*>( socket )->setAuthentified();
	static_cast<KIrc::ClientSocket*>( socket )->server()->setName( message.prefix() );

	receivedServerMessage(context, message, socket);

//	socket->owner()->setEnabled
	return KIrc::Handler::CoreHandled;
}

/* 002: ":Your host is <servername>, running version <ver>"
 * Gives information about the host. The given information are close to 004.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_002(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(context, message, socket);
	return KIrc::Handler::CoreHandled;
}

/* 003: "This server was created <date>"
 * Gives the date that this server was created.
 * NOTE: This is useful for determining the uptime of the server).
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_003(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	receivedServerMessage(context, message, socket);
	return KIrc::Handler::NotHandled;
}

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_004(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(5, 5);

	receivedServerMessage(context, message, socket);
//	emit incomingHostInfo(message.arg(1),message.arg(2),message.arg(3),message.arg(4));
	return KIrc::Handler::NotHandled;
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_005(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(?, ?);

	receivedServerMessage(context, message, socket);
	return KIrc::Handler::NotHandled;
}

/* 250: ":Highest connection count: <integer> (<integer> clients)
 *       (<integer> since server was (re)started)"
 * Tells connections statistics about the server for the uptime activity.
 * NOT IN RFC1459 NOR RFC2812
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_250(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(1, 1);

	receivedServerMessage(context, message, socket);
	return KIrc::Handler::NotHandled;
}

/* 251: ":There are <integer> users and <integer> services on <integer> servers"
 * Tells how many user there are on all the different servers in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_251(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(1, 1);

	receivedServerMessage(context, message, socket);
	return KIrc::Handler::NotHandled;
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_252(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(2, 2);

	bool ok = false;
//	Entity::Ptr from; Entity::Ptr to;
// 	QString text = i18np("There is %1 operator online.", "There are %1 operators online.", ev->message().argAt(1).toULong(&ok));

// 	if (ok)
//		postEvent(ev, "OperatorsOnline", from, to, text);
	return KIrc::Handler::NotHandled;
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_253(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	bool ok = false;
//	Entity::Ptr from; Entity::List to;
// 	QString text = i18np("There is %1 unknown connection.", "There are %1 unknown connections.", ev->message().argAt(1).toULong(&ok));

// 	if (ok)
//		postEvent(ev, "UnkownConnections", from, to, text);
	return KIrc::Handler::NotHandled;
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_255(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_263(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Entity::Ptr from; Entity::List to;
//	QString text = i18n("Server was too busy to execute %1.", ev->message().argAt(1));

//	postEvent(ev, "ServerTooBusy", from, to, text);
	return KIrc::Handler::NotHandled;
}

/* 265: ":Current local  users: <integer>  Max: <integer>"
 * Tells statistics about the current local server state.
 * NOT IN RFC2812
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_265(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_266(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 301: "<nick> :<away message>"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_301(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

/*
	Entity entity = message.entityFromArg(1);
	entity->setAwayMessage(message.suffix);
	entity->setMode("+a");

	receivedServerMessage(message);
*/
	return KIrc::Handler::NotHandled;
}

/* 303: ":*1<nick> *(" " <nick> )"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_303(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

/*
	QStringList nicks = QStringList::split(QRegExp(QChar(' ')), message.suffix());
	for(QStringList::Iterator it = nicks.begin(); it != nicks.end(); ++it)
	{
		if (!(*it).trimmed().isEmpty())
			emit incomingUserOnline(*it);
	}
*/
	return KIrc::Handler::NotHandled;
}

/* 305: ":You are no longer marked as being away"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_305(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
/*
	Entity::Ptr self = this->self();
	self->setAwayMessage(QString());
//	self->setModes("-a");
	postInfoEvent(message, i18n("You are no longer marked as being away."));
*/
	return KIrc::Handler::NotHandled;
}


/* 306: ":You have been marked as being away"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_306(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Entity::Ptr from; Entity::List to;

// 	if (postEvent(ev, "YouAreAway", from, to, i18n("You have been marked as being away."))) {
//		self->setModes("+a");
// 	}
	return KIrc::Handler::NotHandled;
}

/* 319: "<nick> :{[@|+]<channel><space>}"
 * Show info a channel a user is logged in (part of a /whois) in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_319(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoIsChannels(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoIsIdentified(message.arg(1));
	return KIrc::Handler::NotHandled;
}

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */

/* 322: "<channel> <# visible> :<topic>"
 * Received one channel from the LIST command.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_322(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingListedChan(message.arg(1), message.arg(2).toUInt(), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_323(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 324: "<channel> <mode> <mode params>"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_324(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelMode(message.arg(1), message.arg(2), message.arg(3));
	return KIrc::Handler::NotHandled;
}

/* 328:
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_328(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelHomePage(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_329(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	return KIrc::Handler::NotHandled;
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_331(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(message.arg(1), suffix);
	return KIrc::Handler::NotHandled;
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_332(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_333(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(4, 4);

/*
	QDateTime d;
	d.setTime_t( message.arg(3).toLong() );
	emit incomingTopicUser(message.arg(1), message.arg(2), d );
*/
	return KIrc::Handler::NotHandled;
}

/* 352:
 * WHO Reply
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_352(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
/*
	QStringList suffix = QStringList::split( ' ', message.suffix() );

	emit incomingWhoReply(
		message.arg(5),
		message.arg(1),
		message.arg(2),
		message.arg(3),
		message.arg(4),
		message.arg(6)[0] != 'H',
		message.arg(7),
		message.suffix().section(' ', 0, 1 ).toUInt(),
		message.suffix().section(' ', 1 )
	);
*/
	return KIrc::Handler::NotHandled;
}


/* 353:
 * NAMES list
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_353(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(3, 3);

//	emit incomingNamesList(message.arg(2), QStringList::split(' ', message.suffix()));
	return KIrc::Handler::NotHandled;
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_366(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_369(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_401(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 404: "<channel name> :Cannot send to channel"
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_404(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You cannot send message to channel %1.", message.arg(2)));
	return KIrc::Handler::NotHandled;
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_406(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 433: "<nick> :Nickname is already in use"
 * Tells us that our nickname is already in use.
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_433(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	if(m_status == Authentifying)
	{
		// This tells us that our nickname is, but we aren't logged in.
		// This differs because the server won't send us a response back telling us our nick changed
		// (since we aren't logged in).
//		m_FailedNickOnLogin = true;
//		emit incomingFailedNickOnLogin(message.arg(1));
	}
//	else
//	{
		// And this is the signal for if someone is trying to use the /nick command or such when already logged in,
		// but it's already in use
//		emit incomingNickInUse(message.arg(1));
//	}
//	postErrorEvent(message, i18n("Nickname %1 is already in use.", message.arg(1)));
	return KIrc::Handler::NotHandled;
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
KIrc::Handler::Handled ClientEventHandler::numericReply_464(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1)

	/* Server need pass.. Call disconnect*/
//	Entity::Ptr from; Entity::List to;
	QString text = i18n("Password Incorect");

//	postEvent(ev, "PasswordIncorrect", from, to, text);

//	emit incomingFailedServerPassword();
	return KIrc::Handler::NotHandled;
}

/* 465: ":You are banned from this server"
 */

/* 481: ":Permission Denied- You're not an IRC operator"
 */

/* 483: ":You can't kill a server!"
 */

/* 484: ":Your connection is restricted!"
 */

/* 491: ":No O-lines for your host"
 */

/* 501: ":Unknown MODE flag"
 */

/* 502: ":Cannot change mode for other users"
 */

