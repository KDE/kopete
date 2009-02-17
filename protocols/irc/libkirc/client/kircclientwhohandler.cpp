/*
    kircclientwhohandler.cpp - IRC Client Who Handler

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

#include "kircclientwhohandler.moc"

#include "kircclientsocket.h"

#include "kirccontext.h"

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

class KIrc::ClientWhoHandlerPrivate
{
public:
	KIrc::Context *context;
};

using namespace KIrc;

ClientWhoHandler::ClientWhoHandler(KIrc::Handler *handler)
	: Handler(handler)
	, d_ptr(new ClientWhoHandlerPrivate)
{
	Q_D(ClientWhoHandler);

	//d->context = context;
}

ClientWhoHandler::~ClientWhoHandler()
{
	delete d_ptr;
}

/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
void ClientWhoHandler::bindNumericReplies()
{
	registerMessageAlias( "352", "RPL_WHOREPLY" );
	registerMessageAlias( "315", "RPL_ENDOFWHO" );

	registerMessageAlias( "301", "RPL_AWAY" );
	registerMessageAlias( "319", "RPL_WHOISCHANNELS" );
	registerMessageAlias( "317", "RPL_WHOISIDLE" );
	registerMessageAlias( "313", "RPL_WHOISOPERATOR" );
	registerMessageAlias( "311", "RPL_WHOISUSER" );
	registerMessageAlias( "318", "RPL_ENDOFWHOis" );

	registerMessageAlias( "314", "RPL_WHOWASUSER" );
	registerMessageAlias( "369", "RPL_ENDOFWHOWAS" );
	registerMessageAlias( "406", "RPL_ERR_WASNOSUCHNICK" );

	registerMessageAlias( "312", "RPL_WHOISSERVER" );
	registerMessageAlias( "431", "ERR_NONICKNAMEGIVEN" );
	registerMessageAlias( "401", "ERR_NOSUCHNICK" );
	registerMessageAlias( "402", "ERR_NOSUCHSERVER" );
}


/* WHO [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
KIrc::Handler::Handled ClientWhoHandler::WHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* WHOIS [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
KIrc::Handler::Handled ClientWhoHandler::WHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* WHOWAS [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
KIrc::Handler::Handled ClientWhoHandler::WHOWAS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
/*
KIrc::Handler::Handled ClientWhoHandler::numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(1, 1);

//	postError(msg, i18n("%1 is a registered nick.", msg.arg(1)));
}
*/

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOISUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(5, 5);

//	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOISSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(4, 4);

//	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOISOPERATOR(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(3, 3);

//	postInfo(msg, i18n("%1 is an IRC operator.", msg.arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOWASUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_ENDOFWHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	receivedServerMessage(msg);
}

/* 301: "<nick> :<away message>"
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_AWAY(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOISIDLE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
KIrc::Handler::Handled ClientWhoHandler::RPL_ENDOFWHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(msg);
}

/* 319: "<nick> :*( ( "@" / "+" ) <channel> " " )"
 * End of WHOIS for a given nick.
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_WHOISCHANNELS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(msg);
}

/* 352    RPL_WHOREPLY
 * "<channel> <user> <host> <server> <nick>
 * ( "H" / "G" > ["*"] [ ( "@" / "+" ) ]
 * :<hopcount> <real name>"
 */

KIrc::Handler::Handled ClientWhoHandler::RPL_WHOREPLY(KIrc::Context* context, const KIrc::Message &message, KIrc::Socket *socket )
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
	//return KIrc::Handler::NotHandled;
}

/* 369: "<nick> :End of WHOWAS"
 * End of WHOWAS Request
 */
KIrc::Handler::Handled ClientWhoHandler::RPL_ENDOFWHOWAS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

//	emit receivedServerMessage(message);
	return KIrc::Handler::NotHandled;
}

/* 406: "<nickname> :There was no such nickname"
 * Like case 401, but when there *was* no such nickname.
 */
KIrc::Handler::Handled ClientWhoHandler::ERR_WASNOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
	CHECK_ARGS(2, 2);

	#warning FIXME 406 MEANS *NEVER*, unlike 401
//	i18n("The channel \"%1\" does not exist").arg(nick)
//	i18n("The nickname \"%1\" does not exist").arg(nick)
	return KIrc::Handler::NotHandled;
}

/* 431: ERR_NONICKNAMEGIVEN
 * ":No nickname given"
 * - Returned when a nickname parameter expected
 *   for a command and isn't found.
 */
KIrc::Handler::Handled ClientWhoHandler::ERR_NONICKNAMEGIVEN(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}


/* 401: "<nickname> :No such nick/channel"
 * Gives a signal to indicate that the command issued failed because the person/channel not being on IRC.
 *  - Used to indicate the nickname parameter supplied to a command is currently unused.
 */
KIrc::Handler::Handled ClientWhoHandler::ERR_NOSUCHNICK(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{

}

/* 402    ERR_NOSUCHSERVER
 * "<server name> :No such server"
 * - Used to indicate the server name given currently
 *   does not exist.
 */
KIrc::Handler::Handled ClientWhoHandler::ERR_NOSUCHSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{

}

