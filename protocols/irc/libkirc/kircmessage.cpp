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
 /*
KIRCRegExp::KIRCRegExp( const char* regex )
{
	const char *error;
	int erroffset;
	re = pcre_compile(
		regex,
		PCRE_NO_UTF8_CHECK,
		&error,
		&erroffset,
		NULL
	);

	pe = pcre_study(
		re,
		0,
		&error
	);
}

KIRCRegExp::~KIRCRegExp()
{
	pcre_free( pe );
	pcre_free( re );
}

bool KIRCRegExp::exactMatch( QCString str )
{
	subject = str;
	caps = pcre_exec(
		re,
		pe,
		str,
		subject.length(),
		0,
		0,
		ovector,
		30
	);

	if( caps > 0 )
	{
	        return true;
	}
	else
	{
		return false;
	}
}

QCString KIRCRegExp::cap( int idx )
{
	if( caps >= idx )
	{
		char buffer[4096];
		pcre_copy_substring( subject, ovector, caps, idx, buffer, 4096 );
		return buffer;
	}
	else
	{
		return "";
	}
}   */

#ifndef _IRC_STRICTNESS_
KRegExp KIRCMessage::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*)*) ?(?: :(.*))?$");
	// Extra end arg space check -------------------------^
#else // _IRC_STRICTNESS_
KRegExp KIRCMessage::m_IRCCommandType1(
	"^(?::([^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){0,13})(?: :(.*))?$");
KRegExp KIRCMessage::m_IRCCommandType2(
	"^(?::[[^ ]+) )?([A-Za-z]+|\\d{3,3})((?: [^ :][^ ]*){14,14})(?: (.*))?$");
#endif // _IRC_STRICTNESS_

KRegExp KIRCMessage::m_IRCNumericCommand("^\\d{3,3}$");

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
		msg += QChar(' ') + args.join( QChar(' ') ).stripWhiteSpace(); // some extra check should be done here

	if (!suffix.isNull())
		msg += QString::fromLatin1(" :") + suffix;

	writeMessage(engine, codec, msg);
}

void KIRCMessage::writeCtcpMessage(KIRC *engine, const QTextCodec *codec,
		const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix )
{
	QString ctcpMsg = ctcpCommand;

	if (!ctcpArgs.isEmpty())
		ctcpMsg += QChar(' ') + ctcpArgs.join( QChar(' ') ).stripWhiteSpace(); // some extra check should be done here

	if (!ctcpSuffix.isNull())
		ctcpMsg += QString::fromLatin1(" :") + ctcpSuffix;

	writeMessage(engine, codec, command, to, suffix + QChar(0x01) + ctcpQuote(ctcpMsg) + QChar(0x01));
}

KIRCMessage KIRCMessage::parse(KIRC *engine, const QTextCodec *codec, bool *parseSuccess)
{
	if(parseSuccess)
		*parseSuccess=false;

	if( engine->socket()->canReadLine() )
	{
		QCString raw( engine->socket()->bytesAvailable()+1 );
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
QCString KIRCMessage::unquote(const char* str)
{
	if( str )
	{
		QCString tmp = str;

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

		tmp.replace("\\\\", "\\");
		tmp.replace("\\1", "\1" );

		return tmp;
	}
	else
	{
		return "";
	}
}

QString KIRCMessage::ctcpQuote(const QString &str)
{
	QString tmp = str;
	tmp.replace( QChar('\\'), QString::fromLatin1("\\\\"));
	tmp.replace( (char)1, QString::fromLatin1("\\1"));
	return tmp;
}

bool KIRCMessage::matchForIRCRegExp(const QCString &line, const QTextCodec *codec, KIRCMessage &message)
{
	if(matchForIRCRegExp(m_IRCCommandType1, codec, line, message))
		return true;
#ifdef _IRC_STRICTNESS_
	if(!matchForIRCRegExp(m_IRCCommandType2, codec, line, message)
		return true;
#endif // _IRC_STRICTNESS_
	return false;
}

bool KIRCMessage::matchForIRCRegExp(KRegExp &regexp, const QTextCodec *codec, const QCString &line, KIRCMessage &msg )
{
	if(regexp.match(line))
	{
		msg.m_raw = line;
		msg.m_prefix  = QString::fromLatin1( unquote( regexp.group(1) ) );
		msg.m_command = QString::fromLatin1( unquote( regexp.group(2) ) );
		msg.m_args = QStringList::split(' ', QString::fromLatin1( regexp.group(3) ) );

		QCString suffix = unquote( regexp.group(4) );
		if( !suffix.isNull() && suffix.length() > 0 )
		{
			if( extractCtcpCommand( suffix, msg.m_ctcpRaw ) )
			{
				msg.m_ctcpMessage = new KIRCMessage();
				msg.m_ctcpMessage->m_raw = msg.m_ctcpRaw;

				int space = msg.m_ctcpRaw.find(' ');
				if( !matchForIRCRegExp(msg.m_ctcpMessage->m_raw, codec, *msg.m_ctcpMessage) )
				{
					if( space > 0 )
					{
						msg.m_ctcpMessage->m_command = QString::fromLatin1(
							msg.m_ctcpRaw.mid(0, space).upper()
						);
					}
					else
						msg.m_ctcpMessage->m_command = QString::null;
				}

				if( space > 0 )
					msg.m_ctcpMessage->m_ctcpRaw = msg.m_ctcpRaw.mid( space );

				msg.m_suffix = QString::null;
			}
			else
			{
				msg.m_suffix = KopeteMessage::decodeString( KSParser::parse( unquote( suffix ) ), codec );
			}
		}
		else
		{
			msg.m_suffix = QString::null;
		}

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
	return m_IRCNumericCommand.match( m_command.latin1() );
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
bool KIRCMessage::extractCtcpCommand(QCString &str, QCString &ctcpline)
{
	// This could also be done using a regexp like: "^([^\\x01])\\x01([^\\x01])\\x01$"
	// (make less code to test)
	uint len = str.length();
	if( str[0] == 1 && str[ len - 1 ] == 1 )
	{
		ctcpline = str.mid( 1, len - 2 );
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
//			<< "Args:" << m_args << endl   //this does not compile on KDE 3.1
			<< "Suffix:" << m_suffix << endl
			<< "CtcpRaw:" << m_ctcpRaw << endl;
	if(m_ctcpMessage)
	{
		kdDebug(14120) << "Contains CTCP Message:" << endl;
		m_ctcpMessage->dump();
	}
}
