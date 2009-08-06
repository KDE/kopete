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

#ifndef TELEPATHY_ACCOUNTS_KCM_PROTOCOL_SELECT_WIDGET_H
#define TELEPATHY_ACCOUNTS_KCM_PROTOCOL_SELECT_WIDGET_H

#include <QtGui/QWidget>

class ProtocolItem;

class QModelIndex;

namespace Tp {
    class PendingOperation;
}

class ProtocolSelectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolSelectWidget(QWidget *parent = 0);
    ~ProtocolSelectWidget();

    ProtocolItem *selectedProtocol();

private Q_SLOTS:
    void getConnectionManagerList();
    void onConnectionManagerListGot(Tp::PendingOperation *op);
    void onCurrentChanged(const QModelIndex &current);

Q_SIGNALS:
    void selectedProtocolChanged(ProtocolItem *item);

private:
    class Private;
    Private * const d;
};


#endif  // Header guard

