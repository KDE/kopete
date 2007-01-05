/*
    session.cpp - Peer To Peer Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "session.h"
#include <kdebug.h>
#include <qfile.h>

namespace PeerToPeer
{

class Session::SessionPrivate
{
	public:
		SessionPrivate() : file(0l), transportId(0), state(Session::Created) {}

		Direction direction;
		QFile *file;
		Q_UINT32 identifier;
		Q_UINT32 transportId;
		SessionState state;
};

Session::Session(const Q_UINT32 identifier, Direction direction, QObject *parent) : QObject(parent), d(new SessionPrivate())
{
	d->identifier = identifier;
	d->direction = direction;
}

Session::~Session()
{
	if (d->file != 0l)
	{
		d->file->close();
		delete d->file;
		d->file = 0l;
	}

	delete d;
	d = 0l;
}

void Session::start()
{
	d->state = Session::Established;
	onStart();
}

Session::Direction Session::direction() const
{
	return d->direction;
}

void Session::end()
{
	d->state = Session::Terminated;
	onEnd();
}

const Q_UINT32 Session::id()
{
	return d->identifier;
}

void Session::setDataStore(QFile *store)
{
	d->file = store;
}

void Session::setState(const SessionState state) const
{
	d->state = state;
}

void Session::setTransport(const Q_UINT32 transportId)
{
	d->transportId = transportId;
}

const Session::SessionState Session::state() const
{
	return d->state;
}

QFile* Session::dataStore()
{
	return d->file;
}

const Q_UINT32 Session::transport()
{
	return d->transportId;
}

void Session::onDataReceived(const QByteArray& data)
{
	kdDebug() << k_funcinfo << endl;
}

void Session::onEndOfData(const Q_INT32 identifier)
{
	kdDebug() << k_funcinfo << endl;
}

void  Session::onMessageSent(const Q_INT32 identifier)
{
	kdDebug() << k_funcinfo << endl;
}

void Session::onMessageReceived(const QByteArray& message, const Q_INT32 identifier, const Q_INT32 relatesTo)
{
	kdDebug() << k_funcinfo << endl;
}

}

#include "session.moc"
