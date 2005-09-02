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

#include "kdemacros.h"

//#include <QList>
#include <QObject>
#include <QSharedDataPointer>
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
class MessagePrivate;

class Message
	: public QObject
{
	Q_OBJECT

	Q_PROPERTY(QByteArray rawLine READ rawLine)
	Q_PROPERTY(QByteArray rawPrefix READ rawPrefix WRITE setPrefix)
	Q_PROPERTY(QByteArray rawCommand READ rawCommand WRITE setCommand)
//	Q_PROPERTY(QByteArrayList rawArgList )
	Q_PROPERTY(QByteArray rawSuffix READ rawSuffix WRITE setSuffix)

//	Q_PROPERTY(QString line READ rawLine)
	Q_PROPERTY(QString prefix READ prefix WRITE setPrefix)
	Q_PROPERTY(QString command READ command WRITE setCommand)
//	Q_PROPERTY(QStringList argList READ argList)
	Q_PROPERTY(QString suffix READ suffix WRITE setSuffix)

public:
	static KIRC::Message parse(const QByteArray &message);

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

	static QByteArray formatCtcp(const QByteArray &ctcpMessage);

	// low level quoting, message quoting
	static QByteArray quote(const QByteArray &str);
	static QByteArray unquote(const QByteArray &str);

	// ctcp level quoting
	static QByteArray ctcpQuote(const QByteArray &str);
	static QByteArray ctcpUnquote(const QByteArray &str);

private:

	static QRegExp sm_IRCCommandType1;
#ifdef _IRC_STRICTNESS_
	static QRegExp sm_IRCCommandType2;
#endif // _IRC_STRICTNESS_

	static QRegExp sm_IRCNumericCommand;

public:
	Message();
	Message(const KIRC::Message &o);

	Message &operator = (const KIRC::Message &o);

public: // Properties read accessors
	QByteArray rawLine() const;
	QByteArray rawPrefix() const;
	QByteArray rawCommand() const;
	QByteArray rawArgs() const;
	QByteArrayList rawArgList() const;
	QByteArray rawSuffix() const;

//	QString line(QTextCodec *codec = 0) const;
	QString prefix(QTextCodec *codec = 0) const;
	QString command(QTextCodec *codec = 0) const;
	QString args(QTextCodec *codec = 0) const;
	QStringList argList(QTextCodec *codec = 0) const;
	QString suffix(QTextCodec *codec = 0) const;

public slots: // Properties write accessors
//	KIRC::Message &setLine(const QByteArray &);
	KIRC::Message &setPrefix(const QByteArray &);
	KIRC::Message &setCommand(const QByteArray &);
	KIRC::Message &setArgs(const QByteArray &);
	KIRC::Message &setArgList(const QByteArrayList &);
	KIRC::Message &setSuffix(const QByteArray &);

//	KIRC::Message &setLine(const QString &, QTextCodec *codec = 0);
	KIRC::Message &setPrefix(const QString &, QTextCodec *codec = 0);
	KIRC::Message &setCommand(const QString &, QTextCodec *codec = 0);
	KIRC::Message &setArgs(const QString &, QTextCodec *codec = 0);
	KIRC::Message &setArgList(const QStringList &, QTextCodec *codec = 0);
	KIRC::Message &setSuffix(const QString &, QTextCodec *codec = 0);
public:
	bool isValid() const;

	bool isNumeric() const;
	void dump() const;

//	KIRC::EntityPtr entityFromPrefix() const;
//	KIRC::EntityPtr entityFromArg(size_t i) const;

	size_t argsSize() const;
	QByteArray rawArg(size_t i) const;
	QString arg(size_t i, QTextCodec *codec = 0) const;

	bool hasCtcpMessage() const;
	KIRC::Message ctcpMessage() const;

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

	QSharedDataPointer<KIRC::MessagePrivate> d;
};

}

#endif // KIRCMESSAGE_H
