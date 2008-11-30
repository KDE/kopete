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

#include "kircclientchannelhandler.moc"

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

class KIrc::ClientChannelHandlerPrivate
	: public KIrc::HandlerPrivate
{
public:
	KIrc::EntitySet channels;
	QByteArray modes;
};

using namespace KIrc;

ClientChannelHandler::ClientChannelHandler(QObject* parent)
	: Handler(new ClientChannelHandlerPrivate, parent)
{
}

ClientChannelHandler::ClientChannelHandler( Handler * parent )
  	: Handler(new ClientChannelHandlerPrivate, parent)
{
  kDebug(14121);
}

ClientChannelHandler::~ClientChannelHandler()
{
}

/* RFC say: "( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0""
 * suspected: ":<channel> *(" "/"," <channel>)"
 * assumed ":<channel>"
 * This is the response of someone joining a channel.
 * Remember that this will be emitted when *you* /join a room for the first time
 */
KIrc::Handler::Handled ClientChannelHandler::JOIN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);
	CHECK_ARGS(0, 0);

	EntityPtr from = context->entityFromName(message.prefix());

#if 0
	if (message.prefix() == QLatin1String("0"))
		// leave all the channels
	else
#endif
	{
		// For now lets forgot about the keys
		EntityList to = context->entitiesFromNames(message.suffix());

		//If we caused the event, add the channel to the channels list
		if(from==socket->owner())
		{
			kDebug(14121)<<"/me caused a join event";
			foreach(const EntityPtr& e,to)
				d->channels.insert(e);
		}
		else //otherwise broadcast a message that someone new is in the channel
		{
			TextEvent *event = new TextEvent( "JOIN", from, to,QString() );
			context->postEvent( event );

			foreach(EntityPtr channel, to)
			{
				channel->context()->add(from);
			}
		}
	}

	return KIrc::Handler::CoreHandled;
}

/* The given user is kicked.
 * "<channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]"
 */
KIrc::Handler::Handled ClientChannelHandler::KICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);
//	CHECK_ARGS(2, 2);

	EntityPtr from = context->entityFromName(message.prefix());
	EntityList channels = context->entitiesFromNames(message.argAt(1));
	EntityList users = context->entitiesFromNames(message.argAt(2));

	//Send an event for each kicked user
	foreach(const EntityPtr & e,users)
	{
		TextEvent *event = new TextEvent("KICK", e, channels, i18n("got kicked by %1").arg(QString( from->name() )));
		context->postEvent(event);
	}

//	context->postEvent(ev, "KICK", from, channels, users);

//	foreach(channel, channels)
//		channel->remove(users);

	return KIrc::Handler::CoreHandled;
}

/* Change the mode of a user.
 * "<nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )"
 */
KIrc::Handler::Handled ClientChannelHandler::MODE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);
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
KIrc::Handler::Handled ClientChannelHandler::NICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);

	CHECK_ARGS(1, 1);

	EntityPtr from = context->entityFromName(message.prefix());
	QByteArray newNick = message.suffix();

	//TODO only display this notification in the channels, the user is actually in
	foreach(const EntityPtr& channel,d->channels)
	{
		TextEvent *event = new TextEvent( "NICK", from, channel,newNick );
		context->postEvent( event );
	}

	//As KIRC::Context only uses a list to store the entities, its enough to change the nick here
	//if we decide to use a QMap or similar, we must change the key too
	from->setName(newNick);

	return KIrc::Handler::CoreHandled;
}

/* Do not support CTCP here, just do the simple message handling.
 */
KIrc::Handler::Handled ClientChannelHandler::NOTICE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	Entity::Ptr from; Entity::List to;
	QString text;

//	postEvent(ev, "Notice", from, to, text);
	return KIrc::Handler::NotHandled;
}

/* This signal emits when a user parts a channel
 * "<channel> *( "," <channel> ) [ <Part Message> ]"
 */
KIrc::Handler::Handled ClientChannelHandler::PART(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

	Q_D(ClientChannelHandler);
	CHECK_ARGS(0, 0);

	kDebug(14121)<<"part: "<<message.toLine();

	EntityPtr from = context->entityFromName(message.prefix());


	// For now lets forgot about the keys
	EntityList to = context->entitiesFromNames(message.argAt(1));
	TextEvent *event = new TextEvent( "PART", from, to,message.suffix() );
	context->postEvent( event );

	foreach(EntityPtr channel, to)
	{
		channel->context()->remove(from);
	}

	return KIrc::Handler::CoreHandled;

//	context->postEvent(ev, "PART", from, channels, text);

//	foreach(channel, channels)
//		channel->remove(from);
}

/* Do not support CTCP here, just do the simple message handling.
 */
KIrc::Handler::Handled ClientChannelHandler::PRIVMSG(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);

	CHECK_ARGS(1, 1);

	kDebug( 14120 )<<"privmsg: "<<message.suffix();
	EntityPtr from = context->entityFromName(message.prefix());
	EntityList to = context->entitiesFromNames(message.argAt(1));
	QByteArray text = message.suffix();

	KIrc::TextEvent *event = new KIrc::TextEvent( "PRIVMSG", from, to, text);
	context->postEvent( event );

	return KIrc::Handler::CoreHandled;
}

KIrc::Handler::Handled ClientChannelHandler::QUIT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientChannelHandler);

	CHECK_ARGS(0, 0);

	kDebug( 14121 )<<"client quittting";

	EntityPtr from=context->entityFromName(message.prefix());

	//Not optimal, as we have to search through every channel,
	//but it shouldn't be that problematic, as I think nobody
	//can work with that many channels, to make this become too slow
	foreach(KIrc::EntityPtr chan,d->channels)
	{
		if(chan->context()->entities().contains(from))
		{
			KIrc::TextEvent *event = new KIrc::TextEvent( "QUIT", from, chan, message.suffix() );
			context->postEvent( event );
		}
	}

	kDebug( 14121 )<<"deleting";
	from->deleteLater();

	//if (postEvent(ev, "QUIT", from, message)) {
	//d->context->remove(from);
	//}

	return KIrc::Handler::CoreHandled;
}

