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

#include "protocol-item.h"

#include "connection-manager-item.h"

#include <KDebug>

ProtocolItem::ProtocolItem(const QString &protocol, ConnectionManagerItem *parent)
 : QObject(parent),
   m_protocol(protocol)
{
    kDebug() << "Creating new ProtocolItem with cmItem: " << parent << " and protocol;" << protocol;

    // TODO: Implement me!
}

ProtocolItem::~ProtocolItem()
{
    kDebug();

    // TODO: Implement me...
}

QString ProtocolItem::protocol() const
{
    return m_protocol;
}

Tp::ProtocolParameterList ProtocolItem::mandatoryParameters() const
{
    kDebug();

    ConnectionManagerItem *item = qobject_cast<ConnectionManagerItem*>(parent());

    Tp::ConnectionManagerPtr cm = item->connectionManager();

    Tp::ProtocolParameterList mandatoryParameters;
    foreach (Tp::ProtocolInfo *info, cm->protocols()) {
        if (info->name() == m_protocol) {
            foreach (Tp::ProtocolParameter *parameter, info->parameters()) {
                if (parameter->isRequired()) {
                    mandatoryParameters << parameter;
                }
            }
        }
    }

    return mandatoryParameters;
}

Tp::ProtocolParameterList ProtocolItem::optionalParameters() const
{
    kDebug();

    ConnectionManagerItem *item = qobject_cast<ConnectionManagerItem*>(parent());

    Tp::ConnectionManagerPtr cm = item->connectionManager();

    Tp::ProtocolParameterList optionalParameters;
    foreach (Tp::ProtocolInfo *info, cm->protocols()) {
        if (info->name() == m_protocol) {
            foreach (Tp::ProtocolParameter *parameter, info->parameters()) {
                if (!parameter->isRequired()) {
                    optionalParameters << parameter;
                }
            }
        }
    }

    return optionalParameters;
}


#include "protocol-item.moc"

