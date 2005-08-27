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

//#include <QList>
#include <QStringList>

// Uncoment this if you want a really rfc compliant message handling.
// This is due to some changes of the message encoding with 14 arguments.(not very frequent :)
// #define _IRC_STRICTNESS_

#ifndef QByteArrayList
#define QByteArrayList QList<QByteArray>
#endif

class QTextCodec;

namespace KIRC
{

class Engine;

class Message
{
public:
	static QString format(
		const QString &command,
		const QStringList &args = QStringList(),
		const QString &suffix = QString::null);

	// temporary hack
	static QString format(
		const QString &command,
		const QString &arg,
		const QString &suffix = QString::null)
	{
		return format(command, QStringList(arg), suffix);
	}

	static QByteArray format(
		const QByteArray &command,
		const QByteArrayList &args = QByteArrayList(),
		const QByteArray &suffix = QByteArray());

	// temporary hack
	static QByteArray format(
		const QByteArray &command,
		const QByteArray &arg,
		const QByteArray &suffix)
	{
		QByteArrayList args;
		args.append(arg);
		return format(command, args, suffix);
	}

	static QString formatCtcp(const QString &ctcpMessage);
	static QByteArray formatCtcp(const QByteArray &ctcpMessage);

	// low level quoting, message quoting
	static QString quote(const QString &str);
	static QByteArray quote(const QByteArray &str);

	static QByteArray unquote(const QByteArray &str);

	// ctcp level quoting
	static QString ctcpQuote(const QString &str);
	static QByteArray ctcpQuote(const QByteArray &str);

	static QByteArray ctcpUnquote(const QByteArray &str);

private:

	static QRegExp sm_IRCCommandType1;
#ifdef _IRC_STRICTNESS_
	static QRegExp sm_IRCCommandType2;
#endif // _IRC_STRICTNESS_

	static QRegExp sm_IRCNumericCommand;

public:
	Message(const QByteArray &message = QByteArray());
	Message(const KIRC::Message &obj);

	~Message();

	bool isValid() const;

	bool isNumeric() const;
	void dump() const;

//	KIRC::EntityPtr entityFromPrefix() const;
//	KIRC::EntityPtr entityFromArg(size_t i) const;

	QByteArray rawLine() const;
	QByteArray rawPrefix() const;
	QByteArray rawCommand() const;
	QByteArray rawArgs() const;
	QByteArrayList rawArgList() const;
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

	bool parse(const QByteArray &message);

	bool matchForIRCRegExp(QRegExp &regexp);

#ifndef _IRC_STRICTNESS_
	bool extractCtcpCommand();
#endif // _IRC_STRICTNESS_

	/**
	 * Set to true if the parse was successfull.
	 */
	bool m_valid;

	/**
	 * Contains the raw message line.
	 */
	QByteArray m_line;

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
	QByteArrayList m_argList;

	/**
	 * Contains the completely dequoted suffix.
	 */
	QByteArray m_suffix;

	class KIRC::Message *m_ctcpMessage;
};

}

#endif // KIRCMESSAGE_H