/*
KIrc::Handler::Handled CLientCommands::squit(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);
}
*/

/* "<channel> [ <topic> ]"
 * The topic of a channel changed. emit the channel, new topic, and the person who changed it.
 */
KIrc::Handler::Handled ClientChannelHandler::TOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);
/*
	Entity::Ptr from;
	Entity::List channel;
	QByteArray topic;

	if (postEvent(ev, "Topic", from, channel, topic)) {
//		channel->set(Topic, topic);
	}
*/
	return KIrc::Handler::NotHandled;
}


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void ClientChannelHandler::bindNumericReplies()
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

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_004(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(5, 5);
	Q_D(ClientChannelHandler);

//	emit incomingHostInfo(message.arg(1),message.arg(2),message.arg(3),message.arg(4));
	d->modes = message.argAt(4);

	// Pretend not handled, used by the main handler
	return NotHandled;
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_005(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(?, ?);

//	receivedServerMessage(context, message, socket);
	return KIrc::Handler::NotHandled;
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
KIrc::Handler::Handled ClientChannelHandler::numericReply_254(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	bool ok = false;
//	EntityPtr from d->entityFromPrefix(message.prefix());
//	Entity::List to;
// 	QString text = i18np("There has been %1 channel formed.", "There have been %1 channels formed.", ev->message().argAt(1).toULong(&ok));

// 	if (ok)
// 	{
//		postEvent(ev, "ChannelsFormed", from, to, text);
//		return CoreHandled;
//	}
	return NotHandled;
}

/* 255: ":I have <integer> clients and <integer> servers"
 * Tells how many clients and servers *this* server handles.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_255(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_263(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_265(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 266: ":Current global users: <integer>  Max: <integer>"
 * Tells statistics about the current global(the whole irc server chain) server state:
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_266(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
// 	postServerInfoEvent(ev);
	return KIrc::Handler::NotHandled;
}

/* 301: "<nick> :<away message>"
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_301(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_303(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_305(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_306(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_319(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoIsChannels(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 320:
 * Indicates that this user is identified with NICSERV on FREENODE.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_320(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_322(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingListedChan(message.arg(1), message.arg(2).toUInt(), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_323(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 324: "<channel> <mode> <mode params>"
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_324(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelMode(message.arg(1), message.arg(2), message.arg(3));
	return KIrc::Handler::NotHandled;
}

/* 328:
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_328(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelHomePage(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_329(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	return KIrc::Handler::NotHandled;
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_331(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(message.arg(1), suffix);
	return KIrc::Handler::NotHandled;
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_332(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_333(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_352(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_353(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(3, 3);
	KIrc::EntityPtr channel=context->entityFromName(message.argAt(3)); //I don't know where the = in this message comes from

	if(!channel->isChannel())
	{
		kDebug(14120)<<"WARNING: got a NAMES list for something that is not a channel";
		return KIrc::Handler::NotHandled;
	}

	QList<QByteArray> nicks=message.suffix().split(' ');
	foreach(const QByteArray &user,nicks)
	{
		//Add the nick to the channel. As nicks are unique through the network, we use the Entities already present in the root context
		QByteArray nick;
		KIrc::EntityStatus status=0;
		if(user.startsWith("@")) //It's a channel operator. drop this information for the moment
		{
			nick=user.mid(1);
			status|=KIrc::Operator;
		}
		else
			nick=user;

		KIrc::EntityPtr entity=context->entityFromName( nick );
		channel->context()->add( entity );
		channel->context()->addStatus(entity,status);
	}

	//TODO: send out a notification about new entities on everyone of these messages,
	//instead of just on "End of NAMES list"

//	emit incomingNamesList(message.arg(2), QStringList::split(' ', message.suffix()));
	return KIrc::Handler::CoreHandled;
}

/* 366: "<channel> :End of NAMES list"
 * Gives a signal to indicate that the NAMES list has ended for channel.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_366(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	KIrc::EntityPtr channel=context->entityFromName(message.argAt(2));

	KIrc::ControlEvent *ev=new KIrc::ControlEvent("CHANNEL_INIT_LIST",channel);
	context->postEvent(ev);

//	emit receivedServerMessage(message);
	return KIrc::Handler::CoreHandled;
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_369(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_401(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 404: "<channel name> :Cannot send to channel"
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_404(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You cannot send message to channel %1.", message.arg(2)));
	return KIrc::Handler::NotHandled;
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_406(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::numericReply_433(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

/* 442: "<channel> :You're not on that channel"
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_442(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You are not on channel %1.", message.arg(1)));
	return KIrc::Handler::NotHandled;
}

/* 464: ":Password Incorrect"
 * Bad server password
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_464(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_471(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, channel is full.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 472: "<char> :is unknown mode char to me for <channel>"
 */

/* 473: "<channel> :Cannot join channel (+i)"
 * Invite Only.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_473(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, channel is invite only.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_474(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, you are banned from that channel.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
KIrc::Handler::Handled ClientChannelHandler::numericReply_475(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, wrong channel key was given.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 476: "<channel> :Bad Channel Mask"
 */

/* 477: "<channel> :Channel doesn't support modes" RFC-2812
 * 477: "<channel> :You need a registered nick to join that channel." DALNET
 */
// void ClientChannelHandler::numericReply_477(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
// {
// 	emit incomingChannelNeedRegistration(message.arg(2), message.suffix());
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

