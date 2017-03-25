/*
    kircconst.cpp - The KIRC constants and enums.

    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2005-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircconst.h"

#include <qtextcodec.h>

/* The usage of the namespace, instead of the "using" keyword, is intentional.
 * Not using it introduce compiler confusion, and lead to new symbols declaration.
 */

/* Please note that the regular expression "[\\r\\n]*$" is used in a QString::replace statement many times.
 * This gets rid of trailing \r\n, \r, \n, and \n\r characters.
 */

const QRegExp KIrc::sm_RemoveLinefeeds(QString::fromLatin1("[\\r\\n]*$"));

const QString KIrc::URL_NICKNAME("nickname");
const QString KIrc::URL_REALNAME("realname");

QTextCodec *KIrc::UTF8 = QTextCodec::codecForName("UTF-8");
