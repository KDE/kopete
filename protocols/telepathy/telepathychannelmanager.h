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

#include <telepathyclienthandler.h>

#include <QtCore/QObject>

#include <TelepathyQt4/ClientRegistrar>

class TelepathyContact;

class TelepathyChannelManager : public QObject
{
    Q_OBJECT

public:
    static TelepathyChannelManager *instance();

    virtual ~TelepathyChannelManager();

    void handleChannels(TelepathyClientHandler::HandleChannelsData *data);

private:
    TelepathyChannelManager(QObject *parent = 0);
    static TelepathyChannelManager *s_self;

    void handleTextChannel(Tp::ChannelPtr channel, TelepathyContact *contact,
                           TelepathyClientHandler::HandleChannelsData *data);
    void handleFileTransferChannel(Tp::ChannelPtr channel, TelepathyContact *contact,
                                   TelepathyClientHandler::HandleChannelsData *data);

    TelepathyContact *getTpContact(Tp::AccountPtr account,
                                   const QString &contactId);
    TelepathyClientHandler *m_clientHandler;
    Tp::ClientRegistrarPtr m_clientRegistrar;
};


#endif  // Header guard

