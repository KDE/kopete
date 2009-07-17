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

#ifndef KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHANNELHANDLER_H
#define KOPETE_PROTOCOL_TELEPATHY_TELEPATHYCHANNELHANDLER_H

#include <QtCore/QObject>

#include <TelepathyQt4/AbstractClientHandler>

class TelepathyChannelHandler : public Tp::AbstractClientHandler
{
public:
    static TelepathyChannelHandler *instance();

    virtual ~TelepathyChannelHandler();

    virtual bool bypassApproval() const;
    virtual void handleChannels(const Tp::MethodInvocationContextPtr<> & context,
                                const Tp::AccountPtr & account,
                                const Tp::ConnectionPtr & connection,
                                const QList<Tp::ChannelPtr> & channels,
                                const QList<Tp::ChannelRequestPtr> & requestsSatisfied,
                                const QDateTime & userActionTime,
                                const QVariantMap & handlerInfo);

protected:

private:
    TelepathyChannelHandler(QObject *parent = 0);
    static TelepathyChannelHandler *s_self;

};


#endif  // Header guard

