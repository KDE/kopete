/*
   httptransfer.h - HTTP transfer

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Http/Transfer"

#include <QtNetwork/QHttpHeader>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>

namespace Papillon
{

class HttpTransfer::Private
{
public:
	Private()
	 : type(HttpTransfer::HttpRequest), httpHeader(0)
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
	Q_ASSERT(type() == HttpTransfer::HttpRequest);

	static_cast<QHttpRequestHeader*>(d->httpHeader)->setRequest(method, path, majorVer, minorVer);
}

void HttpTransfer::setHttpHeader(const QHttpRequestHeader &header)
{
	Q_ASSERT(type() == HttpTransfer::HttpRequest);

	// Operator= is not virtual so we need to cast to derivate here.
	QHttpRequestHeader *requestHeader = static_cast<QHttpRequestHeader*>(d->httpHeader);
	requestHeader->operator=(header);
}

void HttpTransfer::setHttpHeader(const QHttpResponseHeader &header)
{
	Q_ASSERT(type() == HttpTransfer::HttpResponse);

	// Operator= is not virtual so we need to cast to derivate here.
	QHttpResponseHeader *responseHeader = static_cast<QHttpResponseHeader*>(d->httpHeader);
	responseHeader->operator=(header);
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
	return d->httpHeader->hasKey(key.toLower());
}

QString HttpTransfer::value(const QString & key) const
{
	return d->httpHeader->value(key.toLower());
}

QList<QPair<QString, QString> > HttpTransfer::values() const
{
	return d->httpHeader->values();
}

void HttpTransfer::setValue(const QString & key, const QString &value)
{
	d->httpHeader->setValue(key.toLower(), value);
}

void HttpTransfer::setValues(const QList<QPair<QString, QString> > &values)
{
	d->httpHeader->setValues(values);
}

int HttpTransfer::statusCode() const
{
	Q_ASSERT(d->type == HttpResponse);

	return static_cast<QHttpResponseHeader*>(d->httpHeader)->statusCode();
}

QString HttpTransfer::method() const
{
	Q_ASSERT(d->type == HttpRequest);

	return static_cast<QHttpRequestHeader*>(d->httpHeader)->method();
}

QString HttpTransfer::path() const
{
	Q_ASSERT(d->type == HttpRequest);

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
	result += d->body;

	return result;
}


}
