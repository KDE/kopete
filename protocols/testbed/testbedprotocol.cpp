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
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"

#include "testbedaccount.h"
#include "testbedcontact.h"
#include "testbedprotocol.h"
#include "testbedaddcontactpage.h"
#include "testbededitaccountwidget.h"

typedef KGenericFactory<TestbedProtocol> TestbedProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_testbed, TestbedProtocolFactory( "kopete_testbed" )  )

TestbedProtocol *TestbedProtocol::s_protocol = 0L;

TestbedProtocol::TestbedProtocol( QObject* parent, const char *name, const QStringList &/*args*/ )
	: Kopete::Protocol( TestbedProtocolFactory::instance(), parent, name ),
	  testbedOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QString::null,  i18n( "Online" ),   i18n( "O&nline" ) ),
	  testbedAway(  Kopete::OnlineStatus::Away, 25, this, 1, "msn_away",  i18n( "Away" ),   i18n( "&Away" ) ),
	  testbedOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QString::null,  i18n( "Offline" ),   i18n( "O&ffline" ) )

{
	kdDebug( 14210 ) << k_funcinfo << endl;

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
	QString displayName = serializedData[ "displayName" ];
	QString type = serializedData[ "contactType" ];

	TestbedContact::TestbedContactType tbcType;
	if ( type == QString::fromLatin1( "echo" ) )
		tbcType = TestbedContact::Echo;
	if ( type == QString::fromLatin1( "null" ) )
		tbcType = TestbedContact::Null;
	else
		tbcType = TestbedContact::Null;

	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( this );

	Kopete::Account *account = accounts[ accountId ];
	if ( !account )
	{
		kdDebug(14210) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	return new TestbedContact(account, contactId, tbcType, displayName, metaContact);
}

AddContactPage * TestbedProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
	kdDebug( 14210 ) << "Creating Add Contact Page" << endl;
	return new TestbedAddContactPage( parent );
}

KopeteEditAccountWidget * TestbedProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kdDebug(14210) << "Creating Edit Account Page" << endl;
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
