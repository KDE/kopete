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
	if (d->state == Session::Terminated ||
		d->state == Session::Declined ||
		d->state == Session::Established ||
		d->state == Session::Faulted)
	{
		/// @note Invalid operation in these state.
		return;
	}

	if (d->state != Session::Accepted)
	{
		d->state = Session::Accepted;
		// Signal that the session was accepted, locally.
		emit accepted();
	}
}

void Session::cancel()
{
	if (d->state == Session::Terminated)
	{
		return;
	}

	// Set the session state to terminated.
	d->state = Session::Terminated;

	onCancel();
	// Signal that the session has ended.
	emit ended();
}

void Session::decline()
{
	if (d->state == Session::Terminated ||
		d->state == Session::Accepted ||
		d->state == Session::Established ||
		d->state == Session::Faulted)
	{
		/// @note Invalid operation in these state.
		return;
	}

	if (d->state != Session::Declined)
	{
		d->state = Session::Declined;
		// Signal that the session was declined, locally.
		emit declined();
	}
}

void Session::end(bool sendBye)
{
	if (d->state == Session::Terminated)
	{
		return;
	}

	// Set the session state to terminated.
	d->state = Session::Terminated;

	onEnd();

	if (sendBye)
	{
		// Signal that the session has ended.
		emit ended();
	}
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
	if (d->state == Session::Terminated ||
		d->state == Session::Declined ||
		d->state == Session::Established ||
		d->state == Session::Faulted)
	{
		/// @note Invalid operation in these state.
		return;
	}

	if (d->state != Session::Established)
	{
		// Set the session state to established.
		d->state = Session::Established;
		onStart();
	}
}

void Session::onCancel()
{
	/// @note Default implementation does nothing.
}

}

#include "session.moc"
