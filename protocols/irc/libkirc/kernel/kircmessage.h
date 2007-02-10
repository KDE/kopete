/*
    kircmessage.h - IRC Client

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "kircglobal.h"

#include <QSharedDataPointer>
#include <QStringList>

class QTextCodec;

namespace KIrc
{

class Socket;

class KIRC_EXPORT Message
{
public:
	enum Direction
	{
		Undefined = 0,
		OutGoing  = 1, // From the client to the network
		InGoing	  = 2  // From the network to the client
	};

	enum Type
	{
		Unknown           = 0,
		Command           = 1, // Text command
		ClientServerReply = 2, // Numeric reply in the 001-099 range
		CommandReply      = 3, // Numeric reply in the 200-399 range
		ErrorReply        = 4, // Numeric reply in the 400-599 range
	};

public:
	Message();
	Message(const KIrc::Message &o);
	~Message();

	Message &operator = (const Message &o);

public:
	bool isValid() const;

	Socket *socket() const;
	void setSocket(Socket *socket);

	Direction direction() const;
	void setDirection(Message::Direction direction);

	QTextCodec *codec() const;
	void setCodec(QTextCodec *codec);

	QByteArray rawLine() const;
	void setLine(const QByteArray &);

	QByteArray rawPrefix() const;
	void setPrefix(const QByteArray &);

	QByteArray rawCommand() const;
	void setCommand(const QByteArray &);

	QList<QByteArray> rawArgs() const;
	void setArgs(const QList<QByteArray> &);

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

	QString suffix(QTextCodec *codec = 0) const;
	void setSuffix(const QString &, QTextCodec *codec = 0);

public:
	size_t argsSize() const;

	QByteArray rawArg(size_t i) const;
	void setArg(size_t i, const QByteArray &arg);

	QString arg(size_t i, QTextCodec *codec = 0) const;
	void setArg(size_t i, const QString &arg, QTextCodec *codec);

	void appendArgs(const QList<QByteArray> &args);
	void appendArgs(const QStringList &args, QTextCodec *codec = 0);

	void appendArg(const QByteArray &arg);
	void appendArg(const QString &arg, QTextCodec *codec = 0);

#if 0
	QByteArray rawArgs(size_t from, size_t to) const;
	QString args(size_t from, size_t to, QTextCodec *codec) const;
#endif
	Entity::Ptr entityFromPrefix() const;
	Entity::Ptr entityFromArg(size_t i) const;

	Entity::Ptr entityFromName(const QByteArray &name);

	Entity::List entitiesFromNames(const QList<QByteArray> &names);

	Entity::List entitiesFromNames(const QByteArray &names, char separator = ',');

	bool isNumericReply() const;

	Type type() const;

private:
	/**
	 * Check the given codec.
	 * @param codec the given codec to check. It can be a null pointer codec.
	 * @return the same codec as the entry if not null, else the engine default codec.
	 */
	QTextCodec *checkCodec(QTextCodec *codec) const;

	bool matchForIRCRegExp(QRegExp &regexp);

	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif

