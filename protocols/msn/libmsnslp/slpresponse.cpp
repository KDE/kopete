/*
    slpresponse.cpp - Peer To Peer Session Layer Protocol Response class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "slpresponse.h"
#include <qregexp.h>

namespace PeerToPeer
{

class SlpResponse::SlpResponsePrivate
{
	public:
		SlpResponsePrivate() : statusCode(0) {}
		Q_INT32 statusCode;
		QString statusDescription;

}; // SlpResponsePrivate

SlpResponse::SlpResponse(const QString& version) : SlpMessage(version), d(new SlpResponsePrivate())
{
}

SlpResponse::SlpResponse(Q_INT32 statusCode, const QString& statusDescription, const QString& version) : SlpMessage(version), d(new SlpResponsePrivate())
{
	d->statusCode = statusCode;
	d->statusDescription = statusDescription;
}

SlpResponse::SlpResponse(const SlpResponse& other) : SlpMessage(other), d(new SlpResponsePrivate())
{
	*this = other;
}

SlpResponse & SlpResponse::operator=(const SlpResponse& other)
{
	SlpMessage::operator=(other);

	d->statusCode = other.statusCode();
	d->statusDescription = other.statusDescription();
	return *this;
}

SlpResponse::~SlpResponse()
{
	delete d;
}

const Q_INT32 SlpResponse::statusCode() const
{
	return d->statusCode;
}

void SlpResponse::setStatusCode(Q_INT32 statusCode)
{
	d->statusCode = statusCode;
}

const QString SlpResponse::statusDescription() const
{
	return d->statusDescription;
}

void SlpResponse::setStatusDescription(const QString& statusDescription)
{
	d->statusDescription = statusDescription;
}

const QString SlpResponse::startLine() const
{
	return QString("MSNSLP/%1 %2 %3").arg(version()).arg(d->statusCode).arg(d->statusDescription);
}

}
