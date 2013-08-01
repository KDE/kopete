/*
    bonjourprotocol.cpp - Kopete Bonjour Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.u>
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

#include "bonjourprotocol.h"

#include <QList>
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kopeteaccountmanager.h>
#include "bonjouraccount.h"
#include "bonjourcontact.h"
#include "bonjouraddcontactpage.h"
#include "bonjoureditaccountwidget.h"

typedef KGenericFactory<BonjourProtocol> BonjourProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_bonjour, BonjourProtocolFactory( "kopete_bonjour" )  )

BonjourProtocol *BonjourProtocol::s_protocol = 0L;

BonjourProtocol::BonjourProtocol( QObject* parent, const QStringList &/*args*/ )
	: Kopete::Protocol( BonjourProtocolFactory::componentData(), parent ),
	  bonjourOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QStringList(QString()),  
			  i18n( "Online" ),   i18n( "O&nline" ), Kopete::OnlineStatusManager::Online ),
	  bonjourAway(  Kopete::OnlineStatus::Away, 25, this, 1, QStringList(QLatin1String("msn_away")),  
			  i18nc( "This Means the User is Away", "Away" ),   i18nc("This Means the User is Away", "&Away" ), 
			  Kopete::OnlineStatusManager::Away ),
	  bonjourOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QStringList(QString()), 
			  i18n( "Offline" ),   i18n( "O&ffline" ), Kopete::OnlineStatusManager::Offline )


{
	kDebug()<<"Protocol Icon is: "<<pluginIcon();

	s_protocol = this;

}

BonjourProtocol::~BonjourProtocol()
{
}

Kopete::Contact *BonjourProtocol::deserializeContact(
	Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
	Kopete::Account* account = 0;
	foreach( Kopete::Account* acct, accounts )
	{
		if ( acct->accountId() == accountId )
			account = acct;
	}

	if ( !account )
	{
		kDebug() << "Account doesn't exist, skipping";
		return 0;
	}

	BonjourContact * contact = new BonjourContact(account, contactId, metaContact);
	contact->setPreferredNameType(nameType);
	return contact;
}

AddContactPage * BonjourProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
	kDebug()<< "Creating Add Contact Page";
	return new BonjourAddContactPage( parent );
}

KopeteEditAccountWidget * BonjourProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kDebug() << "Creating Edit Account Page";
	return new BonjourEditAccountWidget( parent, account );
}

Kopete::Account *BonjourProtocol::createNewAccount( const QString &accountId )
{
	return new BonjourAccount( this, accountId );
}

BonjourProtocol *BonjourProtocol::protocol()
{
	return s_protocol;
}



#include "bonjourprotocol.moc"
