/*
    wlmserver.h - Kopete Wlm Protocol

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

#ifndef WLMSERVER_H
#define WLMSERVER_H
#include <QObject>
#include "wlmlibmsn.h"

class WlmAccount;

class WlmServer : public QObject
{
  Q_OBJECT
public:
    WlmServer( WlmAccount * account, const QString & m_accountID, const QString & m_password );
    ~WlmServer ();

    void WlmConnect( const QString& server, uint port );
    void WlmDisconnect();

    WlmAccount * m_account;
    QString m_accountID;
    QString m_password;
    Callbacks cb;

    MSN::NotificationServerConnection * mainConnection;
    std::string myFriendlyName;
    std::string myUsername;
};

#endif
