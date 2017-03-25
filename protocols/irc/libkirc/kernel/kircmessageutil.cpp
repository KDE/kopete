/*
    kircmessageutil.cpp - Some utilities to build some message.

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kircmessageutil.h"

#include "kircbytearrayescaper.h"

#include <kdebug.h>

/*
#ifndef _IRC_STRICTNESS_
QRegExp Message::sm_IRCNumericCommand("^\\d{1,3}$");

// TODO: This regexp parsing is no good. It's slower than it needs to be, and
// is not codec-safe since QString requires a codec.
QRegExp Message::sm_IRCCommandType1(
    "^(?::([^ ]+) )?([A-Za-z]+|\\d{1,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$");
    // Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
QRegExp Message::sm_IRCNumericCommand("^\\d{3,3}$");

QRegExp Message::sm_IRCCommandType1(
    "^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$");
QRegExp Message::sm_IRCCommandType2(
    "^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$");
#endif // _IRC_STRICTNESS_
*/
/*
QByteArray MessageUtil::format(
        const QByteArray &command,
        const QList<ByteArray> &args,
        const QByteArray &suffix)
{
#ifdef __GNUC__
    #warning implement me
#endif
    QByteArray msg = command;

    // FIXME: use a while loop instead and quote the arguments
    // FIXME: make the empty arguments a "*" as mostly used in the IRC convention.
//	if (!args.isEmpty())
//		msg += QChar(' ') + args.join(QChar(' ')).trimmed(); // some extra check should be done here

//	if (!suffix.isNull())
//		msg += QByteArray::fromLatin1(" :") + suffix;

    return msg;
}

QByteArray MessageUtil::formatCtcp(const QByteArray &str)
{
#ifdef __GNUC__
    #warning implement me
#endif
//	return QChar(0x01) + ctcpQuote(str) + QChar(0x01);
    return str;
}
*/

static KIrc::ByteArrayEscaper IrcEscaper('\020', KIrc::ByteArrayEscaper::EscapeList()
                                         << KIrc::ByteArrayEscaper::Escape('\r', 'r')
                                         << KIrc::ByteArrayEscaper::Escape('\n', 'n')
                                         << KIrc::ByteArrayEscaper::Escape('\0', '0')
                                         );

QByteArray KIrc::MessageUtil::quote(const QByteArray &buffer)
{
    return IrcEscaper.escape(buffer);
}

QByteArray KIrc::MessageUtil::unquote(const QByteArray &buffer)
{
    return IrcEscaper.unescape(buffer);
}

#ifndef KIRC_STRICT

static KIrc::ByteArrayEscaper IrcCtcpEscaper('\\', KIrc::ByteArrayEscaper::EscapeList()
                                             << KIrc::ByteArrayEscaper::Escape((char)1, '1')
                                             );

QByteArray KIrc::MessageUtil::quoteCtcp(const QByteArray &buffer)
{
    return IrcCtcpEscaper.escape(buffer);
}

QByteArray KIrc::MessageUtil::unquoteCtcp(const QByteArray &buffer)
{
    return IrcCtcpEscaper.unescape(buffer);
}

#endif
