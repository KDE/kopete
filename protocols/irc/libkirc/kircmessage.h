/*
    kircmessage.h - IRC Client

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

#ifndef KIRCMESSAGE_H
#define KIRCMESSAGE_H

#include "kircentity.h"

#include <kbufferedio.h>

#include <q3dict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>

#include <kopetemessage.h>

// Uncoment this if you want a really rfc compliant message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

namespace KIRC
{

class Engine;

class Message
	: public QObject
{
	Q_OBJECT

public:
	static void writeRawMessage(KIRC::Engine *engine, const QTextCodec *codec, const QString &str);

	static void writeMessage(KIRC::Engine *engine, const QTextCodec *codec, const QString &str);

	static void writeMessage(KIRC::Engine *engine, const QTextCodec *codec,
		const QString &command, const QStringList &args, const QString &suffix);

	static void writeCtcpMessage(KIRC::Engine *engine, const QTextCodec *codec,
		const QString &command, const QString &to,
		const QString &ctcpMessage);

	static void writeCtcpMessage(KIRC::Engine *engine, const QTextCodec *codec,
		const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs = QStringList(), const QString &ctcpSuffix = QString::null );

	Message();
	Message(const KIRC::Message &obj);
	Message(const KIRC::Message *obj);

	~Message();

	inline const QString nickFromPrefix() const
	{ return Kopete::Message::unescape(KIRC::Entity::userNick(m_prefix)); }

	QString toString() const;

	bool isNumeric() const;
	bool isValid() const;
	void dump() const;
	void decodeAgain( const QTextCodec *codec );

	inline const Q3CString &raw() const
		{ return m_raw; }
	inline const QString &prefix() const
		{ return m_prefix; }
	inline const QString &command() const
		{ return m_command; }
	inline size_t argsSize() const
		{ return m_args.size(); }
	inline const QString &arg(size_t i) const
		{ return m_args[i]; }
	inline const QStringList &args() const
		{ return m_args; }
	inline const QString &suffix() const
		{ return m_suffix; }
	inline const QString &ctcpRaw() const
		{ return m_ctcpRaw; }

	inline bool hasCtcpMessage() const
		{ return m_ctcpMessage!=0; }
	inline class KIRC::Message &ctcpMessage() const
		{ return *m_ctcpMessage; }

	static KIRC::Message parse(KIRC::Engine *engine, const QTextCodec *codec, bool *parseSuccess=0);

private:
	/**
	 * Contains the low level dequoted message.
	 */
	Q3CString m_raw;

	/**
	 * Contains the completely dequoted prefix.
	 */
	QString m_prefix;
	/**
	 * Contains the completely dequoted command.
	 */
	QString m_command;
	/**
	 * Contains the completely dequoted args.
	 */
	QStringList m_args;
	/**
	 * Contains the completely dequoted suffix.
	 */
	QString m_suffix;

	/**
	 * If it is a message contains the completely dequoted rawCtcpLine.
	 * If it is a ctcp message contains the completely dequoted rawCtcpArgsLine.
	 */
	QString m_ctcpRaw;

	// low level quoting, message quoting
	static QString quote(const QString &str);
	static QString unquote(const QString &str);

	// ctcp level quoting
	static QString ctcpQuote(const QString &str);
	static QString ctcpUnquote(const QString &str);

	static bool extractCtcpCommand(Q3CString &str, Q3CString &ctcpline);

	static bool matchForIRCRegExp(const Q3CString &line, const QTextCodec *codec, KIRC::Message &message);
	static bool matchForIRCRegExp(QRegExp &regexp, const QTextCodec *codec, const Q3CString &line, KIRC::Message &message);

	class KIRC::Message *m_ctcpMessage;

	static QRegExp m_IRCCommandType1;
  #ifdef _IRC_STRICTNESS_
	static QRegExp m_IRCCommandType2;
  #endif // _IRC_STRICTNESS_

	static QRegExp m_IRCNumericCommand;
};

}

#endif // KIRCMESSAGE_H
