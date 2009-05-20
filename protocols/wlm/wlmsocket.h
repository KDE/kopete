/*
    wlmsocket.h - Kopete Wlm Protocol

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

#ifndef WLMSOCKET_H
#define WLMSOCKET_H

#include <QObject>

#include <QSslSocket>

#include <msn/msn.h>

class WlmSocket : public QSslSocket
{
  Q_OBJECT 
public:
    MSN::NotificationServerConnection * mainConnection;
    bool main;
    bool m_isSSL;
    explicit WlmSocket(MSN::NotificationServerConnection * mainConnection, bool isSSL = false);
    ~WlmSocket ();
    bool isSSL(){ return m_isSSL; }

public slots:
    void incomingData ();
    void connectionReady ();
    void connectionFinished ();
    void connectionEncryptedReady();

};
#endif
