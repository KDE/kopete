/*
    kircmessage.h - IRC Client

    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2003-2005 by the Kopete developers <kopete-devel@kde.org>

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

#include <qcstring.h>
#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <qvaluelist.h>

// Uncoment this if you want a really rfc compliant message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

namespace KIRC
{

class Engine;

class Message
{
public:
	static QString Message::format(const QString &command, const QStringList &args, const QString &suffix);

	// low level quoting, message quoting
	static QString quote(const QString &str);
	static QString unquote(const QString &str);

	// ctcp level quoting
	static QString ctcpQuote(const QString &str);
	static QString ctcpUnquote(const QString &str);

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

	static KIRC::Message parse(KIRC::Engine *engine, bool *parseSuccess=0);

private:
	static bool matchForIRCRegExp(KIRC::Message &message);
	static bool matchForIRCRegExp(QRegExp &regexp, KIRC::Message &message);

	static bool extractCtcpCommand(Message &msg);

	static QRegExp sm_IRCCommandType1;
#ifdef _IRC_STRICTNESS_
	static QRegExp sm_IRCCommandType2;
#endif // _IRC_STRICTNESS_

	static QRegExp sm_IRCNumericCommand;

public:
	Message(KIRC::Engine *);
	Message(const KIRC::Message &obj);
	Message(const KIRC::Message *obj);

	~Message();

	bool isNumeric() const;
	bool isValid() const;
	void dump() const;

	KIRC::EntityPtr entityFromPrefix() const;
	KIRC::EntityPtr entityFromArg(size_t i) const;

	QByteArray rawLine() const;
	QByteArray rawPrefix() const;
	QByteArray rawCommand() const;
	QByteArray rawArgs() const;
	QValueList<QByteArray> rawArgList() const;
	QByteArray rawArg(size_t i) const;
	QByteArray rawSuffix() const;

//	QString line(QTextCodec *codec = 0) const;
	QString prefix(QTextCodec *codec = 0) const;
	QString command(QTextCodec *codec = 0) const;
	QString args(QTextCodec *codec = 0) const;
	QStringList argList(QTextCodec *codec = 0) const;
	QString arg(size_t i, QTextCodec *codec = 0) const;
	QString suffix(QTextCodec *codec = 0) const;

	size_t argsSize() const;

	inline bool hasCtcpMessage() const
		{ return m_ctcpMessage!=0; }
	inline class KIRC::Message &ctcpMessage() const
		{ return *m_ctcpMessage; }

private:
	/**
	 * Check the given codec.
	 * @param codec the given codec to check. It can be a null pointer codec.
	 * @return the same codec as the entry if not null, else the engine default codec.
	 */
	QTextCodec *checkCodec(QTextCodec *codec) const;

	KIRC::Engine *m_engine;

	/**
	 * Contains the raw message line.
	 */
	QByteArray m_raw;

	/**
	 * Contains the raw prefix.
	 */
	QByteArray m_prefix;

	/**
	 * Contains the raw command.
	 */
	QByteArray m_command;

	/**
	 * Contains the raw args in a single line.
	 */
	QByteArray m_args;

	/**
	 * Contains the raw args plitted in a list.
	 */
	QValueList<QByteArray> m_argList;

	/**
	 * Contains the completely dequoted suffix.
	 */
	QByteArray m_suffix;

	class KIRC::Message *m_ctcpMessage;
};

}

#endif // KIRCMESSAGE_H
