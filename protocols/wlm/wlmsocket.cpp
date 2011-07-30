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

#include <QTimer>
#include <QNetworkProxy>

#include "kopetesockettimeoutwatcher.h"
#include "kopeteaccount.h"
#include "wlmaccount.h"
#include "wlmserver.h"
#include <kopetetransfermanager.h>
#include <kopetecontact.h>

WlmSocket::WlmSocket(MSN::NotificationServerConnection * mainConnection, bool isSSL, WlmServer* server)
: mMainConnection(mainConnection), mIsSSL(isSSL), mPingTimer(0)
{
    QObject::connect( this, SIGNAL(connected()), this, SLOT(connectionReady()) );
    QObject::connect( this, SIGNAL(disconnected()), this, SLOT(connectionFinished()) );
    QObject::connect( this, SIGNAL(encrypted()), this, SLOT(connectionEncryptedReady()) );
    QObject::connect( this, SIGNAL(bytesWritten(qint64)), this, SLOT(resetPing()) );

    Kopete::SocketTimeoutWatcher* timeoutWatcher = Kopete::SocketTimeoutWatcher::watch( this );
    if ( timeoutWatcher )
        connect( timeoutWatcher, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)) );

    if (server && server->m_account)
    {
        WlmAccount *acc= server->m_account;
        if(acc->isProxyEnabled())
        {
            QString proxyHost = acc->proxyHost();
            uint proxyPort = acc->proxyPort();
            QNetworkProxy::ProxyType proxyType = (QNetworkProxy::ProxyType) acc->proxyType();
            QString proxyUsername = acc->proxyUsername();
            QString proxyPassword = acc->proxyPassword();
            setProxy(QNetworkProxy(proxyType, proxyHost, proxyPort, proxyUsername, proxyPassword));
        }
    }
}

WlmSocket::~WlmSocket()
{
    delete mPingTimer;
}

void WlmSocket::connectionEncryptedReady()
{
    MSN::Connection * c;
    
    if ( !mMainConnection )
        return;
    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mMainConnection->connectionWithSocket( (void*)this );

    // if this is a libmsn socket
    if (c != NULL)
    {
        if ( c->isConnected() == false )
        {
            c->socketConnectionCompleted();
        }
        // If this event is due to new data becoming available 
        c->socketIsWritable();
    }

    if ( c == mMainConnection )
        initPingTimer();
}

void WlmSocket::connectionReady()
{
    MSN::Connection * c;
    
    // ssl is connected when encrypted() is raised
    if( isSSL() )
        return;

    if ( !mMainConnection )
        return;
    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mMainConnection->connectionWithSocket( (void*)this );

    // if this is a libmsn socket
    if (c != NULL)
    {
        if (c->isConnected() == false)
        {
            c->socketConnectionCompleted();
        }
        // If this event is due to new data becoming available 
        c->socketIsWritable();
    }

    if ( c == mMainConnection )
        initPingTimer();
}

void WlmSocket::connectionFinished()
{
    delete mPingTimer;
    mPingTimer = 0;
}

void WlmSocket::incomingData()
{
    MSN::Connection * c;

    if ( !mMainConnection )
        return;

    // Retrieve the connection associated with the
    // socket's file handle on which the event has
    // occurred.
    c = mMainConnection->connectionWithSocket((void*)this);

    // if this is a libmsn socket
    if (c != NULL)
    {
        if (c->isConnected() == false)
        {
            c->socketConnectionCompleted();
        }
        // If this event is due to new data becoming available 
        c->dataArrivedOnSocket();
    }
}

void WlmSocket::resetPing()
{
    if ( mPingTimer )
        mPingTimer->start();
}

void WlmSocket::pingTimeout()
{
    if ( !mMainConnection || (mMainConnection->connectionState() < MSN::NotificationServerConnection::NS_CONNECTED) )
        return;

    MSN::Connection *c = mMainConnection->connectionWithSocket((void*)this);
    if ( c == mMainConnection )
        mMainConnection->sendPing();
}

void WlmSocket::initPingTimer()
{
    if ( !mPingTimer )
    {
        mPingTimer = new QTimer();
        QObject::connect( mPingTimer, SIGNAL(timeout()), this, SLOT(pingTimeout()) );
        mPingTimer->setInterval( 50000 );
    }
    mPingTimer->start();
}

#include "wlmsocket.moc"
