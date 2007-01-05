/*
    mediasession.cpp - Peer To Peer Media Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "mediasession.h"

namespace PeerToPeer
{

MediaSession::MediaSession(const Q_UINT32 identifier, Session::Direction direction, QObject *parent) : Session(identifier, direction, parent)
{
}

MediaSession::~MediaSession()
{
}

void MediaSession::onBegin()
{
}

void MediaSession::onEnd()
{
}

}

#include "mediasession.moc"
