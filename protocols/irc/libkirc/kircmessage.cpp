/*
    kircmessage.cpp - IRC Client

    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003      by the Kopete developers <kopete-devel@kde.org>

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

#include "kircmessage.h"

#ifndef _IRC_STRICTNESS_
const QRegExp KIRCMessage::m_IRCCommandType1(QString::fromLatin1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*)*) ?(?: :([^\\r\\n]*))?\\r\\n$"));
	// Extra end arg space check -------------------------^
	// Buggy here --------------------------------------------------^
#else // _IRC_STRICTNESS_
const QRegExp KIRCMessage::m_IRCCommandType1(QString::fromLatin1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,14})(?: :([^\\r\\n]*))?\\r\\n$"));
const QRegExp KIRCMessage::m_IRCCommandType2(QString::fromLatin1(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: ([^\\r\\n])*)?\\r\\n$"));
#endif // _IRC_STRICTNESS_

const QRegExp KIRCMessage::m_IRCNumericCommand(QString::fromLatin1("^\\d{3,3}$"));


KIRCMessage::KIRCMessage(const KIRCMessage &obj)
	: m_ctcpMessage(0)
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

KIRCMessage::KIRCMessage(const KIRCMessage *obj)
	: m_ctcpMessage(0)
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

KIRCMessage KIRCMessage::writeString(QIODevice *dev, const QString &str, QTextCodec *codec)
{
	QCString s;
	QString txt = str + QString::fromLatin1("\r\n");

	QTextCodec *_codec = codec?codec:QTextCodec::codecForContent(txt, txt.length());
	if (codec)
		s = _codec->fromUnicode(txt);
	else
		s = txt.local8Bit();

	kdDebug(14121) << ">> " << s;
	// FIXME: Should check the amount of data really writen.
	dev->writeBlock(s.data(), s.length());
	return parse(txt);
}

KIRCMessage KIRCMessage::writeMessage(QIODevice *dev,
		const QString &command, const QStringList &args, const QString &suffix,
		QTextCodec *codec)
{
	QString msg = command;
	for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
		msg += QChar(' ') + *it;
	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + quote(suffix);

	return writeString(dev, msg, codec);
}

KIRCMessage KIRCMessage::writeMessage(QIODevice *dev,
		const QString &command, const QString &arg, const QString &suffix,
		QTextCodec *codec)
{
	QString msg = command;
	if (!arg.isNull())
		msg += QChar(' ') + arg;
	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + quote(suffix);

	return writeString(dev, msg, codec);
}

KIRCMessage KIRCMessage::writeCtcpMessage(QIODevice *dev,
		const QString &command, const QString &to /*prefix*/, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix,
		QTextCodec *codec)
{
	QString ctcpMessage = ctcpCommand;
	for (QStringList::ConstIterator it = ctcpArgs.begin(); it != ctcpArgs.end(); ++it)
		ctcpMessage += QChar(' ') + *it;
	if (!ctcpSuffix.isNull())
		ctcpMessage += QChar(' ') + ctcpSuffix;
	if (!ctcpSuffix.isNull())
		ctcpMessage += QString::fromLatin1(" :") + ctcpSuffix;

	return writeMessage(	dev, command, getNickFromPrefix(to),
				suffix + QChar(0x01) + quote(ctcpMessage) + QChar(0x01), codec);
}

// if codec==0 => autodetect
KIRCMessage KIRCMessage::parse(KBufferedIO *dev, bool *parseSuccess, QTextCodec *codec)
{
	if(parseSuccess) *parseSuccess=false;
	if(dev->canReadLine())
	{
		QCString raw;
		QString line;

		raw.resize(dev->bytesAvailable()+1);
		Q_LONG length = dev->readLine(raw.data(), raw.count());
		if (length > -1)
		{
			raw.resize(length);
			raw = unquote(raw);
			kdDebug(14121) << "<< " << raw; //endl allready included

			QTextCodec *_codec = codec?codec:QTextCodec::codecForContent(raw.data(), raw.length());

			if (codec)
				line = _codec->toUnicode(raw);
			else
				line = raw;

			KIRCMessage msg = parse(line, parseSuccess);
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

	if(parseSuccess) *parseSuccess=false;
	if(matchForIRCRegExp(line,msg))
	{
		if(extractCtcpCommand(msg.m_suffix, msg.m_ctcpRaw))
		{
			msg.m_raw = unquote(msg.m_ctcpRaw);
			kdDebug(14121) << "Found CTCP command:\"" << msg.m_ctcpRaw << "\"" << endl;
			msg.m_ctcpMessage = new KIRCMessage();
			QString tmpCtcpLine = msg.m_ctcpRaw + QString::fromLatin1("\r\n");
			msg.m_ctcpMessage->m_raw = tmpCtcpLine.latin1();
			if(!matchForIRCRegExp(tmpCtcpLine, *msg.m_ctcpMessage))
			{
				msg.m_ctcpMessage->m_command = msg.m_ctcpRaw.section(' ', 0, 0).upper();
			}
			msg.m_ctcpMessage->m_ctcpRaw = msg.m_ctcpRaw.section(' ', 1);
		}
		if(parseSuccess) *parseSuccess=true;
	}
	else
	{
		KMessageBox::error(0, "\"" + line + "\"", "Unmatched line");
		kdDebug(14120) << "Unmatched line:\"" << line << "\"" << endl;
	}
	return msg;
}

QString KIRCMessage::quote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\") );
	tmp.replace( QChar('\0'), QString::fromLatin1("\\0") );
	tmp.replace( QChar((uchar)0x01), QString::fromLatin1("\\1") );
	tmp.replace( QChar('\n'), QString::fromLatin1("\\n") );
	tmp.replace( QChar('\r'), QString::fromLatin1("\\r") );
//	tmp.replace( QChar(' '), QString::fromLatin1("\\@") );
	return tmp;
}

QString KIRCMessage::unquote(const QString &str)
{
	// FIXME: The unquote system is buggy. ex "\\return to daddy" gives "\015eturn to daddy"
	QString tmp = str;
	tmp.replace(QString::fromLatin1("\\\\"), QChar('\\'));
//	tmp.replace(QString::fromLatin1("\\@"), QChar(' '));
	tmp.replace(QString::fromLatin1("\\r"), QChar('\r'));
	tmp.replace(QString::fromLatin1("\\n"), QChar('\n'));
	tmp.replace(QString::fromLatin1("\\1"), QChar((uchar)0x01));
	tmp.replace(QString::fromLatin1("\\0"), QChar('\0'));
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
		args    = QStringList::split(QRegExp(" "), regexp.cap(3).stripWhiteSpace());
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

bool KIRCMessage::isNumericMessage() const
{
	QRegExp re(m_IRCNumericCommand);
	return re.exactMatch( m_command );
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
	kdDebug(14120)	<< "Raw:" << m_raw
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
