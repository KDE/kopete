/*
    ircnetwork.h - IRC Network List

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCNETWORKLIST_H
#define IRCNETWORKLIST_H

#include <QList>
#include <QObject>

namespace IRC {
struct Host
{
    QString host;
    uint port;
    QString password;
    bool ssl;
};

struct Network
{
    QString name;
    QString description;
    QList<Host> hosts;
};

typedef QList<IRC::Network> NetworkList;

class Networks : public QObject
{
    Q_OBJECT

private:
    Networks();
    ~Networks();

public:
    static IRC::Networks *self();

    const IRC::NetworkList &networks() const;
    void setNetworks(const IRC::NetworkList &networks);

    const IRC::Network &network(const QString &name);

public slots:
    void slotReadNetworks();
    bool slotSaveNetworkConfig() const;

//	void addNetwork(const IRCNetwork &network);

private:
    struct Private;
    Private *const d;
};
}

#endif
