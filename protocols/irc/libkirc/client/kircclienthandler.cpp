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

#include "kircclienthandler.moc"

#include "kircclientsocket.h"

#include "kircclientmotdhandler.h"
#include "kircclientchannelhandler.h"
#include "kircclientpingponghandler.h"
#include "kircclientwhohandler.h"

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
	KIrc::ClientChannelHandler *channelHandler;
	KIrc::ClientPingPongHandler *pingPongHandler;
	KIrc::ClientWhoHandler *whoHandler;
};

using namespace KIrc;

ClientEventHandler::ClientEventHandler(QObject* parent)
	: Handler(new ClientEventHandlerPrivate, parent)
{
	Q_D(ClientEventHandler);
	kDebug(14121);
	d->motdHandler = new KIrc::ClientMotdHandler(this);
	d->channelHandler = new KIrc::ClientChannelHandler(this);
	d->pingPongHandler = new KIrc::ClientPingPongHandler(this);
	d->whoHandler = new KIrc::ClientWhoHandler(this);

	bindNumericReplies();
}

ClientEventHandler::~ClientEventHandler()
{
}

void ClientEventHandler::bindNumericReplies()
{
	//Bind the numeric replies to their string names, described in the RFCs
	registerMessageAlias( "001", "RPL_WELCOME" );
	registerMessageAlias( "004", "RPL_MYINFO" );
	registerMessageAlias( "005", "RPL_ISUPPORT" );
	registerMessageAlias( "252", "RPL_LUSEROP" );
	registerMessageAlias( "253", "RPL_LUSERUNKNOWN" );
	registerMessageAlias( "254", "RPL_LUSERCHANNELS" );
	registerMessageAlias( "263", "RPL_TRYAGAIN" );
	registerMessageAlias( "301", "RPL_AWAY" );
	registerMessageAlias( "303", "RPL_ISON" );
	registerMessageAlias( "305", "RPL_UNAWAY" );
	registerMessageAlias( "306", "RPL_NOWAWAY" );
	registerMessageAlias( "352", "RPL_WHOREPLY" );
	registerMessageAlias( "353", "RPL_NAMREPLY" );
	registerMessageAlias( "366", "RPL_ENDOFNAMES" );
	registerMessageAlias( "369", "RPL_ENDOFWHOWAS" );

	registerMessageAlias( "401", "ERR_NOSUCHNICK" );
	registerMessageAlias( "404", "ERR_CANNOTSENDTOCHAN" );
	registerMessageAlias( "406", "ERR_WASNOSUCHNICK" );
	registerMessageAlias( "433", "ERR_NICKNAMEINUSE" );
	registerMessageAlias( "464", "ERR_PASSWDMISMATCH" );



	//Bind aliases to receivedServerMessage for events who just need to display a message without further handling

	/* 002: ":Your host is <servername>, running version <ver>"
	 * Gives information about the host. The given information are close to 004.
	 */
	registerMessageAlias( "002", "receivedServerMessage" );

	/* 003: "This server was created <date>"
	 * Gives the date that this server was created.
	 * NOTE: This is useful for determining the uptime of the server).
	 */
	registerMessageAlias( "003", "receivedServerMessage" );

	/* 250: ":Highest connection count: <integer> (<integer> clients)
	 *       (<integer> since server was (re)started)"
	 * Tells connections statistics about the server for the uptime activity.
	 * NOTE IN RFC1459 NOR RFC2812
	 */
	registerMessageAlias( "250", "receivedServerMessage" );

	/* 251: ":There are <integer> users and <integer> services on <integer> servers"
	 * Tells how many user there are on all the different servers in the form of:
	 */
	registerMessageAlias( "251", "receivedServerMessage" );

	/* 255: ":I have <integer> clients and <integer> servers"
	 * Tells how many clients and servers *this* server handles.
	 */
	registerMessageAlias( "255", "receivedServerMessage" );


	/* 265: ":Current local  users: <integer>  Max: <integer>"
	 * Tells statistics about the current local server state.
	 * NOT IN RFC2812
	 */
	registerMessageAlias( "265", "receivedServerMessage" );

	/* 266: ":Current global users: <integer>  Max: <integer>"
	 * Tells statistics about the current global(the whole irc server chain) server state:
	 */
	registerMessageAlias( "266", "receivedServerMessage" );


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

KIrc::Handler::Handled ClientEventHandler::receivedServerMessage(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket*>( socket );
	KIrc::TextEvent *event=new KIrc::TextEvent( "ServerInfo", client->server(), client->owner(), message.suffix() );
	context->postEvent( event );

	return KIrc::Handler::CoreHandled;
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


/* 001: "Welcome to the Internet Relay Network <nick>!<user>@<host>"
 * Gives a welcome message in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::RPL_WELCOME(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

/* 004: "<servername> <version> <available user modes> <available channel modes>"
 * Gives information about the servername, version, available modes, etc.
 */
KIrc::Handler::Handled ClientEventHandler::RPL_MYINFO( KIrc::Context *context,  const KIrc::Message &message,  KIrc::Socket *socket )
{
  //TODO handle this somehow?

  return KIrc::Handler::CoreHandled;
}

/* 005:
 * Gives capability information. TODO: This is important!
 */
KIrc::Handler::Handled ClientEventHandler::RPL_ISUPPORT(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	CHECK_ARGS(?, ?);

	//TODO: Handle this better?
	QList<QByteArray> params=message.args();
	//remove the first three parameters
	params.removeFirst();
	params.removeFirst();
	params.removeFirst();
	QByteArray txt;
	foreach( QByteArray b, params )
		txt+=b+' ';
	txt+=message.suffix();

	KIrc::ClientSocket *client = static_cast<KIrc::ClientSocket*>( socket );
	KIrc::TextEvent *event=new KIrc::TextEvent( "ServerInfo", client->server(), client->owner(), txt );
	context->postEvent( event );

	return KIrc::Handler::CoreHandled;
}

/* 252: "<integer> :operator(s) online"
 * Issues a number of operators on the server in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::RPL_LUSEROP(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	Q_D(ClientEventHandler);

	CHECK_ARGS(2, 2);

	bool ok = false;
//	Entity::Ptr from; Entity::Ptr to;
// 	QString text = i18np("There is %1 operator online.", "There are %1 operators online.", ev->message().argAt(1).toULong(&ok));

// 	if (ok)
//		postEvent(ev, "OperatorsOnline", from, to, text);

	receivedServerMessage( context, message, socket );
	return KIrc::Handler::CoreHandled;
}

/* 253: "<integer> :unknown connection(s)"
 * Tells how many unknown connections the server has in the form of:
 */
KIrc::Handler::Handled ClientEventHandler::RPL_LUSERUNKNOWN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	bool ok = false;
//	Entity::Ptr from; Entity::List to;
// 	QString text = i18np("There is %1 unknown connection.", "There are %1 unknown connections.", ev->message().argAt(1).toULong(&ok));

// 	if (ok)
//		postEvent(ev, "UnkownConnections", from, to, text);

	receivedServerMessage( context, message, socket );
	return KIrc::Handler::CoreHandled;
}

/* 254: "<integer> :channels formed"
 * Tells how many total channels there are on this network.
 *  */
KIrc::Handler::Handled ClientEventHandler::RPL_LUSERCHANNELS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	//do some more here?
	receivedServerMessage( context, message, socket );
	return KIrc::Handler::CoreHandled;
}

