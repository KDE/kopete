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

#include <qdict.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qregexp.h>
#include <kbufferedio.h>

#include "kircentity.h" // Don't remove this prefix should/could(MUST?) be returned as a KIRCEntity

// Uncoment this if you want a really rfc compliant message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

class KIRC;

class KIRCMessage
{
friend class KIRC;
public:
	KIRCMessage();
	KIRCMessage(const KIRCMessage &obj);
	KIRCMessage(const KIRCMessage *obj);

	~KIRCMessage();

	static void writeRawMessage(KIRC *engine, const QTextCodec *codec, const QString &str);

	static void writeMessage(KIRC *engine, const QTextCodec *codec, const QString &str);

	static void writeMessage(KIRC *engine, const QTextCodec *codec,
		const QString &command, const QStringList &args, const QString &suffix);

	static void writeCtcpMessage(KIRC *engine, const QTextCodec *codec,
		const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpMessage, const QStringList &ctcpArgs = QStringList(), const QString &ctcpSuffix = QString::null );

	inline const QString nickFromPrefix() const
		{ return KIRCEntity::userNick(m_prefix); }

	QString toString() const;

	bool isNumeric() const;
	bool isValid() const;
	void dump() const;

	// The raw message
	inline const QCString &raw() const
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
	inline const QCString &ctcpRaw() const
		{ return m_ctcpRaw; }

	inline bool hasCtcpMessage() const
		{ return m_ctcpMessage!=0; }
	inline const class KIRCMessage &ctcpMessage() const
		{ return *m_ctcpMessage; }

	static KIRCMessage parse(KIRC *engine, const QTextCodec *codec, bool *parseSuccess=0);

private:
	/**
	 * Contains the low level dequoted message.
	 */
	QCString m_raw;

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
	QCString m_ctcpRaw;

	// low level quoting, message quoting
	static QString quote(const QString &str);
	static QCString unquote(const char* str);

	// ctcp level quoting
	static QString ctcpQuote(const QString &str);

	static bool extractCtcpCommand(QCString &str, QCString &ctcpline);
	static bool matchForIRCRegExp(const QCString &line, const QTextCodec *codec, KIRCMessage &message);

	class KIRCMessage *m_ctcpMessage;

	static QRegExp m_IRCCommand;
	static QRegExp m_IRCNumericCommand;
};

#endif // KIRCMESSAGE_H
