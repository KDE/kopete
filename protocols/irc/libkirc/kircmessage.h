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

#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <kbufferedio.h>

// Uncoment this if you want a really rfc complient message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

class KIRCMessage
{
protected:
	KIRCMessage()
			: m_ctcpMessage(0)
		{}
public:
	KIRCMessage(const KIRCMessage &obj);
	KIRCMessage(const KIRCMessage *obj);

	~KIRCMessage();

public:
	static KIRCMessage writeString(QIODevice *dev, const QString &str, QTextCodec *codec=0);
	static KIRCMessage writeMessage(QIODevice *dev,
			const QString &command, const QStringList &args, const QString &suffix,
			QTextCodec *codec=0);
	static KIRCMessage writeMessage(QIODevice *dev,
			const QString &command, const QString &arg, const QString &suffix,
			QTextCodec *codec=0);
	static KIRCMessage writeCtcpMessage(QIODevice *dev,
			const QString &command, const QString &to /*prefix*/, const QString &suffix,
			const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix,
			QTextCodec *codec=0);

	// FIXME: short term solution move me to the the KIRCUser class
	inline static QString getNickFromPrefix(const QString &prefix)
		{ return prefix.section('!', 0, 0); }

	static KIRCMessage parse(KBufferedIO *dev, bool *parseSuccess=0, QTextCodec *codec=0);
protected:
	static KIRCMessage parse(const QString &line, bool *parseSuccess=0);

	static QString quote(const QString &str);
	static QString unquote(const QString &str);

protected:
	static bool extractCtcpCommand(QString &str, QString &ctcpline);
	static bool matchForIRCRegExp(const QString &line, KIRCMessage &message);
	static bool matchForIRCRegExp(QRegExp &regexp, const QString &line,
		QString &prefix, QString &command, QStringList &args, QString &suffix);

public:
	QString toString() const;

	bool isNumericMessage() const;
	bool isValid() const;
	void dump() const;

public:
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

protected:
	QCString m_raw;

	QString m_prefix;
	QString m_command;
	QStringList m_args;
	QString m_suffix;

	// If it is a message contains the rawCtcpLine
	// If it is a ctcp message contains the rawCtcpArgsLine
	QString m_ctcpRaw;

	class KIRCMessage *m_ctcpMessage;

protected:
	static const QRegExp m_IRCCommandType1;
#ifdef _IRC_STRICTNESS_
	static const QRegExp m_IRCCommandType2;
#endif // _IRC_STRICTNESS_

	static const QRegExp m_IRCNumericCommand;
};

#endif // KIRCMESSAGE_H
