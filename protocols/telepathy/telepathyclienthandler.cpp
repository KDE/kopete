/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include "telepathyclienthandler.h"

#include "telepathychannelmanager.h"
#include "telepathyprotocol.h"

#include <KDebug>

#include <QtCore/QDateTime>
#include <QtCore/QVariantMap>

#include <TelepathyQt4/Account>
#include <TelepathyQt4/Channel>
#include <TelepathyQt4/ChannelRequest>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/MethodInvocationContext>
#include <TelepathyQt4/TextChannel>

static inline Tp::ChannelClassList channelClassList()
{
    QMap<QString, QDBusVariant> class1;
    class1[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] =
                                    QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT);
    class1[TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"] = QDBusVariant(Tp::HandleTypeContact);

    return Tp::ChannelClassList() << Tp::ChannelClass(class1);
}

TelepathyClientHandler::TelepathyClientHandler()
 : Tp::AbstractClientHandler(channelClassList(), false)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

TelepathyClientHandler::~TelepathyClientHandler()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyClientHandler::handleChannels(const Tp::MethodInvocationContextPtr<> & context,
                                 const Tp::AccountPtr & account,
                                 const Tp::ConnectionPtr & connection,
                                 const QList<Tp::ChannelPtr> & channels,
                                 const QList<Tp::ChannelRequestPtr> & requestsSatisfied,
                                 const QDateTime & userActionTime,
                                 const QVariantMap & handlerInfo)
{
    HandleChannelsData *data = new HandleChannelsData(context, account, connection, channels,
                                                      requestsSatisfied, userActionTime,
                                                      handlerInfo);

    TelepathyChannelManager::instance()->handleChannels(data);
}

bool TelepathyClientHandler::bypassApproval() const
{
    return true;
}

