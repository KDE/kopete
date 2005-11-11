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

#include "kircsocket.h"
#include "kircmessageutil.h"

#include <kdebug.h>

#include <QPointer>
#include <QSharedData>
#include <QTextCodec>

using namespace KIRC;

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
class KIRC::Message::Private
	: public QSharedData
{
public:
	Private()
		: valid(false),
		  dirty(false)
	{ }

	QPointer<KIRC::Socket> socket;
	KIRC::Message::Direction direction;

	QByteArray line;
	QByteArray prefix;
	QByteArray command;
	QByteArrayList args;
	QByteArray suffix;

	QList<KIRC::Message> ctcpMessages;

	bool valid:1; // if the message was parsed successfuly
	bool dirty:1; // if the message contents is modified, so should be rebuilded
};

Message::Message()
	: d(new Private())
{
}

Message::Message(const QByteArray &rawLine, Direction direction)
	: d(new Private())
{
	setLine(rawLine);
	setDirection(direction);
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

Socket *Message::socket() const
{
	return d->socket;
}

Message &Message::setSocket(Socket *socket)
{
	d->socket = socket;
	return *this;
}

Message::Direction Message::direction() const
{
	return d->direction;
}

Message &Message::setDirection(Message::Direction direction)
{
	d->direction = direction;
	return *this;
}

QByteArray Message::rawLine() const
{
	return d->line;
}

Message &Message::setLine(const QByteArray &line)
{
	d->line = line;
	d->valid = false;
/*
	// Match a regexp instead of the replace ...
//	d->line.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
 
	#warning implement me: parsing

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
	return *this;
}

QByteArray Message::rawPrefix() const
{
	return d->prefix;
}

Message &Message::setPrefix(const QByteArray &prefix)
{
	if (d->prefix != prefix)
	{
		d->dirty = true;
		d->prefix = prefix;
	}
	return *this;
}

QByteArray Message::rawCommand() const
{
	return d->command;
}

Message &Message::setCommand(const QByteArray &command)
{
	if (d->command != command)
	{
		d->dirty = true;
		d->command = command;
	}
	return *this;
}

QByteArrayList Message::rawArgs() const
{
	return d->args;
}

Message &Message::setArgs(const QByteArrayList &args)
{
	if (d->args != args)
	{
		d->dirty = true;
		d->args = args;
	}
	return *this;
}

QByteArray Message::rawArg(size_t i) const
{
	return d->args[i];
}

QByteArray Message::rawSuffix() const
{
	return d->suffix;
}

Message &Message::setSuffix(const QByteArray &suffix)
{
	if (d->suffix != suffix)
	{
		d->dirty = true;
		d->suffix = suffix;
	}
	return *this;
}

QString Message::prefix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->prefix);
}

Message &Message::setPrefix(const QString &prefix, QTextCodec *codec)
{
	return setPrefix(checkCodec(codec)->fromUnicode(prefix));
}

QString Message::command(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->command);
}

Message &Message::setCommand(const QString &command, QTextCodec *codec)
{
	return setCommand(checkCodec(codec)->fromUnicode(command));
}

QStringList Message::args(QTextCodec *codec) const
{
	QStringList args;
	codec = checkCodec(codec);

	foreach(const QByteArray &arg, d->args)
		args.append(codec->toUnicode(arg));

	return args;
}

Message &Message::setArgs(const QStringList &args, QTextCodec *codec)
{
	QByteArrayList arrayList;
	codec = checkCodec(codec);

	foreach (const QString &arg, args)
		arrayList.append(codec->fromUnicode(arg));

	return setArgs(arrayList);
}

QString Message::arg(size_t i, QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->args[i]);
}
/*
Message &Message::setArgs(size_t i, QString arg, QTextCodec *codec)
{
	return setArg(i, checkCodec(codec)->fromUnicode(arg));
}
*/
QString Message::suffix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->suffix);
}

Message &Message::setSuffix(const QString &suffix, QTextCodec *codec)
{
	return setSuffix(checkCodec(codec)->fromUnicode(suffix));
}

bool Message::isValid() const
{
	return d->valid;
}

bool Message::isNumeric() const
{
#warning FIXME
//	return sm_IRCNumericCommand.exactMatch(d->command);
	return false;
}

void Message::dump() const
{
	kdDebug(14120)	<< "Line:" << d->line << endl
			<< "Prefix:" << d->prefix << endl
			<< "Command:" << d->command << endl
			<< "Args:" << d->args << endl
			<< "Suffix:" << d->suffix << endl;
/*
	if (d->ctcpMessage)
	{
		kdDebug(14120) << "Contains CTCP Message:" << endl;
		d->ctcpMessage->dump();
	}
*/
}
/*
Entity::Ptr Message::entityFromPrefix(KIRC::Engine *engine) const
{
	if (d->prefix.isEmpty())
		return engine->self();
	else
		return engine->getEntity(d->prefix);
}

Entity::Ptr Message::entityFromArg(KIRC::Engine *engine, size_t i) const
{
	return engine->getEntity(d->argList[i]);
}
*/
size_t Message::argsSize() const
{
	return d->args.size();
}

QTextCodec *Message::checkCodec(QTextCodec *codec) const
{
	if (codec)
		return codec;

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
bool Message::extractCtcpCommand()
{
	if (d->suffix.isEmpty())
		return false;
/*
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
*/
	return false;
}
#endif

