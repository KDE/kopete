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

#include "kircengine.h"
#include "kircmessage.h"

#include <kdebug.h>

#include <QSharedData>
#include <QTextCodec>

using namespace KIRC;

//QRegExp Message::sm_("^()\\r\\n$")

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

class KIRC::MessagePrivate
	: public QSharedData
{
public:
	QByteArray line;
	QByteArray prefix;
	QByteArray command;
	QByteArray args;
	QByteArrayList argList;
	QByteArray suffix;

	QList<KIRC::Message> ctcpMessages;

	bool valid:1; // if the message was parsed successfuly
	bool dirty:1; // if the message contents is modified, so should be rebuilded
};

QByteArray Message::format(
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

QByteArray Message::formatCtcp(const QByteArray &str)
{
	#warning implement me
//	return QChar(0x01) + ctcpQuote(str) + QChar(0x01);
	return str;
}

QByteArray Message::quote(const QByteArray &str)
{
	#warning implement me
/*
	QString tmp = str;
	QChar q('\020');
	tmp.replace(q, q+QString(q));
	tmp.replace(QChar('\r'), q+QString::fromLatin1("r"));
	tmp.replace(QChar('\n'), q+QString::fromLatin1("n"));
	tmp.replace(QChar('\0'), q+QString::fromLatin1("0"));
	return tmp;
*/
	return str;
}

// FIXME: The unquote system is buggy.
QByteArray Message::unquote(const QByteArray &str)
{
	#warning implement me
/*
	QString tmp = str;

	char b[3];
	b[0] = 20; b[1] = 20; b[2] = '\0';
	char b2[2];
	b2[0] = (char)20; b2[1] = '\0';

	tmp.replace( b, b2 );
	b[1] = 'r';
	tmp.replace( b, "\r");
	b[1] = 'n';
	tmp.replace( b, "\n");
	b[1] = '0';
	tmp.replace( b, "\0");

	return tmp;
*/
	return str;
}

QByteArray Message::ctcpQuote(const QByteArray &str)
{
	#warning implement me
/*
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( (char)1, QString::fromLatin1("\\1"));
	return tmp;
*/
	return str;
}

QByteArray Message::ctcpUnquote(const QByteArray &str)
{
	#warning implement me
/*
	QString tmp = str;
	tmp.replace("\\\\", "\\");
	tmp.replace("\\1", "\1" );
	return tmp;
*/
}

Message::Message()
	: d(new MessagePrivate())
{
}

Message::Message(const Message &o)
        : d(o.d)
{
}

Message &Message::operator = (const Message &o)
{
	d = o.d;
	return *this;
}

QByteArray Message::rawLine() const
{
	return d->line;
}
/*
Message &Message::setLine(const QByteArray &line)
{
	d->line = line;
	d->valid = false;

	// Match a regexp instead of the replace ...
//	d->line.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
 
	#warning implement me: parsing
/*
	QList<QByteArray> tokens = line.split(' ');

	while (tokens.size() > 0)
	{
	}

	i = 0;

	while (line.length() > 0)
	{
		token = line.mid(, i);
	}

	if (regexp.exactMatch(d->raw))
	{
//		d->line    = regexp.cap(1).latin1();
		d->prefix  = regexp.cap(1).latin1();
		d->command = regexp.cap(2).latin1();
		d->args    = regexp.cap(3).latin1();
//		d->argList = QStringList::split(' ', d->args);
		d->suffix  = regexp.cap(4).latin1();

#ifndef _IRC_STRICTNESS_
		extractCtcpCommand();
#endif // _IRC_STRICTNESS_

		d->valid = true;
	}
*/
	return *this;
}
*/
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

QByteArray Message::rawArgs() const
{
	return d->args;
}

QByteArrayList Message::rawArgList() const
{
	return d->argList;
}

Message &Message::setArgList(const QByteArrayList &argList)
{
	if (d->argList != argList)
	{
		d->dirty = true;
		d->argList = argList;
	}
	return *this;
}

QByteArray Message::rawArg(size_t i) const
{
	return d->argList[i];
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
/*
Message &Message::setPrefix(const QString &prefix, QTextCodec *codec)
{
	return setPrefix(checkCodec(codec)->fromUnicode(prefix));
}
*/
QString Message::command(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->command);
}
/*
Message &Message::setCommand(const QString &command, QTextCodec *codec)
{
	return setCommand(checkCodec(codec)->fromUnicode(command));
}
*/
QString Message::args(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->args);
}
/*
Message &Message::setArgs(const QString &args, QTextCodec *codec)
{
	return setArgs(checkCodec(codec)->fromUnicode(prefix));
}
*/
QStringList Message::argList(QTextCodec *codec) const
{
	QStringList argList;
	codec = checkCodec(codec);

	for (size_t i=0; i < d->argList.size(); ++i)
		argList.append(codec->toUnicode(d->argList[i]));

	return argList;
}
/*
Message &Message::setArgList(const QStringList &argList, QTextCodec *codec)
{
	return setArgList(checkCodec(codec)->fromUnicode(prefix));
}
*/
QString Message::arg(size_t i, QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->argList[i]);
}
/*
Message &Message::setArg(const QString &arg, QTextCodec *codec)
{
	return setArg(checkCodec(codec)->fromUnicode(arg));
}
*/
QString Message::suffix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(d->suffix);
}
/*
Message &Message::setSuffix(const QString &suffix, QTextCodec *codec)
{
	return setSuffix(checkCodec(codec)->fromUnicode(prefix));
}
*/
bool Message::isValid() const
{
	return d->valid;
}

bool Message::isNumeric() const
{
	return sm_IRCNumericCommand.exactMatch(d->command);
}

void Message::dump() const
{
	kdDebug(14120)	<< "Line:" << d->line << endl
			<< "Prefix:" << d->prefix << endl
			<< "Command:" << d->command << endl
			<< "Args:" << d->args << endl
			<< "ArgList:" << d->argList << endl
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
EntityPtr Message::entityFromPrefix(KIRC::Engine *engine) const
{
	if (d->prefix.isEmpty())
		return engine->self();
	else
		return engine->getEntity(d->prefix);
}

EntityPtr Message::entityFromArg(KIRC::Engine *engine, size_t i) const
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

//	return entityFromPrefix()->codec();
	return 0;
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
#ifndef _IRC_STRICTNESS_
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
#endif // _IRC_STRICTNESS_

#include "kircmessage.moc"
