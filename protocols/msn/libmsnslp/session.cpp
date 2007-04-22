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

namespace PeerToPeer
{

class Session::SessionPrivate
{
	public:
		SessionPrivate() : state(Session::Created) {}

		Direction direction;
		Q_UINT32 id;
		SessionState state;
};

Session::Session(const Q_UINT32 id, Direction direction, QObject *parent) : QObject(parent), d(new SessionPrivate())
{
	d->id = id;
	d->direction = direction;
}

Session::~Session()
{
	delete d;
}

const Session::Direction Session::direction() const
{
	return d->direction;
}

const Q_UINT32 Session::id() const
{
	return d->id;
}

const Session::SessionState Session::state() const
{
	return d->state;
}

void Session::accept()
{
	// Signal that the session was accepted, locally.
	emit accepted();
}

void Session::cancel()
{
	// Set the session state to cancelled.
	d->state = Session::Canceled;
	// Signal that the session was cancelled, locally.
	emit cancelled();
}

void Session::decline()
{
	// Signal that the session was declined, locally.
	emit declined();
}

void Session::end()
{
	// Set the session state to terminated.
	d->state = Session::Terminated;
	onEnd();
}

void Session::fault()
{
	// Set the session state to faulted.
	d->state = Session::Faulted;
	onFaulted();
	// Signal that the session has faulted.
	emit faulted();
}

void Session::setState(const SessionState& state)
{
	d->state = state;
}

void Session::start()
{
	// Set the session state to established.
	d->state = Session::Established;
	onStart();
}

}

#include "session.moc"
