/*
    kircmessageutil.cpp - Some utilities to build some message.

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete engineelopers <kopete-engineel@kde.org>

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

#include <kdebug.h>

//QRegExp Message::sm_("^()\\r\\n$")
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
		const QByteArrayList &args,
		const QByteArray &suffix)
{
	#warning implement me
	QByteArray msg = command;

	// FIXME: use a while loop instead and quote the arguments
	// FIXME: make the empty arguments a "*" as mostly used in the IRC convention.
//	if (!args.isEmpty())
//		msg += QChar(' ') + args.join(QChar(' ')).stripWhiteSpace(); // some extra check should be done here

//	if (!suffix.isNull())
//		msg += QByteArray::fromLatin1(" :") + suffix;

	return msg;
}

QByteArray MessageUtil::formatCtcp(const QByteArray &str)
{
	#warning implement me
//	return QChar(0x01) + ctcpQuote(str) + QChar(0x01);
	return str;
}
*/

struct quote_data
{
	char in, out;
};

static QByteArray quote(const QByteArray &in, const struct quote_data *d)
{
	#warning implement me
	char esc = d->in;
	QByteArray out(in);

	return out;
}

static QByteArray unquote(const QByteArray &in, const struct quote_data *d)
{
	#warning implement me
	char esc = d->in;
	QByteArray out(in);

	return out;
}

static struct quote_data irc_quote_data[] =
{
	{ '\020', '\020' },
	{ '\r', 'r' },
	{ '\n', 'n' },
	{ '\0', '0' },
	{ 0, 0 }
};

QByteArray KIRC::MessageUtil::quote(const QByteArray &buffer)
{
	return quote(buffer, irc_quote_data);
}

QByteArray KIRC::MessageUtil::unquote(const QByteArray &buffer)
{
	return unquote(buffer, irc_quote_data);
}

#ifndef KIRC_STRICT

static struct quote_data irc_ctcp_quote_data =
{
	{ '\\', '\\' },
	{ 1, '1' },
	{ 0, 0 }
};

QByteArray KIRC::MessageUtil::quoteCtcp(const QByteArray &buffer)
{
	return quote(buffer, irc_ctcp_quote_data);
}

QByteArray KIRC::MessageUtil::unquoteCtcp(const QByteArray &buffer)
{
	return unquote(buffer, irc_ctcp_quote_data);
}

#endif

