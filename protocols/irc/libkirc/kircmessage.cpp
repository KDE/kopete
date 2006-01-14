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

// FIXME: Remove the following dependencies.
#include "kopetemessage.h"
#include "ksparser.h"

#include <kdebug.h>
#include <kextsock.h>
#include <klocale.h>
#include <kmessagebox.h>

using namespace KIRC;

#ifndef _IRC_STRICTNESS_
QRegExp Message::m_IRCNumericCommand("^\\d{1,3}$");

// TODO: This regexp parsing is no good. It's slower than it needs to be, and
// is not codec-safe since QString requires a codec. NEed to parse this with
// our own parsing class that operates on the raw QCStrings
QRegExp Message::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{1,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$");
	// Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
QRegExp Message::m_IRCNumericCommand("^\\d{3,3}$");

QRegExp Message::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$");
QRegExp Message::m_IRCCommandType2(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$");
#endif // _IRC_STRICTNESS_

Message::Message()
	: m_ctcpMessage(0)
{
}

Message::Message(const Message &obj)
        : m_ctcpMessage(0)
{
	m_raw = obj.m_raw;

	m_prefix = obj.m_prefix;
	m_command = obj.m_command;
	m_args = obj.m_args;
	m_suffix = obj.m_suffix;

	m_ctcpRaw = obj.m_ctcpRaw;

	if (obj.m_ctcpMessage)
		m_ctcpMessage = new Message(obj.m_ctcpMessage);
}

Message::Message(const Message *obj)
	: m_ctcpMessage(0)
{
	m_raw = obj->m_raw;

	m_prefix = obj->m_prefix;
	m_command = obj->m_command;
	m_args = obj->m_args;
	m_suffix = obj->m_suffix;

	m_ctcpRaw = obj->m_ctcpRaw;

	if (obj->m_ctcpMessage)
		m_ctcpMessage = new Message(obj->m_ctcpMessage);
}

Message::~Message()
{
	if (m_ctcpMessage)
		delete m_ctcpMessage;
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
		msg = msg.stripWhiteSpace() + QString::fromLatin1(" :") + suffix;

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

Message Message::parse(Engine *engine, const QTextCodec *codec, bool *parseSuccess)
{
	if (parseSuccess)
		*parseSuccess=false;

	if (engine->socket()->canReadLine())
	{
		QCString raw(engine->socket()->bytesAvailable()+1);
		Q_LONG length = engine->socket()->readLine(raw.data(), raw.count());

		if( length > -1 )
		{
			raw.resize( length );

			// Remove trailing '\r\n' or '\n'.
			//
			// Some servers send '\n' instead of '\r\n' that the RFCs say they should be sending.

			if (length > 1 && raw.at(length-2) == '\n') {
				raw.at(length-2) = '\0';
			}
			if (length > 2 && raw.at(length-3) == '\r') {
				raw.at(length-3) = '\0';
			}

			kdDebug(14121) << "<< " << raw << endl;

			Message msg;
			if(matchForIRCRegExp(raw, codec, msg))
			{
				if(parseSuccess)
					*parseSuccess = true;
			}
			else
			{
				kdDebug(14120) << k_funcinfo << "Unmatched line: \"" << raw << "\"" << endl;
			}

			return msg;
		}
		else
			kdWarning(14121) << k_funcinfo << "Failed to read a line while canReadLine returned true!" << endl;
	}

	return Message();
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

	char b[3] = { 020, 020, '\0' };
	const char b2[2] = { 020, '\0' };

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

bool Message::matchForIRCRegExp(const QCString &line, const QTextCodec *codec, Message &message)
{
	if(matchForIRCRegExp(m_IRCCommandType1, codec, line, message))
		return true;
#ifdef _IRC_STRICTNESS_
	if(!matchForIRCRegExp(m_IRCCommandType2, codec, line, message))
		return true;
#endif // _IRC_STRICTNESS_
	return false;
}

// FIXME: remove the decodeStrings calls or update them.
// FIXME: avoid the recursive call, it make the ctcp command unquoted twice (wich is wrong, but valid in most of the cases)
bool Message::matchForIRCRegExp(QRegExp &regexp, const QTextCodec *codec, const QCString &line, Message &msg )
{
	if( regexp.exactMatch( codec->toUnicode(line) ) )
	{
		msg.m_raw = line;
		msg.m_prefix  = unquote(regexp.cap(1));
		msg.m_command = unquote(regexp.cap(2));
		msg.m_args = QStringList::split(' ', regexp.cap(3));

		QCString suffix = codec->fromUnicode(unquote(regexp.cap(4)));
		if (!suffix.isNull() && suffix.length() > 0)
		{
			QCString ctcpRaw;
			if (extractCtcpCommand(suffix, ctcpRaw))
			{
				msg.m_ctcpRaw = codec->toUnicode(ctcpRaw); 

				msg.m_ctcpMessage = new Message();
				msg.m_ctcpMessage->m_raw = codec->fromUnicode(ctcpUnquote(msg.m_ctcpRaw));

				int space = ctcpRaw.find(' ');
				if (!matchForIRCRegExp(msg.m_ctcpMessage->m_raw, codec, *msg.m_ctcpMessage))
				{
					QCString command;
					if (space > 0)
						command = ctcpRaw.mid(0, space).upper();
					else
						command = ctcpRaw.upper();
					msg.m_ctcpMessage->m_command =
						Kopete::Message::decodeString( KSParser::parse(command), codec );
				}

				if (space > 0)
					msg.m_ctcpMessage->m_ctcpRaw =
						Kopete::Message::decodeString( KSParser::parse(ctcpRaw.mid(space)), codec );
			}

			msg.m_suffix = Kopete::Message::decodeString( KSParser::parse(suffix), codec );
		}
		else
			msg.m_suffix = QString::null;
		return true;
	}
	return false;
}

void Message::decodeAgain( const QTextCodec *codec )
{
	matchForIRCRegExp(m_raw, codec, *this);
}

// FIXME: there are missing parts
QString Message::toString() const
{
	if( !isValid() )
		return QString::null;

	QString msg = m_command;
	for (QStringList::ConstIterator it = m_args.begin(); it != m_args.end(); ++it)
		msg += QChar(' ') + *it;
	if (!m_suffix.isNull())
		msg += QString::fromLatin1(" :") + m_suffix;

	return msg;
}

bool Message::isNumeric() const
{
	return m_IRCNumericCommand.exactMatch(m_command);
}

bool Message::isValid() const
{
//	This could/should be more complex but the message validity is tested durring the parsing
//	So this is enougth as we don't allow the editing the content.
	return !m_command.isEmpty();
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command.
 * FIXME: The code currently only match for a textual message or a ctcp message not both mixed as it can be (even if very rare).
 */
bool Message::extractCtcpCommand(QCString &message, QCString &ctcpline)
{
	uint len = message.length();

	if( message[0] == 1 && message[len-1] == 1 )
	{
		ctcpline = message.mid(1,len-2);
		message.truncate(0);

		return true;
	}

	return false;
}

void Message::dump() const
{
	kdDebug(14120)	<< "Raw:" << m_raw << endl
			<< "Prefix:" << m_prefix << endl
			<< "Command:" << m_command << endl
			<< "Args:" << m_args << endl
			<< "Suffix:" << m_suffix << endl
			<< "CtcpRaw:" << m_ctcpRaw << endl;
	if(m_ctcpMessage)
	{
		kdDebug(14120) << "Contains CTCP Message:" << endl;
		m_ctcpMessage->dump();
	}
}
