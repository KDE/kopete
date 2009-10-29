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

#include <telepathyprotocolinternal.h>

#include <telepathychannelmanager.h>

#include "kopeteglobal.h"

#include <KDebug>
#include <KLocale>

#include <TelepathyQt4/Types>

TelepathyProtocolInternal *TelepathyProtocolInternal::s_self = 0;

TelepathyProtocolInternal::TelepathyProtocolInternal(Kopete::Protocol *protocol)
  : QObject(protocol),
    Available(Kopete::OnlineStatus::Online, 25, protocol, 1,
              QStringList(), i18n("Available"), i18n("A&vailable"),
              Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasStatusMessage),
    Away(Kopete::OnlineStatus::Away, 18, protocol, 4,
         QStringList(QString::fromLatin1("contact_away_overlay")), i18n("Away"), i18n("&Away"),
         Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasStatusMessage),
    Busy(Kopete::OnlineStatus::Away, 20, protocol, 2,
         QStringList("contact_busy_overlay"), i18n("Busy"), i18n("&Busy"),
         Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage),
    Connecting(Kopete::OnlineStatus::Connecting, 50, protocol, 10,
               QStringList(QString::fromLatin1("contact_connecting_overlay")),
               i18n("Connecting"), i18n("Connecting"),
               Kopete::OnlineStatusManager::Offline, Kopete::OnlineStatusManager::HideFromMenu),
    ExtendedAway(Kopete::OnlineStatus::Away, 15, protocol, 4,
                 QStringList(QString::fromLatin1("contact_away_overlay")), i18n("Away"), i18n("Away"),
                 Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HideFromMenu),
    Hidden(Kopete::OnlineStatus::Invisible, 3, protocol, 8,
           QStringList(QString::fromLatin1("contact_invisible_overlay")),
           i18n("Invisible"), i18n("&Hidden"),
           Kopete::OnlineStatusManager::Invisible),
    Offline(Kopete::OnlineStatus::Offline, 0, protocol, 7,
            QStringList(), i18n("Offline"), i18n("&Offline"),
            Kopete::OnlineStatusManager::Offline, Kopete::OnlineStatusManager::DisabledIfOffline),
    propAvatarToken("telepathyAvatarToken", i18n("Telepathy Avatar token"),
                    QString(), Kopete::PropertyTmpl::PersistentProperty |
                    Kopete::PropertyTmpl::PrivateProperty),
    propPhoto(Kopete::Global::Properties::self()->photo()),
    m_protocol(protocol)
{
    kDebug();

    Tp::registerTypes();

    s_self = this;

    // Ensure the ChannelManager singleton is constructed.
    TelepathyChannelManager::instance();
}

TelepathyProtocolInternal *TelepathyProtocolInternal::protocolInternal()
{
    return s_self;
}

Kopete::Protocol *TelepathyProtocolInternal::protocol() const
{
    return m_protocol;
}

QString TelepathyProtocolInternal::formatTelepathyConfigGroup(const QString &connectionManager,
                                                      const QString &protocol,
                                                      const QString &account)
{
    kDebug();

    return QString("Telepathy_%1_%2_%3").arg(connectionManager).arg(protocol).arg(account);
}

Tp::ConnectionPresenceType TelepathyProtocolInternal::kopeteStatusToTelepathy(const Kopete::OnlineStatus &kPresence)
{
    kDebug();

    Tp::ConnectionPresenceType tpPresence;

    Kopete::OnlineStatusManager::Categories categories = kPresence.categories();

    if ((categories == Kopete::OnlineStatusManager::FreeForChat) ||
        (categories == Kopete::OnlineStatusManager::Online)) {
        tpPresence = Tp::ConnectionPresenceTypeAvailable;
    } else if ((categories == Kopete::OnlineStatusManager::Idle) ||
               (categories == Kopete::OnlineStatusManager::Away)) {
        tpPresence = Tp::ConnectionPresenceTypeAway;
    } else if (categories == Kopete::OnlineStatusManager::ExtendedAway) {
        tpPresence = Tp::ConnectionPresenceTypeExtendedAway;
    } else if (categories == Kopete::OnlineStatusManager::Busy) {
        tpPresence = Tp::ConnectionPresenceTypeBusy;
    } else if (categories == Kopete::OnlineStatusManager::Invisible) {
        tpPresence = Tp::ConnectionPresenceTypeHidden;
    } else {
        tpPresence = Tp::ConnectionPresenceTypeOffline;
    }

    return tpPresence;
}

Kopete::OnlineStatus TelepathyProtocolInternal::telepathyStatusToKopete(Tp::ConnectionPresenceType tpPresence)
{
    kDebug();

    Kopete::OnlineStatus kPresence;

    switch (tpPresence)
    {
    case Tp::ConnectionPresenceTypeAvailable:
        kPresence = Available;
        break;
    case Tp::ConnectionPresenceTypeAway:
        kPresence = Away;
        break;
    case Tp::ConnectionPresenceTypeExtendedAway:
        kPresence = ExtendedAway;
        break;
    case Tp::ConnectionPresenceTypeBusy:
        kPresence = Busy;
        break;
    case Tp::ConnectionPresenceTypeHidden:
        kPresence = Hidden;
        break;
    case Tp::ConnectionPresenceTypeOffline:
        kPresence = Offline;
        break;
    default:
        kPresence = Kopete::OnlineStatus::Unknown;
        break;
    }

    return kPresence;
}


#include "telepathyprotocolinternal.moc"

