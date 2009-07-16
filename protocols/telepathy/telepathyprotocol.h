/*
 * telepathyprotocol.h - Windows Live Telepathy Kopete protocol definition.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef TELEPATHYPROTOCOL_H
#define TELEPATHYPROTOCOL_H

#include <kopeteprotocol.h>
#include <kopeteonlinestatus.h>
#include <kopeteproperty.h>

#include <TelepathyQt4/Constants>

#define TELEPATHY_DEBUG_AREA 14400

namespace Kopete
{
class Account;
}

//class AddContactPage;
//class KopeteEditAccountWidget;

/**
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT TelepathyProtocol : public Kopete::Protocol
{
    Q_OBJECT
public:
    const Kopete::OnlineStatus Available;
    const Kopete::OnlineStatus Away;
    const Kopete::OnlineStatus Busy;
    const Kopete::OnlineStatus Hidden;
    const Kopete::OnlineStatus ExtendedAway;
    const Kopete::OnlineStatus Offline;

    const Kopete::PropertyTmpl propAvatarToken;

    TelepathyProtocol(QObject *parent, const QVariantList &args);

    virtual Kopete::Account *createNewAccount(const QString &accountId);
    virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);
    virtual KopeteEditAccountWidget * createEditAccountWidget(Kopete::Account *account,
                                                              QWidget *parent);

    static TelepathyProtocol *protocol();

    virtual Kopete::Contact *deserializeContact(Kopete::MetaContact *metaContact,
                                                const QMap<QString,
                                                QString> &serializedData,
                                                const QMap<QString, QString> &addressBookData);
    QString formatTelepathyConfigGroup(const QString &connectionManager,
                                       const QString &protocol,
                                       const QString &accountId);

    Tp::ConnectionPresenceType kopeteStatusToTelepathy(const Kopete::OnlineStatus &status);
    Kopete::OnlineStatus telepathyStatusToKopete(Tp::ConnectionPresenceType presence);

private:
    static TelepathyProtocol *s_self;
};


#endif  //header guard

