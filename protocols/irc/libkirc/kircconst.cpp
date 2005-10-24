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

#define CONST_STRING(str) const QByteArray KIRC::str(#str)

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp KIRC::sm_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

CONST_STRING(AWAY);
CONST_STRING(ERROR);
CONST_STRING(INVITE);
CONST_STRING(ISON);
CONST_STRING(JOIN);
CONST_STRING(KICK);
CONST_STRING(LIST);
CONST_STRING(MODE);
CONST_STRING(MOTD);
CONST_STRING(NICK);
CONST_STRING(NOTICE);
CONST_STRING(PART);
CONST_STRING(PASS);
CONST_STRING(PING);
CONST_STRING(PONG);
CONST_STRING(PRIVMSG);
CONST_STRING(QUIT);
CONST_STRING(SQUIT);
CONST_STRING(TOPIC);
CONST_STRING(USER);
CONST_STRING(WHO);
CONST_STRING(WHOIS);
CONST_STRING(WHOWAS);

QTextCodec *KIRC::UTF8 = QTextCodec::codecForName("UTF-8");

