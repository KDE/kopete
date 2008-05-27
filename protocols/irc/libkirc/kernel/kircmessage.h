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

#ifndef KIRC_MESSAGE_H
#define KIRC_MESSAGE_H

#include "kircglobal.h"

#include "kircentity.h"

#include <QtCore/QSharedDataPointer>
#include <QtCore/QStringList>

class QTextCodec;

namespace KIrc
{

class Context;
class EntityManager;

class MessagePrivate;

/*
static inline
_SetEnv setprefix(const QByteArray &prefix)
{
	return setenv("", prefix);
}

static inline
_SetEnv setsuffix(const QByteArray &suffix)
{
	return setenv("", suffix);
}
*/
class KIRC_EXPORT Message
{
public:
	static QByteArray quote(const QByteArray &buffer);
	static QByteArray unquote(const QByteArray &buffer);

	static QByteArray quoteCtcp(const QByteArray &buffer);
	static QByteArray unquoteCtcp(const QByteArray &buffer);

public:
	Message();
	Message(const QByteArray &prefix,
		const QList<QByteArray> &args,
		const QByteArray &suffix);
	Message(const KIrc::Message &o);
	~Message();

	Message &operator = (const Message &o);

public:
	static Message fromLine(const QByteArray &line, bool *ok = 0);
	QByteArray toLine() const;
//	QString toLine(QTextCodec *codec) const;

	QByteArray prefix() const;
//	QString prefix(QTextCodec *codec) const;

	KIrc::Message &operator << (const QByteArray &arg);
	KIrc::Message &operator << (const KIrc::OptArg &arg);
	QList<QByteArray> args() const;
//	QStringList args(QTextCodec *codec) const;

	QByteArray argAt(int i) const;
//	QString argAt(int i, QTextCodec *codec) const;

	QByteArray suffix() const;
//	QString suffix(QTextCodec *codec) const;

public:
	bool isNumericReply() const;

	QSharedDataPointer<KIrc::MessagePrivate> d;
};

}

#endif
