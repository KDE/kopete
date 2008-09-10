/*
    wlmserver.cpp - Kopete Wlm Protocol

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
#include "wlmserver.h"

WlmServer::WlmServer (WlmAccount * account, QString & accountID, QString & password):
m_account (account),
m_accountID (accountID), m_password (password), mainConnection (NULL)
{
}

WlmServer::~WlmServer ()
{
    WlmDisconnect ();
    qDeleteAll(cb.socketList);
}

void
WlmServer::WlmConnect ()
{
    cb.m_server = this;
    cb.sessionID = 123456;
    mainConnection =
        new MSN::NotificationServerConnection (m_accountID.toLatin1 ().data (),
                                               m_password.toLatin1 ().data (),
                                               cb);
    cb.mainConnection = mainConnection;

    if (mainConnection)
        mainConnection->connect ("messenger.hotmail.com", 1863);
}

void
WlmServer::WlmDisconnect ()
{
    WlmSocket *a = 0;

    if (mainConnection)
    {
        if (mainConnection->connectionState () !=
            MSN::NotificationServerConnection::NS_DISCONNECTED)
        {
            delete mainConnection;
            mainConnection = NULL;
        }
    }

    QListIterator<WlmSocket *> i(cb.socketList);
    while (i.hasNext())
    {
        a = i.next();
        QObject::disconnect (a, 0, 0, 0);
        QObject::disconnect (a->sock, 0, 0, 0);
        a->sock->enableRead (false);
        a->sock->enableWrite (false);
        a->sock->disconnect ();
        cb.socketList.removeAll (a);
    }
    cb.socketList.clear ();
}

#include "wlmserver.moc"
