/*
    kircclient.cpp - IRC Client

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2008 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

//#include "kircclientsocket_p.moc"

#include "kircentity.h"
#include "kircstdmessages.h"
#include "kircclienthandler.h"

#include "kirccontext.h"

#include <QtNetwork/QSslSocket>

#include <kdebug.h>

using namespace KIrc;

ClientSocketPrivate::ClientSocketPrivate(ClientSocket *socket)
    : SocketPrivate(socket)
    , failedNickOnLogin(false)
{
}

ClientSocket::ClientSocket(Context *context)
    : Socket(context, new ClientSocketPrivate(this))
{
    Q_D(ClientSocket);
    d->server = new Entity(context);
    d->server->setType(Entity::Server);

    ClientEventHandler *clientHandler = new ClientEventHandler(context);
    context->addEventHandler(clientHandler);
    clientHandler->setEnabled(true);

    context->setEnabled(true);

//	d->versionString = QString::fromLatin1("Anonymous client using the KIRC engine.");
//	d->userString = QString::fromLatin1("Response not supplied by user.");
//	d->sourceString = QString::fromLatin1("Unknown client using the KIRC engine.");
}

ClientSocket::~ClientSocket()
{
//	StdCommands::quit(this, QLatin1String("KIRC Deleted"));
}

EntityPtr ClientSocket::server() const
{
    Q_D(const ClientSocket);
    return d->server;
}

QUrl ClientSocket::url() const
{
    Q_D(const ClientSocket);
    return d->url;
}

void ClientSocket::connectToServer(const QUrl &url)
{
    Q_D(Socket);
    kDebug()<<"connectiong to "<<url;

    QTcpSocket *socket;

    if (url.scheme() == "irc") {
        socket = new QTcpSocket(this);
//		socket->setSocketFlags( KExtendedSocket::inputBufferedSocket | KExtendedSocket::inetSocket );
        connectToServer(url, socket);
    } else if (url.scheme() == "ircs") {
        socket = new QSslSocket(this);
//		socket->setSocketFlags( KExtendedSocket::inetSocket );
        connectToServer(url, socket);
    } else {
//		#warning FIXME: send an event here to reflect the error
    }
}

void ClientSocket::connectToServer(const QUrl &url, QAbstractSocket *socket)
{
    Q_D(ClientSocket);

    close();

    if (!socket) {
//		#warning FIXME: send an event here to reflect the error
    }

    QString host = url.host();
    if (host.isEmpty()) {
        host = "localhost";
    }

    int port = url.port();

    if (port == -1) {
        // Make the port being guessed by the socket (look into /etc/services)
        //port = url.scheme();
    }

//	the given url is now validated
    d->url = url;
    setSocket(socket);

#ifdef __GNUC__
    #warning FIXME: send an event here to reflect connection state
#endif

    socket->connectToHost(host, port);
}

void ClientSocket::socketStateChanged(QAbstractSocket::SocketState newstate)
{
    Q_D(ClientSocket);
    QUrl url = d->url;

    kDebug(14120)<<"state changed to "<<newstate;

    switch (newstate) {
    case QAbstractSocket::ConnectedState:
        setConnectionState(Socket::Authentifying);

        // If password is given for this server, send it now, and don't expect a reply

        //if (url.hasPass())
        //StdMessage::pass(this, url.pass());

#ifdef __GNUC__
        #warning make the following string arguments static const
#endif
        writeMessage(StdMessages::user(url.userName().toLatin1(), "127.0.0.1", url.host().toLatin1(), url.queryItemValue("realname").toLatin1()));
        writeMessage(StdMessages::nick(url.queryItemValue("nickname").toLatin1()));

        break;
    default:
        Socket::socketStateChanged(newstate);
        break;
    }
}

void ClientSocket::setAuthentified()
{
    setConnectionState(Socket::Authentified);
}

KIrc::EntityPtr ClientSocket::joinChannel(const QByteArray &channelName)
{
    Q_D(ClientSocket);
    writeMessage(KIrc::StdMessages::join(channelName));
    KIrc::EntityPtr channel = KIrc::EntityPtr(d->context->entityFromName(channelName));

    return channel;
}
