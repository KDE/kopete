/*
    message.cpp - Peer To Peer Message class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "message.h"

namespace PeerToPeer
{

class Message::MessagePrivate
{
	public:
		QString body;
		QMap<QString, QVariant> context;
		QMap<QString, QVariant> headers;
		QString version;

};// MessagePrivate

Message::Message(const QString& version) : d(new MessagePrivate())
{
	d->version = version;
}

Message::Message(const Message& other) : d(new MessagePrivate())
{
	*this = other;
}

Message & Message::operator=(const Message& other)
{
	d->body = other.body();
	d->context = other.context();
	d->headers = other.headers();
	d->version = other.version();
	return *this;
}

Message::~Message()
{
	delete d;
}

const QString Message::body() const
{
	return d->body;
}

const Q_INT32 Message::contentLength() const
{
	return d->headers["Content-Length"].toInt();
}

const QString Message::contentType() const
{
	return d->headers["Content-Type"].toString();
}

QMap<QString, QVariant> & Message::context()
{
	return d->context;
}

const QMap<QString, QVariant> & Message::context() const
{
	return d->context;
}

void Message::copyHeadersFrom(const QMap<QString, QVariant> & collection)
{
	QMap<QString, QVariant>::ConstIterator i;
	for(i = collection.begin(); i != collection.end(); ++i)
	{
		d->headers.insert(i.key(), i.data());
	}
}

const QString Message::from() const
{
	return d->headers["From"].toString();
}

QMap<QString, QVariant> & Message::headers()
{
	return d->headers;
}

const QMap<QString, QVariant> & Message::headers() const
{
	return d->headers;
}

const Q_INT32 Message::identifier() const
{
	return d->context["identifier"].toInt();
}

const Q_INT32 Message::relatesTo() const
{
	return d->context["relatesTo"].toInt();
}

void Message::setBody(const QString& body)
{
	d->body = body;
}

void Message::setIdentifier(const Q_INT32 identifier)
{
	d->context["identifier"] = QVariant(identifier);
}

void Message::setRelatesTo(const Q_INT32 relatesTo)
{
	d->context["relatesTo"] = QVariant(relatesTo);
}

const QString Message::to() const
{
	return d->headers["To"].toString();
}

const QString Message::version() const
{
	return d->version;
}

}
