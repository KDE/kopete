/*
    kircmessageutil.h - Some utilities to build some message.

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

#ifndef KIRC_MESSAGEUTIL_H
#define KIRC_MESSAGEUTIL_H

#include <QByteArray>

namespace KIRC {
namespace MessageUtil {

extern QByteArray quote(const QByteArray &buffer);
extern QByteArray unquote(const QByteArray &buffer);

#ifndef KIRC_STRICT

extern QByteArray quoteCtcp(const QByteArray &buffer);
extern QByteArray unquoteCtcp(const QByteArray &buffer);

//extern void writeCtcpQueryMessage(const QString &to, const QString &ctcpMessage, QTextCodec *codec = 0);
//extern void writeCtcpReplyMessage(const QString &to, const QString &ctcpMessage, QTextCodec *codec = 0);

//extern void writeCtcpErrorMessage(const QString &to, const QString &ctcpLine, const QString &errorMsg, QTextCodec *codec = 0);

#endif

} // MessageUtil
} // KIRC

#endif