/* 263: "<command> :Please wait a while and try again."
 * Server is too busy.
 */
KIrc::Handler::Handled ClientEventHandler::RPL_TRYAGAIN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Entity::Ptr from; Entity::List to;
//	QString text = i18n("Server was too busy to execute %1.", ev->message().argAt(1));

//	postEvent(ev, "ServerTooBusy", from, to, text);
	return KIrc::Handler::NotHandled;
}


/* 301: "<nick> :<away message>"
 */
KIrc::Handler::Handled ClientEventHandler::RPL_AWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientEventHandler::RPL_ISON(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientEventHandler::RPL_UNAWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientEventHandler::RPL_NOWAWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	Entity::Ptr from; Entity::List to;

// 	if (postEvent(ev, "YouAreAway", from, to, i18n("You have been marked as being away."))) {
//		self->setModes("+a");
// 	}
	return KIrc::Handler::NotHandled;
}

/* 321: "<channel> :Users  Name" ("Channel :Users  Name")
 * RFC1459: Declared.
 * RFC2812: Obsoleted.
 */


/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
KIrc::Handler::Handled ClientEventHandler::ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 404: "<channel name> :Cannot send to channel"
 */
KIrc::Handler::Handled ClientEventHandler::ERR_CANNOTSENDTOCHAN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	postErrorEvent(message, i18n("You cannot send message to channel %1.", message.arg(2)));
	return KIrc::Handler::NotHandled;
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
KIrc::Handler::Handled ClientEventHandler::ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientEventHandler::ERR_NICKNAMEINUSE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientEventHandler::ERR_PASSWDMISMATCH(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

