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
#include "meanwhileserver.h"
#include "meanwhileaccount.h"
#include <kgenericfactory.h>
#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"

MeanwhileProtocol *MeanwhileProtocol::s_protocol = 0L;

typedef KGenericFactory<MeanwhileProtocol> MeanwhileProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( 
                kopete_meanwhile, 
                MeanwhileProtocolFactory( "kopete_meanwhile" ))

MeanwhileProtocol::MeanwhileProtocol( QObject* parent, 
                                      const char *name, 
                                      const QStringList &/*args*/)
    : KopeteProtocol(MeanwhileProtocolFactory::instance(), 
                     parent, name ),
      meanwhileOffline( KopeteOnlineStatus::Offline, 
                        25, this, 0, QString::null,
                        i18n( "Go Offline" ),
                        i18n( "Offline" ) ),
      meanwhileOnline( KopeteOnlineStatus::Online, 
                        25, this, 1, QString::null,
                        i18n( "Go Online" ),
                        i18n( "Online" ) ),
      meanwhileAway( KopeteOnlineStatus::Away, 
                        25, this, 2, "meanwhile_away",
                        i18n( "Go Away" ),
                        i18n( "Away" ) ),
      meanwhileBusy( KopeteOnlineStatus::Away, 
                        25, this, 3, "meanwhile_dnd",
                        i18n( "Mark as busy" ),
                        i18n( "Busy" ) ),
      meanwhileIdle( KopeteOnlineStatus::Away, 
                        25, this, 4, "meanwhile_idle",
                        i18n( "Marked as Idle" ),
                        i18n( "Idle" ) ),
      meanwhileUnknown( KopeteOnlineStatus::Offline, 
                        25, this, 5, "meanwhile_unknown",
                        i18n( "Where am i" ),
                        i18n( "Catch me if you can" ) ),
      statusMessage(QString::fromLatin1("statusMessage"), i18n("Status Message"),QString::null,false,true),
	  awayMessage(Kopete::Global::Properties::self()->awayMessage())
{
//    LOG("MeanwhileProtocol()");
    s_protocol = this;
}

MeanwhileProtocol::~MeanwhileProtocol()
{
}

AddContactPage * MeanwhileProtocol::createAddContactWidget(
                                    QWidget *parent, 
                                    KopeteAccount * account )
{
	return new MeanwhileAddContactPage(parent, account);
}

KopeteEditAccountWidget * MeanwhileProtocol::createEditAccountWidget( 
                                    KopeteAccount *account, 
                                    QWidget *parent )
{
//    LOG("createEditAccountWidget");
	return new MeanwhileEditAccountWidget( parent, account, this );
}

KopeteAccount *MeanwhileProtocol::createNewAccount( 
                                    const QString &accountId )
{
	return new MeanwhileAccount( this, accountId, accountId.ascii() );
}

MeanwhileProtocol *MeanwhileProtocol::protocol()
{
    return s_protocol;
}

KopeteContact *MeanwhileProtocol::deserializeContact( 
                            KopeteMetaContact *metaContact,
                            const QMap<QString, 
                            QString> &serializedData, 
                            const QMap<QString, QString> & /* addressBookData */ )
{
    QString contactId = serializedData[ "contactId" ];
    QString accountId = serializedData[ "accountId" ];

    MeanwhileAccount *theAccount = 
            static_cast<MeanwhileAccount*>(
                            KopeteAccountManager::manager()->
                                    findAccount(protocol()->pluginId(), accountId));

    if(!theAccount)
    {
        return 0;
    }

    theAccount->addContact(contactId, serializedData["displayName"], metaContact, KopeteAccount::DontChangeKABC, serializedData["group"]);
    return theAccount->contacts()[contactId];
}


#include "meanwhileprotocol.moc"
