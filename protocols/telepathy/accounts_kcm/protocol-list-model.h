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

#ifndef TELEPATHY_ACCOUNTS_KCM_PROTOCOL_LIST_MODEL_H
#define TELEPATHY_ACCOUNTS_KCM_PROTOCOL_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

#include <TelepathyQt4/ConnectionManager>

class ProtocolItem;
class ConnectionManagerItem;

class ProtocolListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolListModel);

public:
    explicit ProtocolListModel(QObject *parent = 0);
    virtual ~ProtocolListModel();

    virtual int rowCount(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    ProtocolItem *itemForIndex(const QModelIndex &index) const;

    void addConnectionManager(Tp::ConnectionManagerPtr connectionManager);
    void addProtocolItem(ProtocolItem *item);



private Q_SLOTS:
    void onNewProtocol(const QString& protocol);

private:
    QList<ConnectionManagerItem*> m_connectionManagerItems;
    QList<ProtocolItem*> m_protocolItems;
};


#endif // header guard


