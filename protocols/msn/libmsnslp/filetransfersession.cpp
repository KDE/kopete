/*
    filetransfersession.cpp - Peer To Peer File Transfer Session

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "filetransfersession.h"

namespace PeerToPeer
{

FileTransferSession::FileTransferSession(const Q_UINT32 identifier, Session::Direction direction, QObject *parent) : Session(identifier, direction, parent)
{
}

FileTransferSession::~FileTransferSession()
{
}

void FileTransferSession::onBegin()
{
}

void FileTransferSession::onEnd()
{
}

}

#include "filetransfersession.moc"
