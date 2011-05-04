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

#include "wlmserver.h"
#include "wlmsocket.h"

WlmServer::WlmServer (WlmAccount * account, const QString & accountID, const QString & password):
m_account (account),
m_accountID (accountID), m_password (password), mainConnection (NULL)
{
}

WlmServer::~WlmServer ()
{
    qDeleteAll(cb.socketList);
    delete mainConnection;
}

void
WlmServer::WlmConnect ( const QString& server, uint port )
{
    cb.m_server = this;
    mainConnection =
        new MSN::NotificationServerConnection (m_accountID.toLatin1 ().constData (),
                                               m_password.toUtf8().constData (),
                                               cb);
    cb.mainConnection = mainConnection;

    if (mainConnection)
        mainConnection->connect (server.toLatin1().constData(), port);
}

void
WlmServer::WlmDisconnect ()
{
    WlmSocket *a = 0;

    if (mainConnection)
    {
        QListIterator<WlmSocket *> i(cb.socketList);
        while (i.hasNext())
        {
            a = i.next();
            QObject::disconnect (a, 0, 0, 0);
            cb.socketList.removeAll (a);
        }
        cb.socketList.clear ();

        if ( mainConnection->connectionState() != MSN::NotificationServerConnection::NS_DISCONNECTED )
            mainConnection->disconnect();
    }
}

#include "wlmserver.moc"
