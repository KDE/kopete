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

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kextsock.h>

#include "kopetemessage.h"

#include "kirc.h"
#include "ksparser.h"
#include "kircmessage.h"

#ifndef _IRC_STRICTNESS_
QRegExp KIRCMessage::m_IRCNumericCommand("^\\d{1,3}$");

// TODO: This regexp parsing is no good. It's slower than it needs to be, and
// is not codec-safe since QString requires a codec. NEed to parse this with
// our own parsing class that operates on the raw QCStrings
QRegExp KIRCMessage::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{1,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$");
	// Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
QRegExp KIRCMessage::m_IRCNumericCommand("^\\d{3,3}$");

QRegExp KIRCMessage::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$");
QRegExp KIRCMessage::m_IRCCommandType2(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$");
#endif // _IRC_STRICTNESS_

KIRCMessage::KIRCMessage()
	: m_ctcpMessage(0)
{
}

KIRCMessage::KIRCMessage(const KIRCMessage &obj)
	: m_ctcpMessage(0)
{
	m_raw = obj.m_raw;

	m_prefix = obj.m_prefix;
	m_command = obj.m_command;
	m_args = obj.m_args;
	m_suffix = obj.m_suffix;

	m_ctcpRaw = obj.m_ctcpRaw;

	if (obj.m_ctcpMessage)
		m_ctcpMessage = new KIRCMessage(obj.m_ctcpMessage);
}

KIRCMessage::KIRCMessage(const KIRCMessage *obj)
	: m_ctcpMessage(0)
{
	m_raw = obj->m_raw;

	m_prefix = obj->m_prefix;
	m_command = obj->m_command;
	m_args = obj->m_args;
	m_suffix = obj->m_suffix;

	m_ctcpRaw = obj->m_ctcpRaw;

	if (obj->m_ctcpMessage)
		m_ctcpMessage = new KIRCMessage(obj->m_ctcpMessage);
}

KIRCMessage::~KIRCMessage()
{
	if (m_ctcpMessage)
		delete m_ctcpMessage;
}

void KIRCMessage::writeRawMessage(KIRC *engine, const QTextCodec *codec, const QString &str)
{
	QCString s;
	QString txt = str + QString::fromLatin1("\r\n");

	s = codec->fromUnicode(txt);

	kdDebug(14121) << ">> " << s;

	// FIXME: Should check the amount of data really writen.
	engine->socket()->writeBlock(s.data(), s.length());
}

void KIRCMessage::writeMessage(KIRC *engine, const QTextCodec *codec, const QString &message)
{
	writeRawMessage(engine, codec, quote(message));
}

void KIRCMessage::writeMessage(KIRC *engine, const QTextCodec *codec,
	const QString &command, const QStringList &args, const QString &suffix)
{
	QString msg = command;

	if (!args.isEmpty())
		msg += QChar(' ') + args.join(QChar(' ')).stripWhiteSpace(); // some extra check should be done here

	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + suffix;

	writeMessage(engine, codec, msg);
}

void KIRCMessage::writeCtcpMessage(KIRC *engine, const QTextCodec *codec,
		const QString &command, const QString&to,
		const QString &ctcpMessage)
{
	writeMessage(engine, codec, command, to, QChar(0x01) + ctcpQuote(ctcpMessage) + QChar(0x01));
}

void KIRCMessage::writeCtcpMessage(KIRC *engine, const QTextCodec *codec,
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

KIRCMessage KIRCMessage::parse(KIRC *engine, const QTextCodec *codec, bool *parseSuccess)
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
			raw.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)

			kdDebug(14121) << "<< " << raw << endl;

			KIRCMessage msg;
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

	return KIRCMessage();
}

QString KIRCMessage::quote(const QString &str)
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
QString KIRCMessage::unquote(const QString &str)
{
#if !KDE_IS_VERSION( 3, 1, 90 )
	// Workaround a KStringHandler bug.
	if (str.isEmpty())
		return QString::fromLatin1("");
#endif

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

QString KIRCMessage::ctcpQuote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( (char)1, QString::fromLatin1("\\1"));
	return tmp;
}

QString KIRCMessage::ctcpUnquote(const QString &str)
{
	QString tmp = str;
	tmp.replace("\\\\", "\\");
	tmp.replace("\\1", "\1" );
	return tmp;
}

bool KIRCMessage::matchForIRCRegExp(const QString &line, const QTextCodec *codec, KIRCMessage &message)
{
	if(matchForIRCRegExp(m_IRCCommandType1, codec, line, message))
		return true;
#ifdef _IRC_STRICTNESS_
	if(!matchForIRCRegExp(m_IRCCommandType2, codec, line, message)
		return true;
#endif // _IRC_STRICTNESS_
	return false;
}

// FIXME: remove the decodeStrings calls or update them.
// FIXME: avoid the recursive call, it make the ctcp command unquoted twice (wich is wrong, but valid in most of the cases)
bool KIRCMessage::matchForIRCRegExp(QRegExp &regexp, const QTextCodec *codec, const QString &line, KIRCMessage &msg )
{
	if (regexp.exactMatch(line))
	{
		msg.m_raw = line;
		msg.m_prefix  = unquote(regexp.cap(1));
		msg.m_command = unquote(regexp.cap(2));
		msg.m_args = QStringList::split(' ', regexp.cap(3));

		QString suffix = regexp.cap(4);
		if (!suffix.isNull() && suffix.length() > 0)
		{
			if (extractCtcpCommand(suffix, msg.m_ctcpRaw))
			{
				msg.m_ctcpMessage = new KIRCMessage();
				msg.m_ctcpMessage->m_raw = msg.m_ctcpRaw;

				int space = msg.m_ctcpRaw.find(' ');
				if (!matchForIRCRegExp(msg.m_ctcpMessage->m_raw, codec, *msg.m_ctcpMessage))
				{
					if (space > 0)
						msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.mid(0, space).upper();
					else
						msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.upper();
				}

				if (space > 0)
					msg.m_ctcpMessage->m_ctcpRaw = KopeteMessage::decodeString(KSParser::parse(msg.m_ctcpRaw.mid(space)).latin1(), codec);
			}

			msg.m_suffix = KopeteMessage::decodeString(KSParser::parse(suffix).latin1(), codec);
		}
		else
			msg.m_suffix = QString::null;
		return true;
	}
	return false;
}


// FIXME: there are missing parts
QString KIRCMessage::toString() const
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

bool KIRCMessage::isNumeric() const
{
	return m_IRCNumericCommand.exactMatch(m_command);
}

bool KIRCMessage::isValid() const
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
bool KIRCMessage::extractCtcpCommand(QString &message, QString &ctcpline)
{
	message = unquote(message);
	uint len = message.length();
	if (message[0] == 1 && message[len-1] == 1)
	{
		ctcpline = ctcpUnquote(message.mid(1, len-2));
		message = QString::null;
		kdDebug(14121) << k_funcinfo << ctcpline << endl;
		return true;
	}
	return false;
}

void KIRCMessage::dump() const
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
