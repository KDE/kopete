/*
    kircclientwhohandler.cpp - IRC Client Who Handler

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

ClientWhoHandler::ClientWhoHandler(Context *context)
    : Handler(context)
    , d_ptr(new ClientWhoHandlerPrivate)
{
    Q_D(ClientWhoHandler);

    d->context = context;
}

ClientWhoHandler::~ClientWhoHandler()
{
    delete d_ptr;
}

/* WHO [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
void ClientWhoHandler::WHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* WHOIS [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
void ClientWhoHandler::WHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* WHOWAS [ <mask> [ "o" ] ]
 * List all the visible users to the user.
 */
void ClientWhoHandler::WHOWAS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
}

/* IMPORTANT NOTE:
 * Numeric replies always have the current nick or * as first argmuent.
 * NOTE: * means undefined in most (all ?) of the cases.
 */
/*
void ClientWhoHandler::bindNumericReplies()
{
    bind(263, this, SLOT(numericReply_263(Message*&)));
    bind(265, this, SLOT(numericReply_265(Message*&)));
    bind(266, this, SLOT(numericReply_266(Message*&)));

//	bind(305, this, SLOT(ignoreMessage(Message*&)), 0, 0 );
//	bind(306, this, SLOT(ignoreMessage(Message*&)), 0, 0 );
    bind(312, this, SLOT(numericReply_312(Message*&)), 3, 3);
    bind(313, this, SLOT(numericReply_313(Message*&)), 2, 2);
    bind(314, this, SLOT(numericReply_314(Message*&)), 5, 5);
    bind(315, this, SLOT(numericReply_315(Message*&)), 2, 2);
    bind(317, this, SLOT(numericReply_317(Message*&)), 3, 4);
    bind(318, this, SLOT(numericReply_318(Message*&)), 2, 2);
    bind(319, this, SLOT(numericReply_319(Message*&)), 2, 2);
    bind(320, this, SLOT(numericReply_320(Message*&)), 2, 2);
//	bind(321, this, SLOT(ignoreMessage(Message*&)), 0, 0 );
    bind(322, this, SLOT(numericReply_322(Message*&)), 3, 3);
    bind(323, this, SLOT(numericReply_323(Message*&)), 1, 1);
    bind(324, this, SLOT(numericReply_324(Message*&)), 2, 4);
    bind(328, this, SLOT(numericReply_328(Message*&)), 2, 2);
    bind(329, this, SLOT(numericReply_329(Message*&)), 3, 3);
//	bind(330, this, SLOT(ignoreMessage(Message*&)), 3, 3); // ???
    bind(331, this, SLOT(numericReply_331(Message*&)), 2, 2);
    bind(332, this, SLOT(numericReply_332(Message*&)), 2, 2);
    bind(352, this, SLOT(numericReply_352(Message*&)), 5, 10);

    //Freenode seems to use this for a non-RFC compliant purpose, as does Unreal
    bind(477, this, SLOT(receivedServerMessage(Message&)),0,0);
}
*/

/* 307: ":is a registered nick"
 * DALNET: Indicates that this user is identified with NICSERV.
 */
/*
void ClientWhoHandler::numericReply_307(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(1, 1);

//	postError(msg, i18n("%1 is a registered nick.", msg.arg(1)));
}
*/

/* 311: "<nick> <user> <host> * :<real name>"
 * Show info about a user (part of a /whois) in the form of:
 */
void ClientWhoHandler::RPL_WHOISUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(5, 5);

//	emit incomingWhoIsUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 312: "<nick> <server> :<server info>"
 * Show info about a server (part of a /whois).
 */
void ClientWhoHandler::RPL_WHOISSERVER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(4, 4);

//	emit incomingWhoIsServer(msg.arg(1), msg.arg(2), msg.suffix());
}

/* 313: "<nick> :is an IRC operator"
 * Show info about an operator (part of a /whois).
 */
void ClientWhoHandler::RPL_WHOISOPERATOR(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
    CHECK_ARGS(3, 3);

//	postInfo(msg, i18n("%1 is an IRC operator.", msg.arg(1)));
}

/* 314: "<nick> <user> <host> * :<real name>"
 * Show WHOWAS Info
 */
void ClientWhoHandler::RPL_WHOWASUSER(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit incomingWhoWasUser(msg.arg(1), msg.arg(2), msg.arg(3), msg.suffix());
}

/* 315: "<name> :End of WHO list"
 * End of WHO list.
 */
void ClientWhoHandler::RPL_ENDOFWHO(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	receivedServerMessage(msg);
}

/* RFC say: "<nick> <integer> :seconds idle"
 * Some servers say: "<nick> <integer> <integer> :seconds idle, signon time"
 * Show info about someone who is idle (part of a /whois) in the form of:
 */
void ClientWhoHandler::RPL_WHOISIDLE(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
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
void ClientWhoHandler::RPL_ENDOFWHOIS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(msg);
}

/* 319: "<nick> :*( ( "@" / "+" ) <channel> " " )"
 * End of WHOIS for a given nick.
 */
void ClientWhoHandler::RPL_WHOISCHANNELS(KIrc::Context *context, const KIrc::Message &message, KIrc::Socket *socket)
{
//	emit receivedServerMessage(msg);
}
