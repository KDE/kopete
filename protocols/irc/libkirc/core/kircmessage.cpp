/*
    kircmessage.cpp - IRC Client

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

#include "kircmessage.h"

#include "kircentitymanager.h"
#include "kircsocket.h"
#include "kircmessageutil.h"

#include <kdebug.h>

#include <QPointer>
#include <QSharedData>
#include <QTextCodec>

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
class KIrc::Message::Private
	: public QSharedData
{
public:
	Private()
		: socket(0)
		, direction(Undefined)
		, codec(0)
		, valid(false)
		, dirty(false)
	{ }

	QPointer<KIrc::Socket> socket;
	KIrc::Message::Direction direction;

	QTextCodec *codec;	
	QByteArray line;
	QByteArray prefix;
	QByteArray command;
	QList<QByteArray> args;
	QByteArray suffix;

	bool valid:1; // if the message was parsed successfuly
	bool dirty:1; // if the message contents is modified, so should be rebuilded
};

using namespace KIrc;

Message::Message()
	: d(new Private())
{
}

Message::Message(const Message &o)
        : d(o.d)
{
}

Message::~Message()
{
}

Message &Message::operator = (const Message &o)
{
	d = o.d;
	return *this;
}

bool Message::isValid() const
{
	return d->valid;
}

Socket *Message::socket() const
{
	return d->socket;
}

void Message::setSocket(Socket *socket)
{
	d->socket = socket;
}

Message::Direction Message::direction() const
{
	return d->direction;
}

void Message::setDirection(Message::Direction direction)
{
	d->direction = direction;
}

QTextCodec *Message::codec() const
{
	return d->codec;
}

void Message::setCodec(QTextCodec *codec)
{
	d->codec = codec;
}

QByteArray Message::rawLine() const
{
	return d->line;
}

void Message::setLine(const QByteArray &line)
{
	d->line = line;
	d->valid = false;
/*
	// Match a regexp instead of the replace ...
//	d->line.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
 
#ifdef __GNUC__
	#warning implement me: parsing
#endif

	int token_start = 0;
	int token_end;
	while(token_endline.indexOf(' ') > 0)
	{
#ifndef KIRC_STRICT
//		eat empty tokens
#endif // KIRC_STRICT

	}

	if (regexp.exactMatch(d->raw))
	{
//		d->line    = regexp.cap(1).latin1();
		d->prefix  = regexp.cap(1).latin1();
		d->command = regexp.cap(2).latin1();
		d->args    = regexp.cap(3).latin1();
//		d->argList = QStringList::split(' ', d->args);
		d->suffix  = regexp.cap(4).latin1();

#ifndef KIRC_STRICT
		extractCtcpCommand();
#endif // KIRC_STRICT

		d->valid = true;
	}
*/
}

QByteArray Message::rawPrefix() const
{
	return d->prefix;
}

void Message::setPrefix(const QByteArray &prefix)
{
	if (d->prefix != prefix)
	{
		d->dirty = true;
		d->prefix = prefix;
	}
}

QByteArray Message::rawCommand() const
{
	return d->command;
}

void Message::setCommand(const QByteArray &command)
{
	if (d->command != command)
	{
		d->dirty = true;
		d->command = command;
	}
}

QList<QByteArray> Message::rawArgs() const
{
	return d->args;
}

void Message::setArgs(const QList<QByteArray> &args)
{
	if (d->args != args)
	{
		d->dirty = true;
		d->args = args;
	}
}

void Message::appendArgs(const QList<QByteArray> &args)
{
	if (!args.isEmpty())
	{
		d->dirty = true;
#ifdef __GNUC__
#warning use operator << until the append methods for QList are defined.
#endif
		d->args << args;
	}
}

QByteArray Message::rawArg(size_t i) const
{
	return d->args[i];
}

void Message::setArg(size_t i, const QByteArray &arg)
{
#ifdef __GNUC__
#warning allow so kind of append mode ?
#endif
	if (d->args[i] != arg)
	{
		d->dirty = true;
		d->args[i] = arg;
	}
}

void Message::appendArg(const QByteArray &arg)
{
	d->dirty = true;
	d->args.append(arg);
}

