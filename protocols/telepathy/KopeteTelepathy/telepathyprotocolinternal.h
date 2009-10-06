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

#ifndef LIB_KOPETE_TELEPATHY_TELEPATHYPROTOCOL_H
#define LIB_KOPETE_TELEPATHY_TELEPATHYPROTOCOL_H

#include <kopeteonlinestatus.h>
#include <kopeteprotocol.h>
#include <kopeteproperty.h>

#include <TelepathyQt4/ClientRegistrar>
#include <TelepathyQt4/Constants>

class KOPETE_EXPORT TelepathyProtocolInternal : public QObject
{
    Q_OBJECT

public:

    /* Online Statuses */
    const Kopete::OnlineStatus Available;
    const Kopete::OnlineStatus Away;
    const Kopete::OnlineStatus Busy;
    const Kopete::OnlineStatus Connecting;
    const Kopete::OnlineStatus ExtendedAway;
    const Kopete::OnlineStatus Hidden;   
    const Kopete::OnlineStatus Offline;

    /* Extra properties needed */
    const Kopete::PropertyTmpl propAvatarToken;
    const Kopete::PropertyTmpl propPhoto;

    /* Constructor/Static Accessor */
    TelepathyProtocolInternal(Kopete::Protocol *protocol);

    static TelepathyProtocolInternal *protocolInternal();

    Kopete::Protocol *protocol() const;

    /* Convenience Methods */
    QString formatTelepathyConfigGroup(const QString &connectionManager,
                                       const QString &protocol,
                                       const QString &account);

    Tp::ConnectionPresenceType kopeteStatusToTelepathy(const Kopete::OnlineStatus &kPresence);
    Kopete::OnlineStatus telepathyStatusToKopete(Tp::ConnectionPresenceType tpPresence);    

private:
    static TelepathyProtocolInternal *s_self;
    Tp::ClientRegistrarPtr m_clientRegistrar;
    Kopete::Protocol *m_protocol;

};


#endif  //header guard

