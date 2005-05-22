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

using namespace KIRC;

#ifndef _IRC_STRICTNESS_
QRegExp Message::sm_IRCNumericCommand("^\\d{1,3}$");

// TODO: This regexp parsing is no good. It's slower than it needs to be, and
// is not codec-safe since QString requires a codec. NEed to parse this with
// our own parsing class that operates on the raw QCStrings
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

QString Message::format(const QString &command, const QStringList &args, const QString &suffix)
{
	QString msg = command;

	// FIXME: use a while loop instead and quote the arguments
	if (!args.isEmpty())
		msg += QChar(' ') + args.join(QChar(' ')).stripWhiteSpace(); // some extra check should be done here

	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + suffix;

	return msg;
}

QString Message::quote(const QString &str)
{
	QString tmp = str;
	QChar q('\020');
	tmp.replace(q, q+QString(q));
	tmp.replace(QChar('\r'), q+QString::fromLatin1("r"));
	tmp.replace(QChar('\n'), q+QString::fromLatin1("n"));
	tmp.replace(QChar('\0'), q+QString::fromLatin1("0"));
	return tmp;
}

// FIXME: The unquote system is buggy.
QString Message::unquote(const QString &str)
{
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
}

QString Message::ctcpQuote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( (char)1, QString::fromLatin1("\\1"));
	return tmp;
}

QString Message::ctcpUnquote(const QString &str)
{
	QString tmp = str;
	tmp.replace("\\\\", "\\");
	tmp.replace("\\1", "\1" );
	return tmp;
}

void Message::writeRawMessage(Engine *engine, const QTextCodec *codec, const QString &str)
{
	// FIXME: Really handle this
	if (!engine->socket())
	{
		kdDebug(14121) << k_funcinfo << "Not connected while attempting to write:" << str << endl;
		return;
	}

	QString txt = str + QString::fromLatin1("\r\n");

	QCString s(codec->fromUnicode(txt));
        kdDebug(14120) << "Message is " << s.length() << " chars" << endl;
	// FIXME: Should check the amount of data really writen.
	int wrote = engine->socket()->writeBlock(s.data(), s.length());

	kdDebug(14121) << QString::fromLatin1("(%1 bytes) >> %2").arg(wrote).arg(str) << endl;
}

void Message::writeMessage(Engine *engine, const QTextCodec *codec, const QString &message)
{
	writeRawMessage(engine, codec, quote(message));
}

void Message::writeMessage(Engine *engine, const QTextCodec *codec,
	const QString &command, const QStringList &args, const QString &suffix)
{
	QString msg = command;

	if (!args.isEmpty())
		msg += QChar(' ') + args.join(QChar(' ')).stripWhiteSpace(); // some extra check should be done here

	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + suffix;

	writeMessage(engine, codec, msg);
}

void Message::writeCtcpMessage(Engine *engine, const QTextCodec *codec,
		const QString &command, const QString&to,
		const QString &ctcpMessage)
{
	writeMessage(engine, codec, command, to, QChar(0x01) + ctcpQuote(ctcpMessage) + QChar(0x01));
}

void Message::writeCtcpMessage(Engine *engine, const QTextCodec *codec,
		const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix )
{
	QString ctcpMsg = ctcpCommand;

	if (!ctcpArgs.isEmpty())
		ctcpMsg += QChar(' ') + ctcpArgs.join(QChar(' ')).stripWhiteSpace(); // some extra check should be done here

	if (!ctcpSuffix.isNull())
		ctcpMsg += QString::fromLatin1(" :") + ctcpSuffix;

	writeMessage(engine, codec, command, to, suffix + QChar(0x01) + ctcpQuote(ctcpMsg) + QChar(0x01));
}

Message Message::parse(Engine *engine, bool *parseSuccess)
{
	Message msg(engine);

	if (parseSuccess)
		*parseSuccess=false;

	if (engine->socket()->canReadLine())
	{
		QCString raw = engine->socket()->readLine();
		raw.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)
		kdDebug(14121) << "<< " << raw << endl;

		msg.m_raw = raw;

		if (matchForIRCRegExp(msg))
		{
			if (parseSuccess)
				*parseSuccess = true;
		}
		else
			kdDebug(14120) << k_funcinfo << "Unmatched line: \"" << raw << "\"" << endl;
	}

	return msg;
}

bool Message::matchForIRCRegExp(Message &msg)
{
	if(matchForIRCRegExp(sm_IRCCommandType1, msg))
		return true;
#ifdef _IRC_STRICTNESS_
	if(matchForIRCRegExp(sm_IRCCommandType2, msg))
		return true;
#endif // _IRC_STRICTNESS_
	return false;
}

bool Message::matchForIRCRegExp(QRegExp &regexp, Message &msg)
{
	QString raw = QString::fromLatin1(msg.m_raw);

	if( regexp.exactMatch(raw) )
	{
		// Fixme QT4: do QByteArray = QString.latin1() directly
		msg.m_prefix  = QCString(regexp.cap(1).latin1());
		msg.m_command = QCString(regexp.cap(2).latin1());
		msg.m_args    = QCString(regexp.cap(3).latin1());
//		msg.m_argList = QStringList::split(' ', m_args);
		msg.m_suffix  = QCString(regexp.cap(4).latin1());

#ifndef _IRC_STRICTNESS_
		extractCtcpCommand(msg);
#endif // _IRC_STRICTNESS_

		return true;
	}
	return false;
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
bool Message::extractCtcpCommand(Message &msg)
{
//	if (msg.m_suffix.isEmpty()) // QT4
	if (msg.m_suffix.isNull() || msg.m_suffix.size() == 0 )
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

Message::Message(KIRC::Engine *engine)
	: m_engine(engine),
	  m_ctcpMessage(0)
{
}

Message::Message(const Message &obj)
        : m_ctcpMessage(0)
{
	m_engine = obj.m_engine;
	m_raw = obj.m_raw;

	m_prefix = obj.m_prefix;
	m_command = obj.m_command;
	m_args = obj.m_args;
	m_suffix = obj.m_suffix;

	if (obj.m_ctcpMessage)
		m_ctcpMessage = new Message(obj.m_ctcpMessage);
}

Message::Message(const Message *obj)
	: m_ctcpMessage(0)
{
	m_engine = obj->m_engine;
	m_raw = obj->m_raw;

	m_prefix = obj->m_prefix;
	m_command = obj->m_command;
	m_args = obj->m_args;
	m_suffix = obj->m_suffix;

	if (obj->m_ctcpMessage)
		m_ctcpMessage = new Message(obj->m_ctcpMessage);
}

Message::~Message()
{
	if (m_ctcpMessage)
		delete m_ctcpMessage;
}

bool Message::isNumeric() const
{
	return sm_IRCNumericCommand.exactMatch(m_command);
}

bool Message::isValid() const
{
//	This could/should be more complex but the message validity is tested durring the parsing
//	So this is enougth as we don't allow the editing the content.
	return !m_command.isEmpty();
}

void Message::dump() const
{
	kdDebug(14120)	<< "Raw:" << m_raw << endl
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

EntityPtr Message::entityFromPrefix() const
{
	return m_engine->getEntity(m_prefix);
}

EntityPtr Message::entityFromArg(size_t i) const
{
	return m_engine->getEntity(m_argList[i]);
}

QByteArray Message::rawLine() const
{
	return m_raw;
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

QValueList<QByteArray> Message::rawArgList() const
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
	return codec ? codec : m_engine->defaultCodec();
}
