/*
    meanwhileprotocl.h - the meanwhile protocol definition

    Copyright (c) 2005      by Jeremy Kerr <jk@ozlabs.org>
    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef MEANWHILEPROTOCOL_H
#define MEANWHILEPROTOCOL_H

#include <kopeteprotocol.h>

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"
#include "addcontactpage.h"

#include <kdebug.h>
#define MEANWHILE_DEBUG 14200
#define HERE kDebug(MEANWHILE_DEBUG) << endl
#define mwDebug() kDebug(MEANWHILE_DEBUG)


class MeanwhileProtocol : public Kopete::Protocol
{
    Q_OBJECT
public:
    MeanwhileProtocol(QObject *parent, const QVariantList &args);

    ~MeanwhileProtocol();

    virtual AddContactPage *createAddContactWidget(QWidget *parent,
            Kopete::Account *account);

    virtual KopeteEditAccountWidget *createEditAccountWidget(
            Kopete::Account *account, QWidget *parent);

    virtual Kopete::Account *createNewAccount(const QString &accountId);

    virtual Kopete::Contact *deserializeContact(
            Kopete::MetaContact *metaContact,
            const QMap<QString,QString> &serializedData,
            const QMap<QString, QString> &addressBookData);

    const Kopete::OnlineStatus accountOfflineStatus();

    const Kopete::OnlineStatus lookupStatus(
            Kopete::OnlineStatusManager::Categories cats);

    const Kopete::OnlineStatus statusOffline;
    const Kopete::OnlineStatus statusOnline;
    const Kopete::OnlineStatus statusAway;
    const Kopete::OnlineStatus statusBusy;
    const Kopete::OnlineStatus statusIdle;
    const Kopete::OnlineStatus statusAccountOffline;

// TODO
//    Kopete::ContactPropertyTmpl statusMessage;
//    Kopete::ContactPropertyTmpl awayMessage;

};

#endif
