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

#define CONST_QSTRING(str) const QString str = QString::fromLatin1(#str)

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

namespace KIRC {

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */
const QRegExp KIRC::sm_RemoveLinefeeds( QString::fromLatin1("[\\r\\n]*$") );

CONST_QSTRING(AWAY);
CONST_QSTRING(ERROR);
CONST_QSTRING(INVITE);
CONST_QSTRING(ISON);
CONST_QSTRING(JOIN);
CONST_QSTRING(KICK);
CONST_QSTRING(LIST);
CONST_QSTRING(MODE);
CONST_QSTRING(MOTD);
CONST_QSTRING(NICK);
CONST_QSTRING(NOTICE);
CONST_QSTRING(PART);
CONST_QSTRING(PASS);
CONST_QSTRING(PING);
CONST_QSTRING(PONG);
CONST_QSTRING(PRIVMSG);
CONST_QSTRING(QUIT);
CONST_QSTRING(SQUIT);
CONST_QSTRING(TOPIC);
CONST_QSTRING(USER);
CONST_QSTRING(WHO);
CONST_QSTRING(WHOIS);
CONST_QSTRING(WHOWAS);

QTextCodec *KIRC::UTF8 = QTextCodec::codecForName("UTF-8");

}