QByteArray Message::rawSuffix() const
{
	return d->suffix;
}

void Message::setSuffix(const QByteArray &suffix)
{
	if (d->suffix != suffix)
	{
		d->dirty = true;
		d->suffix = suffix;
	}
}

QString Message::prefix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->prefix);
}

void Message::setPrefix(const QString &prefix, QTextCodec *codec)
{
	setPrefix(checkCodec(codec)->fromUnicode(prefix));
}

QString Message::command(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->command);
}

void Message::setCommand(const QString &command, QTextCodec *codec)
{
	setCommand(checkCodec(codec)->fromUnicode(command));
}

QStringList Message::args(QTextCodec *codec) const
{
	QStringList args;
	codec = checkCodec(codec);

	foreach(const QByteArray &arg, d->args)
		args.append(codec->toUnicode(arg));

	return args;
}

void Message::setArgs(const QStringList &args, QTextCodec *codec)
{
	QList<QByteArray> arrayList;
	codec = checkCodec(codec);

	foreach (const QString &arg, args)
		arrayList.append(codec->fromUnicode(arg));

	setArgs(arrayList);
}

QString Message::arg(size_t i, QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->args[i]);
}

void Message::setArg(size_t i, const QString &arg, QTextCodec *codec)
{
	setArg(i, checkCodec(codec)->fromUnicode(arg));
}

void Message::appendArg(const QString &arg, QTextCodec *codec)
{
	appendArg(checkCodec(codec)->fromUnicode(arg));
}

QString Message::suffix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->suffix);
}

void Message::setSuffix(const QString &suffix, QTextCodec *codec)
{
	setSuffix(checkCodec(codec)->fromUnicode(suffix));
}

size_t Message::argsSize() const
{
	return d->args.size();
}
/*
Entity::Ptr Message::entityFromPrefix() const
{
        if (d->prefix.isEmpty())
                return engine->self();
        else
                return socket()->entityManager()->entityFromName(d->prefix);
}

Entity::Ptr Message::entityFromArg(size_t i) const
{
        return socket()-entityManager()->entityFromName(d->argList[i]);
}
*/
Entity::Ptr Message::entityFromName(const QByteArray &name)
{
	return socket()->entityManager()->entityFromName(name);
}

Entity::List Message::entitiesFromNames(const QList<QByteArray> &names)
{
	return socket()->entityManager()->entitiesFromNames(names);
}

Entity::List Message::entitiesFromNames(const QByteArray &names, char separator)
{
	return socket()->entityManager()->entitiesFromNames(names, separator);
}

bool Message::isNumericReply() const
{
#ifdef __GNUC__
#warning FIXME
#endif
//      return sm_IRCNumericCommand.exactMatch(d->command);
        return false;
}

Message::Type Message::type() const
{
#ifdef __GNUC__
#warning FIXME
#endif
	return Unknown;
}

QTextCodec *Message::checkCodec(QTextCodec *codec) const
{
	if (codec)
		return codec;

	if (d->codec)
		return d->codec;

//	if (d->engine)
//	return entityFromPrefix()->codec();

	return UTF8;
}

#ifndef KIRC_STRICT
/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
/*
bool Message::extractCtcpCommand()
{
	if (d->suffix.isEmpty())
		return false;

	uint len = message.length();

	if (message[0] == 1 && message[len-1] == 1)
	{
		ctcpline = ctcpUnquote(unquote(message.mid(1,len-2)));
		message.truncate(0);

				msg.d->ctcpMessage = new Message(msg.d->engine);
				msg.d->ctcpMessage->d->raw = msg.d->ctcpRaw.latin1();

				int space = msg.d->ctcpRaw.find(' ');
				if (!matchForIRCRegExp(msg.d->ctcpMessage->d->raw, codec, *msg.d->ctcpMessage))
				{
					if (space > 0)
						msg.d->ctcpMessage->d->command = msg.d->ctcpRaw.mid(0, space).upper();
					else
						msg.d->ctcpMessage->d->command = msg.d->ctcpRaw.upper();
				}

				if (space > 0)
					msg.d->ctcpMessage->d->ctcpRaw = msg.d->ctcpRaw.mid(space).latin1();

		return true;
	}
	return false;
}
*/
#endif

