/*
    meanwhileprotocol.cpp - the meanwhile protocol 

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
#include "meanwhileprotocol.h"
#include "meanwhileaddcontactpage.h"
#include "meanwhileeditaccountwidget.h"
#include "meanwhileaccount.h"
#include <kgenericfactory.h>
#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"

#include "mw_common.h"

typedef KGenericFactory<MeanwhileProtocol> MeanwhileProtocolFactory;
K_EXPORT_COMPONENT_FACTORY(kopete_meanwhile,
    MeanwhileProtocolFactory("kopete_meanwhile"))

MeanwhileProtocol::MeanwhileProtocol(QObject* parent, const char *name,
        const QStringList &/*args*/)
: Kopete::Protocol(MeanwhileProtocolFactory::instance(), parent, name),

    statusOffline(Kopete::OnlineStatus::Offline, 25, this, 0, QString::null,
            i18n("Offline"), i18n("Offline"),
            Kopete::OnlineStatusManager::Offline,
	    Kopete::OnlineStatusManager::DisabledIfOffline),

    statusOnline(Kopete::OnlineStatus::Online, 25, this, mwStatus_ACTIVE,
            QString::null, i18n("Online"), i18n("Online"),
            Kopete::OnlineStatusManager::Online, 0),

    statusAway(Kopete::OnlineStatus::Away, 20, this, mwStatus_AWAY,
            "meanwhile_away", i18n("Away"), i18n("Away"),
            Kopete::OnlineStatusManager::Away,
	    Kopete::OnlineStatusManager::HasAwayMessage),

    statusBusy(Kopete::OnlineStatus::Away, 25, this, mwStatus_BUSY,
            "meanwhile_dnd", i18n("Busy"), i18n("Busy"),
          Kopete::OnlineStatusManager::Busy,
	  Kopete::OnlineStatusManager::HasAwayMessage),

    statusIdle(Kopete::OnlineStatus::Away, 30, this, mwStatus_AWAY,
            "meanwhile_idle", i18n("Idle"), i18n("Idle"),
            Kopete::OnlineStatusManager::Idle, 0),

    statusAccountOffline(Kopete::OnlineStatus::Offline, 0, this, 0,
            QString::null, i18n("Account Offline")),

    statusMessage(QString::fromLatin1("statusMessage"),
        i18n("Status Message"), QString::null, false, true),

    awayMessage(Kopete::Global::Properties::self()->awayMessage())
{
    HERE;

    addAddressBookField("messaging/meanwhile", Kopete::Plugin::MakeIndexField);
}

MeanwhileProtocol::~MeanwhileProtocol()
{
}

AddContactPage * MeanwhileProtocol::createAddContactWidget(QWidget *parent,
        Kopete::Account *account )
{
	return new MeanwhileAddContactPage(parent, account);
}

KopeteEditAccountWidget * MeanwhileProtocol::createEditAccountWidget(
        Kopete::Account *account, QWidget *parent )
{
	return new MeanwhileEditAccountWidget(parent, account, this);
}

Kopete::Account *MeanwhileProtocol::createNewAccount(const QString &accountId)
{
	return new MeanwhileAccount(this, accountId, accountId.ascii());
}

Kopete::Contact *MeanwhileProtocol::deserializeContact( 
                            Kopete::MetaContact *metaContact,
                            const QMap<QString, 
                            QString> &serializedData, 
                            const QMap<QString, QString> & /* addressBookData */ )
{
    QString contactId = serializedData[ "contactId" ];
    QString accountId = serializedData[ "accountId" ];

    MeanwhileAccount *theAccount = 
            static_cast<MeanwhileAccount*>(
                            Kopete::AccountManager::self()->
                                    findAccount(pluginId(), accountId));

    if(!theAccount)
    {
        return 0;
    }

    theAccount->addContact(contactId, metaContact, Kopete::Account::DontChangeKABC);
    return theAccount->contacts()[contactId];
}

const Kopete::OnlineStatus MeanwhileProtocol::accountOfflineStatus()
{
    return statusAccountOffline;
}

const Kopete::OnlineStatus MeanwhileProtocol::lookupStatus(
            enum Kopete::OnlineStatusManager::Categories cats)
{
    return Kopete::OnlineStatusManager::self()->onlineStatus(this, cats);
}
#include "meanwhileprotocol.moc"
