/*
    wlmprotocol.cpp - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmprotocol.h"
#include "wlmaccount.h"
#include "wlmcontact.h"
#include "wlmaddcontactpage.h"
#include "wlmeditaccountwidget.h"

#include <kgenericfactory.h>
#include <kdebug.h>
#include <KAboutData>
#include "kopeteaccountmanager.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteappearancesettings.h"
#include "kopeteidentity.h"
#include "kopeteavatarmanager.h"

static const KAboutData aboutdata ("kopete_wlm", 0, ki18n ("WLM"), "1.0");

WlmProtocol *
    WlmProtocol::s_protocol = 0L;
K_PLUGIN_FACTORY (WlmProtocolFactory, registerPlugin < WlmProtocol > ();
    )K_EXPORT_PLUGIN (WlmProtocolFactory ("kopete_wlm"))
    WlmProtocol::WlmProtocol (QObject * parent,
                              const QVariantList & /*args */ )
    :
Kopete::Protocol(WlmProtocolFactory::componentData (), parent, true),
wlmOnline (Kopete::OnlineStatus::Online, 25, this, 1, QStringList (),
           i18n ("Online"), i18n ("O&nline"),
           Kopete::OnlineStatusManager::Online),
wlmAway (Kopete::OnlineStatus::Away, 18, this, 2, QStringList ("contact_away_overlay"),
         i18n ("Away"), i18n ("&Away"), Kopete::OnlineStatusManager::Away),
wlmBusy (Kopete::OnlineStatus::Busy, 20, this, 3, QStringList ("wlm_busy"),
         i18n ("Busy"), i18n ("&Busy"), Kopete::OnlineStatusManager::Busy),
wlmBeRightBack (Kopete::OnlineStatus::Away, 22, this, 4,
                QStringList ("wlm_brb"), i18n ("Be Right Back"),
                i18n ("Be &Right Back"), 0),
wlmOnThePhone (Kopete::OnlineStatus::Busy, 12, this, 5,
               QStringList ("contact_phone_overlay"), i18n ("On the Phone"),
               i18n ("On The &Phone"), 0),
wlmOutToLunch (Kopete::OnlineStatus::Away, 15, this, 6,
               QStringList ("contact_food_overlay"), i18n ("Out to Lunch"),
               i18n ("Out To &Lunch"), 0),
wlmInvisible (Kopete::OnlineStatus::Invisible, 3, this, 7,
              QStringList ("contact_invisible_overlay"), i18n ("Invisible"),
              i18n ("&Invisible"), Kopete::OnlineStatusManager::Invisible),
wlmOffline (Kopete::OnlineStatus::Offline, 0, this, 8,
            QStringList (QString ()), i18n ("Offline"), i18n ("O&ffline"),
            Kopete::OnlineStatusManager::Offline,
            Kopete::OnlineStatusManager::DisabledIfOffline),
wlmIdle (Kopete::OnlineStatus::Away, 10, this, 9,
         QStringList ("contact_away_overlay"), i18n ("Idle"), i18n ("&Idle"),
         Kopete::OnlineStatusManager::Idle),
wlmUnknown (Kopete::OnlineStatus::Unknown, 25, this, 0,
            QStringList ("status_unknown"), i18n ("Status not available")),
wlmConnecting (Kopete::OnlineStatus::Connecting, 2, this, 10,
               QStringList ("wlm_connecting"), i18n ("Connecting")),
currentSong ("currentSong", i18nc ("This is used in the tooltip of a contact", "Listening To")),
contactCapabilities ("contactCapabilities", "Used to keep track of the contact capabilities", QString(), 
		 Kopete::PropertyTmpl::PrivateProperty),
displayPhotoSHA1("displayPhotoSHA1", "Display Photo SHA-1 Hash", QString(),
                 Kopete::PropertyTmpl::PersistentProperty | Kopete::PropertyTmpl::PrivateProperty)
{
    kDebug (14210) << k_funcinfo;

    s_protocol = this;

    //TODO FIXME
    QStringList shownProps =
        Kopete::AppearanceSettings::self ()->toolTipContents ();

    if (!shownProps.contains ("currentSong"))
        shownProps << QString::fromLatin1 ("currentSong");
    Kopete::AppearanceSettings::self ()->setToolTipContents (shownProps);

    setCapabilities (Kopete::Protocol::BaseFgColor |
                     Kopete::Protocol::BaseFont | 
                     Kopete::Protocol::BaseFormatting);
}

WlmProtocol::~WlmProtocol ()
{
}

Kopete::Contact * WlmProtocol::deserializeContact (Kopete::MetaContact *
         metaContact, const QMap <QString, QString> &serializedData,
                           const QMap < QString, QString > & )
{
    QString contactId = serializedData["contactId"];
    QString contactSerial = serializedData["contactSerial"];
    QString accountId = serializedData["accountId"];
    QString dontShowEmoticons = serializedData["dontShowEmoticons"];
    Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

    QList <Kopete::Account*>accounts =
        Kopete::AccountManager::self ()->accounts (this);
    Kopete::Account * account = 0;
    foreach (Kopete::Account * acct, accounts)
    {
        if (acct->accountId () == accountId)
            account = acct;
    }

    if (!account)
    {
        kDebug (14210) << "Account doesn't exist, skipping";
        return 0;
    }
    WlmContact * c = new WlmContact (account, contactId, contactSerial, metaContact);

    if(dontShowEmoticons == "true")
        c->slotDontShowEmoticons(true);

    c->setPreferredNameType(nameType);

    return c;
}

AddContactPage *
WlmProtocol::createAddContactWidget (QWidget * parent, Kopete::Account *account)
{
    kDebug (14210) << "Creating Add Contact Page";
    return new WlmAddContactPage(account, parent);
}

KopeteEditAccountWidget *
WlmProtocol::createEditAccountWidget (Kopete::Account * account,
                                      QWidget * parent)
{
    kDebug (14210) << "Creating Edit Account Page";
    return new WlmEditAccountWidget (parent, account);
}

Kopete::Account * WlmProtocol::createNewAccount (const QString & accountId)
{
    return new WlmAccount (this, accountId);
}

WlmProtocol *
WlmProtocol::protocol ()
{
    return s_protocol;
}

bool WlmProtocol::validContactId (const QString& contactId)
{
    QRegExp rx("[^@\\s]+@([^@\\.\\s]+\\.)+[^@\\.\\s]+");
    return ( rx.exactMatch( contactId ) );
}

#include "wlmprotocol.moc"
