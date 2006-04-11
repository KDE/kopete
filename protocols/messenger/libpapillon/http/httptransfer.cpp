/*
   httptransfer.h - HTTP transfer

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "httptransfer.h"

#include <QtNetwork/QHttpHeader>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

namespace Papillon
{

class HttpTransfer::Private
{
public:
	Private()
	 : httpHeader(0)
	{}

	~Private()
	{
		delete httpHeader;
	}

	HttpTransfer::HttpTransferType type;
	// We use base class as a pointer for polymorphist.
	QHttpHeader *httpHeader;
	QByteArray body;
};

HttpTransfer::HttpTransfer(HttpTransferType type)
 : d(new Private)
{
	d->type = type;
	switch(type)
	{
		case HttpRequest:
			d->httpHeader = new QHttpRequestHeader;
			break;
		case HttpResponse:
			d->httpHeader = new QHttpResponseHeader;
			break;
	}
}

HttpTransfer::~HttpTransfer()
{
	delete d;
}

HttpTransfer::HttpTransferType HttpTransfer::type() const
{
	return d->type;
}

bool HttpTransfer::isValid() const
{
	return d->httpHeader->isValid();
}

uint HttpTransfer::contentLength() const
{
	return d->httpHeader->contentLength();
}

QString HttpTransfer::contentType() const
{
	return d->httpHeader->contentType();
}

void HttpTransfer::setContentType(const QString &contentType)
{
	d->httpHeader->setContentType(contentType);
}

void HttpTransfer::setRequest(const QString &method, const QString &path, int majorVer, int minorVer)
{
	Q_ASSERT(d->type & HttpRequest);

	static_cast<QHttpRequestHeader*>(d->httpHeader)->setRequest(method, path, majorVer, minorVer);
}

void HttpTransfer::setHttpHeader(QHttpHeader *header)
{
	*d->httpHeader = *header;
}

bool HttpTransfer::hasContentLength() const
{
	return d->httpHeader->hasContentLength();
}

bool HttpTransfer::hasContentType() const
{
	return d->httpHeader->hasContentType();
}

bool HttpTransfer::hasKey(const QString &key) const
{
	return d->httpHeader->hasKey(key);
}

QString HttpTransfer::value(const QString & key) const
{
	return d->httpHeader->value(key);
}

QList<QPair<QString, QString> > HttpTransfer::values() const
{
	return d->httpHeader->values();
}

void HttpTransfer::setValue(const QString & key, const QString &value)
{
	d->httpHeader->setValue(key, value);
}

void HttpTransfer::setValues(const QList<QPair<QString, QString> > &values)
{
	d->httpHeader->setValues(values);
}

int HttpTransfer::statusCode() const
{
	Q_ASSERT(d->type & HttpResponse);

	return static_cast<QHttpResponseHeader*>(d->httpHeader)->statusCode();
}

QString HttpTransfer::method() const
{
	Q_ASSERT(d->type & HttpRequest);

	return static_cast<QHttpRequestHeader*>(d->httpHeader)->method();
}

QString HttpTransfer::path() const
{
	Q_ASSERT(d->type & HttpRequest);

	return static_cast<QHttpRequestHeader*>(d->httpHeader)->path();
}

QByteArray HttpTransfer::body() const
{
	return d->body;
}

void HttpTransfer::setBody(const QByteArray &body)
{
	d->body = body;
	d->httpHeader->setContentLength( body.size() );
}

QByteArray HttpTransfer::toRawCommand()
{
	QByteArray result;

	result += d->httpHeader->toString().toUtf8();
	result += QByteArray("\r\n");
	result += d->body;

	return result;
}


}
