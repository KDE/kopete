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
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <kbufferedio.h>

// Uncoment this if you want a really rfc compliant message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

class KIRCMessage
{
public:
	KIRCMessage();
	KIRCMessage(const KIRCMessage &obj);
	KIRCMessage(const KIRCMessage *obj);

	~KIRCMessage();

	static KIRCMessage writeRawMessage(QIODevice *dev, const QString &str, const QTextCodec *codec );
	static KIRCMessage writeMessage(QIODevice *dev, const QString &str, const QTextCodec *codec );

	static KIRCMessage writeMessage(QIODevice *dev,
			const QString &command, const QString &arg, const QString &suffix,
			const QTextCodec *code0);
	static KIRCMessage writeMessage(QIODevice *dev,
			const QString &command, const QStringList &args, const QString &suffix,
			const QTextCodec *codec);

	static KIRCMessage writeCtcpMessage(QIODevice *dev,
			const QString &command, const QString &to /*prefix*/, const QString &suffix,
			const QString &ctcpMessage,
			const QTextCodec *codec);
	static KIRCMessage writeCtcpMessage(QIODevice *dev,
			const QString &command, const QString &to /*prefix*/, const QString &suffix,
			const QString &ctcpCommand, const QString &ctcpArg, const QString &ctcpSuffix,
			const QTextCodec *codec);
	static KIRCMessage writeCtcpMessage(QIODevice *dev,
			const QString &command, const QString &to /*prefix*/, const QString &suffix,
			const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix,
			const QTextCodec *codec);

	// FIXME: short term solution move me to the the KIRCUser class
	inline static QString getNickFromPrefix(const QString &prefix)
		{ return prefix.section('!', 0, 0); }

	static KIRCMessage parse(KBufferedIO *dev, bool *parseSuccess, const QTextCodec *codec );

	QString toString() const;

	bool isNumericMessage() const;
	bool isValid() const;
	void dump() const;

	// The raw message
	inline const QCString &raw() const
		{ return m_raw; }

	inline const QString &prefix() const
		{ return m_prefix; }
	inline const QString &command() const
		{ return m_command; }
	inline const QStringList &args() const
		{ return m_args; }
	inline const QString &suffix() const
		{ return m_suffix; }
	inline const QString &ctcpRaw() const
		{ return m_ctcpRaw; }

	inline bool hasCtcpMessage() const
		{ return m_ctcpMessage!=0; }
	inline const class KIRCMessage &ctcpMessage() const
		{ return *m_ctcpMessage; }

	static KIRCMessage parse(const QString &line, bool *parseSuccess=0);

protected:
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
	QString m_ctcpRaw;

	// low level quoting, message quoting
	static QString quote(const QString &str);
	static QString unquote(const QString &str);

	// ctcp level quoting
	static QString ctcpQuote(const QString &str);
	static QString ctcpUnquote(const QString &str);

	static bool extractCtcpCommand(QString &str, QString &ctcpline);
	static bool matchForIRCRegExp(const QString &line, KIRCMessage &message);
	static bool matchForIRCRegExp(QRegExp &regexp, const QString &line,
	QString &prefix, QString &command, QStringList &args, QString &suffix);

	class KIRCMessage *m_ctcpMessage;

	static const QRegExp m_IRCCommandType1;
#ifdef _IRC_STRICTNESS_
	static const QRegExp m_IRCCommandType2;
#endif // _IRC_STRICTNESS_

	static const QRegExp m_IRCNumericCommand;
};

#endif // KIRCMESSAGE_H
