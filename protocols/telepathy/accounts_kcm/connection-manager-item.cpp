/*
 * This file is part of telepathy-accounts-kcm
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

#include "connection-manager-item.h"

#include "protocol-list-model.h"

#include <KDebug>

#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

ConnectionManagerItem::ConnectionManagerItem(const Tp::ConnectionManagerPtr &connectionManager,
                                             ProtocolListModel *parent)
 : QObject(parent),
   m_connectionManager(connectionManager)
{
    kDebug();

    // Get the connection manager ready
    connect(m_connectionManager->becomeReady(Tp::ConnectionManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onConnectionManagerReady(Tp::PendingOperation*)));
}

ConnectionManagerItem::~ConnectionManagerItem()
{
    kDebug();

    // TODO: Implement me...
}

Tp::ConnectionManagerPtr ConnectionManagerItem::connectionManager() const
{
    kDebug();

    return m_connectionManager;
}

void ConnectionManagerItem::onConnectionManagerReady(Tp::PendingOperation *op)
{
    kDebug();

    if (op->isError()) {
        kDebug() << "An error occurred in making the Connection Manager ready."
                 << op->errorName()
                 << op->errorMessage();
        return;
    }

    // Emit the protocol() signal for every protocol supported by this connection manager.
    foreach (const QString &protocol, m_connectionManager->supportedProtocols()) {
        Q_EMIT newProtocol(protocol);
    }
}


#include "connection-manager-item.moc"

