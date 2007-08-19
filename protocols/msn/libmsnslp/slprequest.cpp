/*
    slprequest.cpp - Peer To Peer Session Layer Protocol Request class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "slprequest.h"
#include <qregexp.h>

namespace PeerToPeer
{

class SlpRequest::SlpRequestPrivate
{
	public:
		QString method;
		QString requestUri;

}; // SlpRequestPrivate

SlpRequest::SlpRequest(const QString& version) : SlpMessage(version), d(new SlpRequestPrivate())
{
}

SlpRequest::SlpRequest(const QString& method, const QString& requestUri, const QString& version) : SlpMessage(version), d(new SlpRequestPrivate())
{
	d->method = method;
	d->requestUri = requestUri;
}

SlpRequest::SlpRequest(const SlpRequest& other) : SlpMessage(other), d(new SlpRequestPrivate(*other.d))
{
}

SlpRequest & SlpRequest::operator=(const SlpRequest& other)
{
	SlpMessage::operator=(other);

	*d = *other.d;
	return *this;
}

SlpRequest::~SlpRequest()
{
	delete d;
}

const QString SlpRequest::method() const
{
	return d->method;
}

void SlpRequest::setMethod(const QString& method)
{
	d->method = method;
}

const QString SlpRequest::requestUri() const
{
	return d->requestUri;
}

void SlpRequest::setRequestUri(const QString& requestUri)
{
	d->requestUri = requestUri;
}

const QString SlpRequest::startLine() const
{
	return QString("%1 MSNMSGR:%2 MSNSLP/%3").arg(d->method, d->requestUri, version());
}

}
