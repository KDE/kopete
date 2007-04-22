
/*
    webcamsession.cpp - Peer to Peer Webcam Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "webcamsession.h"
#include <kdebug.h>

namespace PeerToPeer
{

class WebcamSession::WebcamSessionPrivate
{
};

WebcamSession::WebcamSession(const Q_UINT32 id, Direction direction, QObject *parent) : Session(id, direction, parent), d(new WebcamSessionPrivate())
{
}

WebcamSession::~WebcamSession()
{
	delete d;
}

void WebcamSession::handleInvite(const Q_UINT32 appId, const QByteArray& context)
{
	if (appId == 4)
	{
		accept();
	}
	else
	{
		// Otherwise, decline the session.
		decline();
	}
}

void WebcamSession::onStart()
{
}

void WebcamSession::onEnd()
{
}

void WebcamSession::onFaulted()
{
}

void WebcamSession::onDataReceived(const QByteArray& data, bool lastChunk)
{
	Q_UNUSED(data);
	Q_UNUSED(lastChunk);
}

void WebcamSession::onReceive(const QByteArray& bytes, const Q_INT32 id, const Q_INT32 correlationId)
{
	Q_UNUSED(bytes);
	Q_UNUSED(id);
	Q_UNUSED(correlationId);
}

void WebcamSession::onSend(const Q_INT32 id)
{
	Q_UNUSED(id);
}

}

#include "webcamsession.moc"
