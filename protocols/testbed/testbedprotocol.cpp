/*
    testbedprotocol.cpp - Kopete Testbed Protocol

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

#include "testbedprotocol.h"

#include <QList>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"
#include "testbedaccount.h"
#include "testbedcontact.h"
#include "testbedaddcontactpage.h"
#include "testbededitaccountwidget.h"

K_PLUGIN_FACTORY( TestbedProtocolFactory, registerPlugin<TestbedProtocol>(); )
K_EXPORT_PLUGIN( TestbedProtocolFactory( "kopete_testbed" ) )

TestbedProtocol *TestbedProtocol::s_protocol = 0L;

TestbedProtocol::TestbedProtocol( QObject* parent, const QVariantList &/*args*/ )
	: Kopete::Protocol( TestbedProtocolFactory::componentData(), parent ),
	  testbedOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QStringList(QString()),
			  i18n( "Online" ),   i18n( "O&nline" ), Kopete::OnlineStatusManager::Online ),
	  testbedAway(  Kopete::OnlineStatus::Away, 25, this, 1, QStringList(QLatin1String("msn_away")),
			  i18n( "Away" ),   i18n( "&Away" ), Kopete::OnlineStatusManager::Away ),
	  testbedBusy(  Kopete::OnlineStatus::Busy, 25, this, 1, QStringList(QLatin1String("msn_busy")),
			  i18n( "Busy" ),   i18n( "&Busy" ), Kopete::OnlineStatusManager::Busy ),
	  testbedOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QStringList(QString()),
			  i18n( "Offline" ),   i18n( "O&ffline" ), Kopete::OnlineStatusManager::Offline )

{
	kDebug( 14210 ) ;

	s_protocol = this;
}

TestbedProtocol::~TestbedProtocol()
{
}

Kopete::Contact *TestbedProtocol::deserializeContact(
	Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	QString type = serializedData[ "contactType" ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

	TestbedContact::Type tbcType;
	if ( type == QLatin1String( "group" ) )
		tbcType = TestbedContact::Group;
	else if ( type == QLatin1String( "echo" ) )
		tbcType = TestbedContact::Echo;
	else if ( type == QLatin1String( "null" ) )
		tbcType = TestbedContact::Null;
	else
		tbcType = TestbedContact::Null;

	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
	Kopete::Account* account = 0;
	foreach( Kopete::Account* acct, accounts )
	{
		if ( acct->accountId() == accountId )
			account = acct;
	}

	if ( !account )
	{
		kDebug(14210) << "Account doesn't exist, skipping";
		return 0;
	}

	TestbedContact * contact = new TestbedContact(account, contactId, metaContact);
	contact->setType( tbcType );
	contact->setPreferredNameType( nameType );
	return contact;
}

AddContactPage * TestbedProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
	kDebug( 14210 ) << "Creating Add Contact Page";
	return new TestbedAddContactPage( parent );
}

KopeteEditAccountWidget * TestbedProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kDebug(14210) << "Creating Edit Account Page";
	return new TestbedEditAccountWidget( parent, account );
}

Kopete::Account *TestbedProtocol::createNewAccount( const QString &accountId )
{
	return new TestbedAccount( this, accountId );
}

TestbedProtocol *TestbedProtocol::protocol()
{
	return s_protocol;
}



#include "testbedprotocol.moc"
