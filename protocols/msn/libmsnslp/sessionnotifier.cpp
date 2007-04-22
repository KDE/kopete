/*
    sessionnotifier.cpp - Peer To Peer Session Notifier

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "sessionnotifier.h"

namespace PeerToPeer
{

class SessionNotifier::SessionNotifierPrivate
{
	public:
		Q_UINT32 session;
		Type type;
};

SessionNotifier::SessionNotifier(const Q_UINT32 session, const Type type, QObject *parent) : QObject(parent), d(new SessionNotifierPrivate())
{
	d->session = session;
	d->type = type;
}

SessionNotifier::~SessionNotifier()
{
	delete d;
}

const Q_UINT32 SessionNotifier::session() const
{
	return d->session;
}

void SessionNotifier::setType(const Type type)
{
	d->type = type;
}

const SessionNotifier::Type SessionNotifier::type() const
{
	return d->type;
}

void SessionNotifier::fireDataReceived(const QByteArray& data, bool lastChunk)
{
	emit dataReceived(data, lastChunk);
}

void SessionNotifier::fireMessageAcknowledged(const Q_INT32 id)
{
	emit messageAcknowledged(id);
}

void SessionNotifier::fireMessageReceived(const QByteArray& message, const Q_INT32 id, const Q_INT32 correlationId)
{
	emit messageReceived(message, id, correlationId);
}

}

#include "sessionnotifier.moc"
