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

#include "telepathychannelhandler.h"

#include "telepathychatsession.h"
#include "telepathyprotocol.h"

#include <KDebug>

#include <kopetechatsessionmanager.h>

#include <TelepathyQt4/TextChannel>

static inline Tp::ChannelClassList channelClassList()
{
    QMap<QString, QDBusVariant> class1;
    class1[TELEPATHY_INTERFACE_CHANNEL ".ChannelType"] =
                                    QDBusVariant(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT);
    class1[TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"] = QDBusVariant(Tp::HandleTypeContact);

    return Tp::ChannelClassList() << Tp::ChannelClass(class1);
}

TelepathyChannelHandler* TelepathyChannelHandler::s_self = 0;

TelepathyChannelHandler::TelepathyChannelHandler(QObject *parent)
 :
   Tp::AbstractClientHandler(channelClassList(), false)
{
    s_self = this;
}

TelepathyChannelHandler::~TelepathyChannelHandler()
{
    s_self = 0;
}

TelepathyChannelHandler *TelepathyChannelHandler::instance()
{
    if (!s_self) {
        s_self = new TelepathyChannelHandler(0);
    }

    return s_self;
}

void TelepathyChannelHandler::handleChannels(const Tp::MethodInvocationContextPtr<> & context,
                                 const Tp::AccountPtr & account,
                                 const Tp::ConnectionPtr & connection,
                                 const QList<Tp::ChannelPtr> & channels,
                                 const QList<Tp::ChannelRequestPtr> & requestsSatisfied,
                                 const QDateTime & userActionTime,
                                 const QVariantMap & handlerInfo)
{
    Q_UNUSED(account);
    Q_UNUSED(connection);
    Q_UNUSED(userActionTime);
    Q_UNUSED(handlerInfo);

    foreach (const Tp::ChannelPtr & channel, channels) {
        Tp::TextChannel *textChannel = qobject_cast<Tp::TextChannel*>(channel.data());
        if (!textChannel) {
            kDebug() << "Ignore non-text channel.";
            continue;
        }

        kDebug() << "handling new channel";

        // Check to see if this channel satisfies a request that was made.
        if (requestsSatisfied.size() == 0) {
            kDebug() << "Other side started this channel.";
            // FIXME: Support remote-initiated channels.
            return;
        }

        kDebug() << "Local initiated channel.";

        foreach (Kopete::ChatSession *session, Kopete::ChatSessionManager::self()->sessions()) {
            TelepathyChatSession *tpc = qobject_cast<TelepathyChatSession*>(session);
            if (!tpc) {
                continue;
            }

            foreach (Tp::ChannelRequestPtr crp, requestsSatisfied) {
                kDebug(TELEPATHY_DEBUG_AREA) << "Channel Request Pointer:" << crp.data();
                kDebug(TELEPATHY_DEBUG_AREA) << "Telepathy Chat Session:" << tpc;
                kDebug(TELEPATHY_DEBUG_AREA) << "Session CRP:" << tpc->channelRequest().data();
                if (tpc->channelRequest()) {
                    kDebug(TELEPATHY_DEBUG_AREA) << "TPC NON_NULL";
                    if (tpc->channelRequest()->userActionTime() == crp->userActionTime()) {
                        tpc->setTextChannel(Tp::TextChannelPtr(textChannel));
                        break;
                    }
                }
            }
        }
    }

    context->setFinished();
}

bool TelepathyChannelHandler::bypassApproval() const
{
    return false;
}
