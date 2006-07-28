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

//#include <QList> // From QStringList
#include <QSharedDataPointer>
#include <QStringList>

typedef QList<QByteArray> QByteArrayList;

class QTextCodec;

namespace KIRC
{

class Socket;

class Message
{
public:
	enum Direction
	{
		Unknown  = 0,
		OutGoing = 1, // From the client to the network
		InGoing	 = 1  // From the network to the client
	};

public:
	Message();
	Message(const QByteArray &rawLine, KIRC::Message::Direction direction);
	Message(const KIRC::Message &o);
	~Message();

	KIRC::Message &operator = (const KIRC::Message &o);

public:
	KIRC::Socket *socket() const;
	void setSocket(Socket *);

	Direction direction() const;
	void setDirection(KIRC::Message::Direction direction);

	QTextCodec *codec() const;
	void setCodec(QTextCodec *codec);

	QByteArray rawLine() const;
	void setLine(const QByteArray &);

	QByteArray rawPrefix() const;
	void setPrefix(const QByteArray &);

	QByteArray rawCommand() const;
	void setCommand(const QByteArray &);

	QByteArrayList rawArgs() const;
	void setArgs(const QByteArrayList &);

	QByteArray rawSuffix() const;
	void setSuffix(const QByteArray &);

//	QString line(QTextCodec *codec = 0) const;
//	void setLine(const QString &, QTextCodec *codec = 0);

	QString prefix(QTextCodec *codec = 0) const;
	void setPrefix(const QString &, QTextCodec *codec = 0);

	QString command(QTextCodec *codec = 0) const;
	void setCommand(const QString &, QTextCodec *codec = 0);

	QStringList args(QTextCodec *codec = 0) const;
	void setArgs(const QStringList &, QTextCodec *codec = 0);
	inline void setArgs(const QString &arg, QTextCodec *codec = 0)
	{ setArgs(QStringList(arg), codec);}

	QString suffix(QTextCodec *codec = 0) const;
	void setSuffix(const QString &, QTextCodec *codec = 0);

#ifndef KIRC_STRICT
	bool hasCtcpMessage() const;
	KIRC::Message ctcpMessage() const;
#endif


public:
	bool isValid() const;

	bool isNumeric() const;
	void dump() const;

//	KIRC::Entity::Ptr entityFromPrefix() const;
//	KIRC::Entity::Ptr entityFromArg(size_t i) const;

	size_t argsSize() const;
	QByteArray rawArg(size_t i) const;
	QString arg(size_t i, QTextCodec *codec = 0) const;

private:
	/**
	 * Check the given codec.
	 * @param codec the given codec to check. It can be a null pointer codec.
	 * @return the same codec as the entry if not null, else the engine default codec.
	 */
	QTextCodec *checkCodec(QTextCodec *codec) const;

	bool matchForIRCRegExp(QRegExp &regexp);

#ifndef _IRC_STRICTNESS_
	bool extractCtcpCommand();
#endif // _IRC_STRICTNESS_

	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif // KIRCMESSAGE_H
