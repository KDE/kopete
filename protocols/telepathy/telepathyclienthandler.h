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

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCLIENTHANDLER_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCLIENTHANDLER_H

#include <TelepathyQt4/AbstractClientHandler>

class TelepathyClientHandler : public Tp::AbstractClientHandler
{
public:
    TelepathyClientHandler();
    virtual ~TelepathyClientHandler();

    virtual bool bypassApproval() const;
    virtual void handleChannels(const Tp::MethodInvocationContextPtr<> & context,
                                const Tp::AccountPtr & account,
                                const Tp::ConnectionPtr & connection,
                                const QList<Tp::ChannelPtr> & channels,
                                const QList<Tp::ChannelRequestPtr> & requestsSatisfied,
                                const QDateTime & userActionTime,
                                const QVariantMap & handlerInfo);

    struct HandleChannelsData {
        const Tp::MethodInvocationContextPtr<> context;
        const Tp::AccountPtr account;
        const Tp::ConnectionPtr connection;
        const QList<Tp::ChannelPtr> channels;
        const QList<Tp::ChannelRequestPtr> requestsSatisfied;
        const QDateTime userActionTime;
        const QVariantMap handlerInfo;

        HandleChannelsData(const Tp::MethodInvocationContextPtr<> & _context,
                                const Tp::AccountPtr & _account,
                                const Tp::ConnectionPtr & _connection,
                                const QList<Tp::ChannelPtr> & _channels,
                                const QList<Tp::ChannelRequestPtr> & _requestsSatisfied,
                                const QDateTime & _userActionTime,
                                const QVariantMap & _handlerInfo)
          : context(_context),
            account(_account),
            connection(_connection),
            channels(_channels),
            requestsSatisfied(_requestsSatisfied),
            userActionTime(_userActionTime),
            handlerInfo(_handlerInfo)
        { }
    };
};


#endif  // Header guard

