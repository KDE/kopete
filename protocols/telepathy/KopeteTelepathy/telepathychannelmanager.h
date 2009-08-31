/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHANNELMANAGER_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHANNELMANAGER_H

#include <KopeteTelepathy/telepathyclienthandler.h>

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QPair>

#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

class TelepathyChannelManager : public QObject
{
    Q_OBJECT

public:
    static TelepathyChannelManager *instance();

    virtual ~TelepathyChannelManager();

    void handleChannels(TelepathyClientHandler::HandleChannelsData *data);

private Q_SLOTS:
    void onChannelReady(Tp::PendingOperation *op);

private:
    TelepathyChannelManager(QObject *parent = 0);
    static TelepathyChannelManager *s_self;

    TelepathyClientHandler *m_clientHandler;
    QMap<Tp::PendingReady*, QPair<Tp::TextChannelPtr, TelepathyClientHandler::HandleChannelsData*> > m_channelToHandleChannelData;
    Tp::ClientRegistrarPtr m_clientRegistrar;
};


#endif  // Header guard

