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

#include <qtextcodec.h>

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

QString Message::format(
		const QString &command,
		const QStringList &args,
		const QString &suffix)
{
	#warning implement me
	QString msg = command;

	return msg;
}

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

QString Message::formatCtcp(const QString &str)
{
	return QChar(0x01) + ctcpQuote(str) + QChar(0x01);
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

QString Message::ctcpQuote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( (char)1, QString::fromLatin1("\\1"));
	return tmp;
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

Message::Message(const QByteArray &msg)
	: m_ctcpMessage(0)
{
	parse(msg);
}

Message::Message(const Message &obj)
        : m_ctcpMessage(0)
{
	m_line = obj.m_line;

	m_prefix = obj.m_prefix;
	m_command = obj.m_command;
	m_args = obj.m_args;
	m_suffix = obj.m_suffix;

	if (obj.m_ctcpMessage)
		m_ctcpMessage = new Message(*obj.m_ctcpMessage);
}

Message::~Message()
{
	if (m_ctcpMessage)
		delete m_ctcpMessage;
}

bool Message::isValid() const
{
	return m_valid;
}

bool Message::isNumeric() const
{
	return sm_IRCNumericCommand.exactMatch(m_command);
}

void Message::dump() const
{
	kdDebug(14120)	<< "Line:" << m_line << endl
			<< "Prefix:" << m_prefix << endl
			<< "Command:" << m_command << endl
			<< "Args:" << m_args << endl
			<< "ArgList:" << m_argList << endl
			<< "Suffix:" << m_suffix << endl;
	if (m_ctcpMessage)
	{
		kdDebug(14120) << "Contains CTCP Message:" << endl;
		m_ctcpMessage->dump();
	}
}
/*
EntityPtr Message::entityFromPrefix(KIRC::Engine *engine) const
{
	if (m_prefix.isEmpty())
		return engine->self();
	else
		return engine->getEntity(m_prefix);
}

EntityPtr Message::entityFromArg(KIRC::Engine *engine, size_t i) const
{
	return engine->getEntity(m_argList[i]);
}
*/
QByteArray Message::rawLine() const
{
	return m_line;
}

QByteArray Message::rawPrefix() const
{
	return m_prefix;
}

QByteArray Message::rawCommand() const
{
	return m_command;
}

QByteArray Message::rawArgs() const
{
	return m_args;
}

QByteArrayList Message::rawArgList() const
{
	return m_argList;
}

QByteArray Message::rawArg(size_t i) const
{
	return m_argList[i];
}

QByteArray Message::rawSuffix() const
{
	return m_suffix;
}

QString Message::prefix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(m_prefix);
}

QString Message::command(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(m_command);
}

QString Message::args(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(m_args);
}

QStringList Message::argList(QTextCodec *codec) const
{
	QStringList argList;
	codec = checkCodec(codec);

	for (size_t i=0; i < m_argList.size(); ++i)
		argList.append(codec->toUnicode(m_argList[i]));

	return argList;
}

QString Message::arg(size_t i, QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(m_argList[i]);
}

QString Message::suffix(QTextCodec *codec) const
{
	return checkCodec(codec)->toUnicode(m_suffix);
}

size_t Message::argsSize() const
{
	return m_args.size();
}

QTextCodec *Message::checkCodec(QTextCodec *codec) const
{
	if (codec)
		return codec;

//	return entityFromPrefix()->codec();
	return 0;
}

bool Message::parse(const QByteArray &line)
{
	QString match;

	m_line = line;
	m_valid = false;

	// Match a regexp instead of the replace ...
//	m_line.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
 
	#warning implement me: parsing
/*
	if (regexp.exactMatch(QString::fromLatin1(m_raw)))
	{
		// Fixme QT4: do QByteArray = QString.latin1() directly
//		m_line    = QCString(regexp.cap(1).latin1());
		m_prefix  = QCString(regexp.cap(1).latin1());
		m_command = QCString(regexp.cap(2).latin1());
		m_args    = QCString(regexp.cap(3).latin1());
//		m_argList = QStringList::split(' ', m_args);
		m_suffix  = QCString(regexp.cap(4).latin1());

#ifndef _IRC_STRICTNESS_
		extractCtcpCommand();
#endif // _IRC_STRICTNESS_

		m_valid = true;
	}
*/
	return m_valid;
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
#ifndef _IRC_STRICTNESS_
bool Message::extractCtcpCommand()
{
	if (m_suffix.isEmpty())
		return false;
/*
	uint len = message.length();

	if (message[0] == 1 && message[len-1] == 1)
	{
		ctcpline = ctcpUnquote(unquote(message.mid(1,len-2)));
		message.truncate(0);

				msg.m_ctcpMessage = new Message(msg.m_engine);
				msg.m_ctcpMessage->m_raw = msg.m_ctcpRaw.latin1();

				int space = msg.m_ctcpRaw.find(' ');
				if (!matchForIRCRegExp(msg.m_ctcpMessage->m_raw, codec, *msg.m_ctcpMessage))
				{
					if (space > 0)
						msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.mid(0, space).upper();
					else
						msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.upper();
				}

				if (space > 0)
					msg.m_ctcpMessage->m_ctcpRaw = msg.m_ctcpRaw.mid(space).latin1();

		return true;
	}
*/
	return false;
}
#endif // _IRC_STRICTNESS_
