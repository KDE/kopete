/*
    kircclienteventhandler.cpp - IRC Client Message Handler

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2006 by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2008-2009 by Alexander Rieder <alexanderrieder@gmail.com>

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

#include "kircstdmessages.h"
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
	bindNumericReplies();
}

ClientChannelHandler::ClientChannelHandler( Handler * parent )
  	: Handler(new ClientChannelHandlerPrivate, parent)
{
	kDebug(14121);
	bindNumericReplies();
}

ClientChannelHandler::~ClientChannelHandler()
{
}


/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
void ClientChannelHandler::bindNumericReplies()
{
	registerMessageAlias( "004", "RPL_MYINFO" );
	registerMessageAlias( "322", "RPL_LIST" );
	registerMessageAlias( "323", "RPL_LISTEND" );
	registerMessageAlias( "324", "RPL_CHANNELMODEIS" );
	registerMessageAlias( "328", "RPL_CHANNEL_URL" );
	registerMessageAlias( "329", "RPL_CREATIONTIME" );
	registerMessageAlias( "331", "RPL_NOTOPIC" );
	registerMessageAlias( "332", "RPL_TOPIC" );
	registerMessageAlias( "333", "RPL_TOPICWHOTIME" );
	registerMessageAlias( "353", "RPL_NAMREPLY" );
	registerMessageAlias( "366", "RPL_ENDOFNAMES" );

	registerMessageAlias( "401", "ERR_NOSUCHNICK" );
	registerMessageAlias( "404", "ERR_CANNOTSENDTOCHAN" );
	registerMessageAlias( "406", "ERR_WASNOSUCHNICK" );
	registerMessageAlias( "442", "ERR_NOTONCHANNEL" );
	registerMessageAlias( "471", "ERR_CHANNELISFULL" );
	registerMessageAlias( "473", "ERR_INVITEONLYCHAN" );
	registerMessageAlias( "474", "ERR_BANNEDFROMCHAN" );
	registerMessageAlias( "475", "ERR_BADCHANNELKEY" );
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
			{
				e->setType( KIrc::Entity::Channel );
				d->channels.insert(e);
			}
		}

		//broadcast a message that someone new is in the channel

		TextEvent *event = new TextEvent( "JOIN", from, to,QString() );
		context->postEvent( event );

		foreach(const EntityPtr& channel, to)
		{
			channel->context()->add(from);
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

	if ( from == socket->owner() ) //we caused the event, drop it
		return CoreHandled;

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
	if ( from == socket->owner() ) //we caused the event, drop it
		return CoreHandled;

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
	from->free();

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

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 *  * Gives information about the servername, version, available modes, etc.
 *   */
