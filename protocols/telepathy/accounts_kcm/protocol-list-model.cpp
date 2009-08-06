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

#include "protocol-list-model.h"

#include "connection-manager-item.h"
#include "protocol-item.h"

#include <KIcon>
#include <KDebug>

ProtocolListModel::ProtocolListModel(QObject *parent)
 : QAbstractListModel(parent)
{
    kDebug();

    m_connectionManagerItems.clear();
    m_protocolItems.clear();
}

ProtocolListModel::~ProtocolListModel()
{
    kDebug();

    // TODO: Implement me!
}

int ProtocolListModel::rowCount(const QModelIndex &index) const
{
    // If the index is the root item, then return the row count.
    if (index == QModelIndex()) {
       return m_protocolItems.size();
    }

    // Otherwise, return 0 (as this is a list model, so all items
    // are children of the root item).
    return 0;
}

QVariant ProtocolListModel::data(const QModelIndex &index, int role) const
{
    // FIXME: This is a basic implementation just so I can see what's going
    // on while developing this code further. Needs expanding.
    QVariant data;

    switch(role)
    {
    case Qt::DisplayRole:
        data = QVariant(m_protocolItems.at(index.row())->protocol());
        break;
    case Qt::DecorationRole:
        // Look for an icon named im-<protocolname>
        data = QVariant(KIcon((QString("im-%1").arg(m_protocolItems.at(index.row())->protocol()))));
        break;
    default:
        break;
    }

    return data;
}

void ProtocolListModel::addConnectionManager(Tp::ConnectionManagerPtr connectionManager)
{
    kDebug() << "Creating a new ConnectionManagerItem from connection manager:"
             << connectionManager.data();

    // Check if the cm is already in the list.
    foreach (const ConnectionManagerItem* cmi, m_connectionManagerItems) {
        if (cmi->connectionManager() == connectionManager) {
            kWarning() << "Connection Manager: "
                       << cmi->connectionManager()->name()
                       << " is already in the list.";
            return;
        }
    }

    kDebug() << "Connection Manager not already in list. Create new ConnectionManagerItem from cm:"
             << connectionManager.data();

    ConnectionManagerItem *item = new ConnectionManagerItem(connectionManager, this);
    m_connectionManagerItems.append(item);

    connect(item, SIGNAL(newProtocol(const QString&)),
            this, SLOT(onNewProtocol(const QString&)));
}


void ProtocolListModel::onNewProtocol(const QString& protocol)
{
    kDebug();

    // New protocol supported by a connection manager. Get the ConnectionManagerItem for it.
    ConnectionManagerItem *cmItem = qobject_cast<ConnectionManagerItem*>(sender());
    if (!cmItem) {
        kWarning() << "Slot was called by a non-ConnectionManagerItem object.";
        return;
    }

    // Check that the protocol/cm pair are not already in the model.
    foreach (ProtocolItem *pi, m_protocolItems) {
        if ((pi->parent() == cmItem) && (pi->protocol() == protocol)) {
            kWarning() << "Protocol/CM pair already in the model.";
            return;
        }
    }

    // Add the Protocol/CM pair to the model.
    beginInsertRows(QModelIndex(), m_protocolItems.size(), m_protocolItems.size());
    ProtocolItem *item = new ProtocolItem(protocol, cmItem);
    m_protocolItems.append(item);
    endInsertRows();
}

ProtocolItem *ProtocolListModel::itemForIndex(const QModelIndex &index) const
{
    return m_protocolItems.at(index.row());
}


#include "protocol-list-model.moc"

