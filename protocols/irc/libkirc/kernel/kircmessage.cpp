/*
    kircmessage.cpp - IRC Client

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

#include "kircmessage.h"

#include "kircbytearrayescaper.h"
#include "kircconst.h"
#include "kirccontext.h"
#include "kircsocket.h"
#include "kircmessageutil.h"

#include <kdebug.h>

#include <QPointer>
#include <QSharedData>
#include <QTextCodec>


//QRegExp Message::sd->("^()\\r\\n$")
/*
#ifndef _IRC_STRICTNESS_
QRegExp Message::sd->IRCNumericCommand("^\\d{1,3}$");

// TODO: This regexp parsing is no good. It's slower than it needs to be, and
// is not codec-safe since QString requires a codec.
QRegExp Message::sd->IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{1,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$");
	// Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
QRegExp Message::sd->IRCNumericCommand("^\\d{3,3}$");

QRegExp Message::sd->IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$");
QRegExp Message::sd->IRCCommandType2(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$");
#endif // _IRC_STRICTNESS_
*/

class KIrc::MessagePrivate
	: public QSharedData
{
public:
//	QByteArray line;

	QByteArray prefix;
	QList<QByteArray> args;
	QByteArray suffix;
};

using namespace KIrc;

static KIrc::ByteArrayEscaper IrcEscaper('\020', KIrc::ByteArrayEscaper::EscapeList()
	<< KIrc::ByteArrayEscaper::Escape('\r', 'r')
	<< KIrc::ByteArrayEscaper::Escape('\n', 'n')
	<< KIrc::ByteArrayEscaper::Escape('\0', '0')
);

QByteArray Message::quote(const QByteArray &buffer)
{
	return IrcEscaper.escape(buffer);
}

QByteArray Message::unquote(const QByteArray &buffer)
{
	return IrcEscaper.unescape(buffer);
}

static KIrc::ByteArrayEscaper IrcCtcpEscaper('\\', KIrc::ByteArrayEscaper::EscapeList()
	<< KIrc::ByteArrayEscaper::Escape((char)1, '1')
);

QByteArray Message::quoteCtcp(const QByteArray &buffer)
{
	return IrcCtcpEscaper.escape(buffer);
}

QByteArray Message::unquoteCtcp(const QByteArray &buffer)
{
	return IrcCtcpEscaper.unescape(buffer);
}

Message::Message() : d(new KIrc::MessagePrivate())
{
}

Message::Message(const QByteArray &prefix,
		 const QList<QByteArray> &args,
		 const QByteArray &suffix)
	: d(new KIrc::MessagePrivate())
{
	d->prefix = prefix;
	d->args = args;
	d->suffix = suffix;
}

/*
Message::Message(const QByteArray &line)
	: Event(Event::Message)
	, d(new Private())
{
	setLine(line);
}
*/
Message::Message(const Message &o)
	: d(o.d)
{
}

Message::~Message()
{
}

Message &Message::operator=(const Message &o)
{
//	Event::operator=(o);
	return *this;
}

Message Message::fromLine(const QByteArray &line, bool *ok)
{
	bool success = true;

	QByteArray prefix;
	QList<QByteArray> args;
	QByteArray suffix;


	// Match a regexp instead of the replace ...
	QList<QByteArray> parts=line.trimmed().split(' ');

	//	remove the trailling \r\n if any(there must be in fact)
	parts.last()=parts.last().remove("\r\n");
	
	QList<QByteArray>::const_iterator it=parts.begin();

	if((*it).startsWith(":"))
	{
		prefix=(*it).mid(1);
		++it;
	}

	for(;it!=parts.end()&&!(*it).startsWith(":");++it)
	{
			args.append((*it));
	}
	
	if(it!=parts.end())
	{
		for(;it!=parts.end();++it)
		{
			suffix=suffix+" "+(*it);
		}
		//remove " :"
		suffix=suffix.mid(2);
	}

/*
	int token_start = 0;
	int token_end;
	while(token_endline.indexOf(' ') > 0)
	{
#ifndef KIRC_STRICT
//		eat empty tokens
#endif // KIRC_STRICT
	}
*/
	if (ok)
		*ok = success;

	if (success)
		return Message(prefix, args, suffix);
	else
		return Message();
}

QByteArray Message::toLine() const
{
	// FIXME: we need to do some escaping checks here.
	QList<QByteArray> list;

	if (!d->prefix.isEmpty())
		list.append(':' + d->prefix);

	foreach(const QByteArray arg, d->args)
	{
		if (arg.isEmpty())
			list.append("*");
		else
			list.append(arg);
	}

	// Append suffix when empty.
	if (!d->suffix.isNull())
		list.append(':' + d->suffix);

	return IrcEscaper.join(list, ' ') + "\r\n";
}

QByteArray Message::prefix() const
{
	return d->prefix;
}

#if 0
QString Message::prefix(QTextCodec *codec) const
{
	Q_ASSERT(codec);
	return codec->toUnicode(d->prefix);
}
#endif

void Message::setPrefix(const QByteArray & prefix)
{
	d->prefix=prefix;
}

Message &Message::operator << (const QByteArray &arg)
{
	d->args << arg;
	return *this;
}

Message &Message::operator << (const KIrc::OptArg &arg)
{
	d->args << arg;
	return *this;
}

QList<QByteArray> Message::args() const
{
	return d->args;
}

#if 0
QString Message::command(QTextCodec *codec) const
{
	Q_ASSERT(codec);
	return codec->toUnicode(d->command);
}
#endif

QByteArray Message::argAt(int i) const
{
	// Using value here so that it check bounds.
	return d->args.value(i);
}

QByteArray Message::suffix() const
{
	return d->suffix;
}

#if 0
QString Message::suffix(QTextCodec *codec) const
{
	Q_ASSERT(codec);
	return codec->toUnicode(d->suffix);
}
#endif

void Message::setSuffix(const QByteArray& suffix)
{
	d->suffix=suffix;
}

bool Message::isNumericReply() const
{
#ifdef __GNUC__
#warning FIXME
#endif
//      return d->IRCNumericCommand.exactMatch(d->command);
        return false;
}

