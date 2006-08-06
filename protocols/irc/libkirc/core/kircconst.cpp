/*
    kircconst.cpp - The KIRC constants and enums.

    Copyright (c) 2005      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2005      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kircconst.h"

#include <qtextcodec.h>

#define CONST_BYTEARRAY(str) const QByteArray KIRC::str(#str)

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp KIRC::sm_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

const QString KIRC::URL_NICKNAME("nickname");
const QString KIRC::URL_REALNAME("realname");

CONST_BYTEARRAY(AWAY);
CONST_BYTEARRAY(ERROR);
CONST_BYTEARRAY(INVITE);
CONST_BYTEARRAY(ISON);
CONST_BYTEARRAY(JOIN);
CONST_BYTEARRAY(KICK);
CONST_BYTEARRAY(LIST);
CONST_BYTEARRAY(MODE);
CONST_BYTEARRAY(MOTD);
CONST_BYTEARRAY(NICK);
CONST_BYTEARRAY(NOTICE);
CONST_BYTEARRAY(PART);
CONST_BYTEARRAY(PASS);
CONST_BYTEARRAY(PING);
CONST_BYTEARRAY(PONG);
CONST_BYTEARRAY(PRIVMSG);
CONST_BYTEARRAY(QUIT);
CONST_BYTEARRAY(SQUIT);
CONST_BYTEARRAY(TOPIC);
CONST_BYTEARRAY(USER);
CONST_BYTEARRAY(WHO);
CONST_BYTEARRAY(WHOIS);
CONST_BYTEARRAY(WHOWAS);

QTextCodec *KIRC::UTF8 = QTextCodec::codecForName("UTF-8");