KIrc::Handler::Handled ClientChannelHandler::RPL_MYINFO( KIrc::Context *context,  const KIrc::Message &message,  KIrc::Socket *socket )
{
	CHECK_ARGS( 5,  5 );
	Q_D( ClientChannelHandler );

	//      emit incomingHostInfo(message.arg(1),message.arg(2),message.arg(3),message.arg(4));
	d->modes = message.argAt( 4 );

	// Pretend not handled, used by the main handler
	return NotHandled;
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
KIrc::Handler::Handled ClientChannelHandler::RPL_LIST(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingListedChan(message.arg(1), message.arg(2).toUInt(), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 323: ":End of LIST"
 * End of the LIST command.
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_LISTEND(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 324: "<channel> <mode> <mode params>"
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_CHANNELMODEIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelMode(message.arg(1), message.arg(2), message.arg(3));
	return KIrc::Handler::NotHandled;
}

/* 328:
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_CHANNEL_URL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingChannelHomePage(message.arg(1), message.suffix());
	return KIrc::Handler::NotHandled;
}

/* 329: "%s %lu"
 * NOTE: What is the meaning of this arguments. DAL-ircd say it's a RPL_CREATIONTIME
 * NOT IN RFC1459 NOR RFC2812
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_CREATIONTIME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	return KIrc::Handler::NotHandled;
}

/* 331: "<channel> :No topic is set"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_NOTOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingExistingTopic(message.arg(1), suffix);
	return KIrc::Handler::NotHandled;
}

/* 332: "<channel> :<topic>"
 * Gives the existing topic for a channel after a join.
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_TOPIC(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	KIrc::EntityPtr channel=context->entityFromName( message.argAt( 2 ) );
	channel->setTopic( message.suffix() );

	TextEvent *event=new TextEvent( "TOPIC", channel, channel, message.suffix() );
	context->postEvent( event );

	//	emit incomingExistingTopic(message.arg(1), message.suffix());
	return KIrc::Handler::CoreHandled;
}

/* 333:
 * Gives the nickname and time who changed the topic
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_TOPICWHOTIME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(4, 4);

	EntityPtr channel=context->entityFromName( message.argAt( 2 ) );
	EntityPtr user=context->entityFromName( message.argAt( 3 ) );

	TextEvent* event=new TextEvent( "TOPIC_WHOTIME", user, channel, message.argAt( 4 ) );
	context->postEvent( event );

	return KIrc::Handler::CoreHandled;
}

/* 353:
 * NAMES list
 */
KIrc::Handler::Handled ClientChannelHandler::RPL_NAMREPLY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::RPL_ENDOFNAMES(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	KIrc::EntityPtr channel=context->entityFromName(message.argAt(2));

	KIrc::ControlEvent *ev=new KIrc::ControlEvent("CHANNEL_INIT_LIST",channel);
	context->postEvent(ev);

//	emit receivedServerMessage(message);
	return KIrc::Handler::CoreHandled;
}

/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 404: "<channel name> :Cannot send to channel"
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_CANNOTSENDTOCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You cannot send message to channel %1.", message.arg(2)));
	return KIrc::Handler::NotHandled;
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 442: "<channel> :You're not on that channel"
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_NOTONCHANNEL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You are not on channel %1.", message.arg(1)));
	return KIrc::Handler::NotHandled;
}

/* 471: "<channel> :Cannot join channel (+l)"
 * Channel is Full
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_CHANNELISFULL(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientChannelHandler::ERR_INVITEONLYCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, channel is invite only.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 474: "<channel> :Cannot join channel (+b)"
 * Banned.
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_BANNEDFROMCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2)

//	postErrorEvent(message, i18n("Cannot join %1, you are banned from that channel.", message.arg(2)) );
	return KIrc::Handler::NotHandled;
}

/* 475: "<channel> :Cannot join channel (+k)"
 * Wrong Chan-key.
 */
KIrc::Handler::Handled ClientChannelHandler::ERR_BADCHANNELKEY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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


KIrc::Handler::Handled ClientChannelHandler::CMD_JOIN( KIrc::Context* context , const KIrc::Command& command, KIrc::Socket* socket )
{
	kDebug( 14121 )<<"joining channel"<<command;
	socket->writeMessage( KIrc::StdMessages::join( command.value(1) ) );
//	KIrc::EntityPtr channel=KIrc::EntityPtr( context->entityFromName( channelName ) );
//	channel->setType(KIrc::Entity::Channel);

	return KIrc::Handler::CoreHandled;
}

KIrc::Handler::Handled ClientChannelHandler::CMD_PART( KIrc::Context* context , const KIrc::Command& command, KIrc::Socket* socket )
{
	Q_D( ClientChannelHandler );

	QByteArray channel;
	QByteArray message;

	KIrc::Command args=command;
	args.removeFirst();

	if ( args.size()>1&&Entity::isChannel( args.first() ) )
		channel=args.takeFirst();
	else if ( context->owner()->isChannel() )
		channel=context->owner()->name();
	else
		return KIrc::Handler::NotHandled;

	//join together the args
	foreach( const QByteArray& a, args )
		message+=a+' ';
	message.chop( 1 );

	socket->writeMessage( KIrc::StdMessages::part( channel, message ) );

	//Remove the channels we are parting from the channels list
	KIrc::EntityList partingChannels=socket->owner()->context()->entitiesFromNames(channel);
	foreach(KIrc::EntityPtr c,partingChannels)
		d->channels.remove(c);


	return KIrc::Handler::CoreHandled;
}

Handler::Handled ClientChannelHandler::CMD_PRIVMSG(KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket)
{
	KIrc::Command args=command;
	args.removeFirst();

  	QByteArray dest=args.takeFirst();
	QByteArray msg;
	foreach( const QByteArray & part, args )
		msg+=part+" ";
	msg.chop( 1 );

	socket->writeMessage( KIrc::StdMessages::privmsg(dest, msg ) );

	return KIrc::Handler::CoreHandled;
}

Handler::Handled ClientChannelHandler::CMD_TOPIC( KIrc::Context* context, const KIrc::Command &command, KIrc::Socket* socket )
{
	QByteArray arg;

	foreach( QByteArray a, command.mid( 1 ) )
		arg+=a+' ';
	arg.chop( 1 );

	if ( arg.isEmpty() )
		socket->writeMessage( KIrc::StdMessages::topic(context->owner()->name()) );
	else
		socket->writeMessage( KIrc::StdMessages::topic(context->owner()->name(), arg ) );

	return KIrc::Handler::CoreHandled;
}

Handler::Handled ClientChannelHandler::CMD_REJOIN(KIrc::Context* context, const KIrc::Command&command, KIrc::Socket* socket)
{
  Q_D(ClientChannelHandler);

  if ( d->channels.isEmpty() )
	  return CoreHandled;

  KIrc::Command cmd=KIrc::Command()<<"JOIN";

  QByteArray channelList;
  foreach(KIrc::EntityPtr e,d->channels)
	channelList+=e->name()+',';
  channelList.chop(1); //Remove the last ','

  cmd<<channelList;

  d->channels.clear();
  CMD_JOIN(context,cmd,socket);

  return KIrc::Handler::CoreHandled;
}

KIrc::Command ClientChannelHandler::handledCommands()
{
	return KIrc::Handler::handledCommands()<<"JOIN"<<"PART"<<"PRIVMSG"<<"TOPIC";
}

