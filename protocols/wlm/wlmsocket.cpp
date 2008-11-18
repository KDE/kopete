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

#include "wlmsocket.h"

#include <QObject>

WlmSocket::WlmSocket (MSN::NotificationServerConnection * mainConnection, bool isSSL) :
    m_isSSL(isSSL)
{
    main = false;
    this->mainConnection = mainConnection;

    QObject::connect (this, SIGNAL (connected ()), this,
                      SLOT (connectionReady ()));
    QObject::connect (this, SIGNAL (disconnected ()), this, 
                      SLOT (connectionFinished ()));
    QObject::connect (this, SIGNAL (encrypted ()), this, 
                      SLOT (connectionEncryptedReady()));

}

void
WlmSocket::connectionEncryptedReady()
{
    MSN::Connection * c;
    
    if (!mainConnection)
        return;
    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mainConnection->connectionWithSocket ((void*)this);

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
WlmSocket::connectionReady ()
{
    MSN::Connection * c;
    
    // ssl is connected when encrypted() is raised
    if(isSSL())
        return;

    if (!mainConnection)
        return;
    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mainConnection->connectionWithSocket ((void*)this);

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
WlmSocket::connectionFinished ()
{
}

void
WlmSocket::incomingData ()
{
    MSN::Connection * c;

    if (!mainConnection)
        return;

    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mainConnection->connectionWithSocket ((void*)this);

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
}

WlmSocket::~WlmSocket ()
{
}

#include "wlmsocket.moc"
