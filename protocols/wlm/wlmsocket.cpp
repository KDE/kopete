/*
    wlmsocket.cpp - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <QObject>
#include <k3streamsocket.h>
#include <k3socketdevice.h>
#include "wlmsocket.h"

void
WlmSocket::canWriteData ()
{
    MSN::Connection * c;

    if (!sock || !mainConnection)
        return;
    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mainConnection->connectionWithSocket (sock->socketDevice ()->
                                              socket ());

    // if this is a libmsn socket
    if (c != NULL)
    {
        if (c->isConnected () == false)
        {
            c->socketConnectionCompleted ();
        }
        // If this event is due to new data becoming available 
        c->socketIsWritable ();
    }
}

void
WlmSocket::disconnected ()
{
    if (sock)
    {
        sock->deleteLater ();
        sock = NULL;
    }
    deleteLater ();
}

void
WlmSocket::connected1 ()
{
/*   MSN::Connection *c;
   // Retrieve the connection associated with the
   // socket's file handle on which the event has
   // occurred.
   c = mainConnection->connectionWithSocket(sock->socketDevice()->socket());
   c->socketConnectionCompleted();
   QSocket::connect(sock, SIGNAL(readyRead()), this, SLOT(incomingData()));
   QSocket::connect(sock, SIGNAL(readyWrite()), this, SLOT(canWriteData()));
   */
}

void
WlmSocket::incomingData ()
{
    MSN::Connection * c;

    if (!sock || !mainConnection)
        return;

    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mainConnection->connectionWithSocket (sock->socketDevice ()->
                                              socket ());

    // if this is a libmsn socket
    if (c != NULL)
    {
        if (c->isConnected () == false)
        {
            c->socketConnectionCompleted ();
        }
        // If this event is due to new data becoming available 
        c->dataArrivedOnSocket ();
    }
    //handle_command(mainConnection);
}

WlmSocket::~WlmSocket ()
{
    if (sock)
    {
        sock->deleteLater ();
        sock = NULL;
    }
}

#include "wlmsocket.moc"
