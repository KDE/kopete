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

#include "kirc.h"
#include "kircmessage.h"

#ifndef _IRC_STRICTNESS_
const QRegExp KIRCMessage::m_IRCCommandType1(QString::fromLatin1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$"));
	// Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
const QRegExp KIRCMessage::m_IRCCommandType1(QString::fromLatin1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$"));
const QRegExp KIRCMessage::m_IRCCommandType2(QString::fromLatin1(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$"));
#endif // _IRC_STRICTNESS_

const QRegExp KIRCMessage::m_IRCNumericCommand(QString::fromLatin1("^\\d{3,3}$"));

KIRCMessage::KIRCMessage() : m_ctcpMessage(0)
{
}

KIRCMessage::KIRCMessage(const KIRCMessage &obj) : m_ctcpMessage(0)
{
	m_raw = obj.m_raw;

	m_prefix = obj.m_prefix;
	m_command = obj.m_command;
	m_args = obj.m_args;
	m_suffix = obj.m_suffix;

	m_ctcpRaw = obj.m_ctcpRaw;

	if(obj.m_ctcpMessage)
		m_ctcpMessage = new KIRCMessage(obj.m_ctcpMessage);
}

KIRCMessage::KIRCMessage(const KIRCMessage *obj) : m_ctcpMessage(0)
{
	m_raw = obj->m_raw;

	m_prefix = obj->m_prefix;
	m_command = obj->m_command;
	m_args = obj->m_args;
	m_suffix = obj->m_suffix;

	m_ctcpRaw = obj->m_ctcpRaw;

	if(obj->m_ctcpMessage)
		m_ctcpMessage = new KIRCMessage(obj->m_ctcpMessage);
}

KIRCMessage::~KIRCMessage()
{
	if(m_ctcpMessage) delete m_ctcpMessage;
}

KIRCMessage KIRCMessage::writeRawMessage(KIRC *engine, const QTextCodec *codec, const QString &str)
{
	QCString s;
	QString txt = str + QString::fromLatin1("\r\n");

	s = codec->fromUnicode(txt);

	kdDebug(14121) << ">> " << s;

	// FIXME: Should check the amount of data really writen.
	engine->socket()->writeBlock(s.data(), s.length());

	return parse(str);
}

KIRCMessage KIRCMessage::writeMessage(KIRC *engine, const QTextCodec *codec, const QString &message)
{
	return writeRawMessage(engine, codec, quote(message));
}

KIRCMessage KIRCMessage::writeMessage(KIRC *engine, const QTextCodec *codec,
	const QString &command, const QStringList &args, const QString &suffix)
{
	QString msg = command;

	if (!args.isEmpty())
		msg += QChar(' ') + args.join( QChar(' ') ).stripWhiteSpace(); // some extra check should be done here

	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + suffix;

	return writeMessage(engine, codec, msg);
}

KIRCMessage KIRCMessage::writeCtcpMessage(KIRC *engine, const QTextCodec *codec,
		const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix )
{
	QString ctcpMsg = ctcpCommand;

	if (!ctcpArgs.isEmpty())
		ctcpMsg += QChar(' ') + ctcpArgs.join( QChar(' ') ).stripWhiteSpace(); // some extra check should be done here

	if (!ctcpSuffix.isNull())
		ctcpMsg += QString::fromLatin1(" :") + ctcpSuffix;

	return writeMessage(engine, codec, command, to, suffix + QChar(0x01) + ctcpQuote(ctcpMsg) + QChar(0x01));
}

KIRCMessage KIRCMessage::parse(KIRC *engine, const QTextCodec *codec, bool *parseSuccess)
{
	if(parseSuccess)
		*parseSuccess=false;

	if( engine->socket()->canReadLine() )
	{
		QCString raw;
		QString line;

		raw.resize(engine->socket()->bytesAvailable()+1);
		Q_LONG length = engine->socket()->readLine(raw.data(), raw.count());
		if( length > -1 )
		{
			raw.resize(length);
			raw.replace("\r\n",""); //remove the trailling \r\n if any(there must be in fact)

			line = codec->toUnicode(raw);

			kdDebug(14121) << "<< " << line << endl;

			KIRCMessage msg = parse( line, parseSuccess );
			msg.m_raw = raw;
			return msg;
		}
		else
			kdWarning(14121) << "Failed to read a line while canReadLine returned true!" << endl;
	}

	return KIRCMessage();
}

KIRCMessage KIRCMessage::parse(const QString &line, bool *parseSuccess)
{
	KIRCMessage msg;

	if(parseSuccess)
		*parseSuccess=false;

	QString newLine = unquote( line );
	if(matchForIRCRegExp(newLine, msg))
	{
		msg.m_prefix = ctcpUnquote(msg.m_prefix);
		msg.m_command = ctcpUnquote(msg.m_command);
		for (QStringList::Iterator it = msg.m_args.begin(); it != msg.m_args.end(); ++it)
			(*it) = ctcpUnquote(*it);

		if(extractCtcpCommand(msg.m_suffix, msg.m_ctcpRaw))
		{
			msg.m_ctcpRaw = ctcpUnquote(msg.m_ctcpRaw);
			kdDebug(14121) << "Found CTCP command:\"" << msg.m_ctcpRaw << "\"" << endl;
			msg.m_ctcpMessage = new KIRCMessage();
			msg.m_ctcpMessage->m_raw = msg.m_ctcpRaw.utf8();
			if(!matchForIRCRegExp(msg.m_ctcpRaw, *msg.m_ctcpMessage))
			{
				msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.section(' ', 0, 0).upper();
			}
			msg.m_ctcpMessage->m_ctcpRaw = msg.m_ctcpRaw.section(' ', 1);
		}
		msg.m_suffix = ctcpUnquote(msg.m_suffix);

		if(parseSuccess)
			*parseSuccess = true;
	}
	else
	{
//		KMessageBox::error(Kopete::UI::Global::mainWidget(), "\"" + line + "\"", "Unmatched line");
		kdDebug(14120) << "Unmatched line:\"" << newLine << "\"" << endl;
	}
	return msg;
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
	QString tmp = str;
	QChar q('\020');
	tmp.replace(q+QString(q), q);
	tmp.replace(q+QString::fromLatin1("r"), QChar('\r'));
	tmp.replace(q+QString::fromLatin1("n"), QChar('\n'));
	tmp.replace(q+QString::fromLatin1("0"), QChar('\0'));
	return tmp;
}

QString KIRCMessage::ctcpQuote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( QChar((uchar)0x01), QString::fromLatin1("\\1"));
	return tmp;
}

// FIXME: The unquote system is buggy.
QString KIRCMessage::ctcpUnquote(const QString &str)
{
	QString tmp = str;
	tmp.replace(QString::fromLatin1("\\\\"), QChar('\\'));
	tmp.replace(QString::fromLatin1("\\1"), QChar((uchar)0x01));
	return tmp;
}

bool KIRCMessage::matchForIRCRegExp(const QString &line,KIRCMessage &message)
{
	QRegExp IRCCommandType1(m_IRCCommandType1);
#ifdef _IRC_STRICTNESS_
	QRegExp IRCCommandType2(m_IRCCommandType2);
#endif // _IRC_STRICTNESS_

	if(matchForIRCRegExp(IRCCommandType1, line,
		message.m_prefix, message.m_command , message.m_args, message.m_suffix))
		return true;
#ifdef _IRC_STRICTNESS_
	if(!matchForIRCRegExp(IRCCommandType2, line,
		message.m_prefix, message.m_command , message.m_args, message.m_suffix))
		return true;
#endif // _IRC_STRICTNESS_
	return false;
}

bool KIRCMessage::matchForIRCRegExp(QRegExp &regexp, const QString &line,
				QString &prefix, QString &command, QStringList &args, QString &suffix)
{
	if(regexp.exactMatch(line))
	{
		prefix  = regexp.cap(1);
		command = regexp.cap(2);
		args    = QStringList::split(' ', regexp.cap(3).stripWhiteSpace());
		suffix  = regexp.cap(4);
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
	return m_IRCNumericCommand.exactMatch( m_command );
}

bool KIRCMessage::isValid() const
{
//	This could/should be more complex but the message validity is tested durring the parsing
//	So this is enougth as we don't allow the editing the content.
	return !m_command.isEmpty();
}

/* Return true if the given string is a special command string
 * (i.e start and finish with the ascii code \001), and the given
 * string is splited to get the first part of the message and fill the ctcp command. */
bool KIRCMessage::extractCtcpCommand(QString &str, QString &ctcpline)
{
	// This could also be done using a regexp like: "^([^\\x01])\\x01([^\\x01])\\x01$"
	// (make less code to test)
	int pos_begin = str.find(QChar(0x01));
	if (pos_begin==-1)
		return false;

	int pos_end = str.find(QChar(0x01), pos_begin+1);
	if (pos_end==-1||pos_end!=(int)str.length()-1)
		return false;

	ctcpline = str.mid(pos_begin+1, pos_end-pos_begin-1);
	str = str.mid(0, pos_begin);
	return true;
}

void KIRCMessage::dump() const
{
	kdDebug(14120)	<< "Raw:" << m_raw << endl
			<< "Prefix:" << m_prefix << endl
			<< "Command:" << m_command << endl
//			<< "Args:" << m_args << endl   //this does not compile on KDE 3.1
			<< "Suffix:" << m_suffix << endl
			<< "CtcpRaw:" << m_ctcpRaw << endl;
	if(m_ctcpMessage)
	{
		kdDebug(14120) << "Contains CTCP Message:" << endl;
		m_ctcpMessage->dump();
	}
}
