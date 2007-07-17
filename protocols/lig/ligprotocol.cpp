/*
    ligprotocol.cpp - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"

#include "ligaccount.h"
#include "ligcontact.h"
#include "ligprotocol.h"
#include "ligaddcontactpage.h"
#include "ligeditaccountwidget.h"

typedef KGenericFactory<LigProtocol> LigProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_lig, LigProtocolFactory( "kopete_lig" )  )

LigProtocol *LigProtocol::s_protocol = 0L;

LigProtocol::LigProtocol( QObject* parent, const char *name, const QStringList &/*args*/ )
	: Kopete::Protocol( LigProtocolFactory::instance(), parent, name ),
	  ligOnline(  Kopete::OnlineStatus::Online, 25, this, 0,  QString::null,  i18n( "Online" ),   i18n( "O&nline" ) ),
	  ligAway(  Kopete::OnlineStatus::Away, 25, this, 1, "msn_away",  i18n( "Away" ),   i18n( "&Away" ) ),
	  ligOffline(  Kopete::OnlineStatus::Offline, 25, this, 2,  QString::null,  i18n( "Offline" ),   i18n( "O&ffline" ) )

{
	kdDebug( 14210 ) << k_funcinfo << endl;

	s_protocol = this;
}

LigProtocol::~LigProtocol()
{
}

Kopete::Contact *LigProtocol::deserializeContact(
	Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	QString displayName = serializedData[ "displayName" ];
	QString type = serializedData[ "contactType" ];

	LigContact::LigContactType lcType;
	if ( type == QString::fromLatin1( "echo" ) )
		lcType = LigContact::Echo;
	if ( type == QString::fromLatin1( "null" ) )
		lcType = LigContact::Null;
	else
		lcType = LigContact::Null;

	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( this );

	Kopete::Account *account = accounts[ accountId ];
	if ( !account )
	{
		kdDebug(14210) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	return new LigContact(account, contactId, lcType, displayName, metaContact);
}

AddContactPage * LigProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * /* account */ )
{
	kdDebug( 14210 ) << "Creating Add Contact Page" << endl;
	return new LigAddContactPage( parent );
}

KopeteEditAccountWidget * LigProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kdDebug(14210) << "Creating Edit Account Page" << endl;
	return new LigEditAccountWidget( this, account, parent );
}

Kopete::Account *LigProtocol::createNewAccount( const QString &accountId )
{
	return new LigAccount( this, accountId );
}

LigProtocol *LigProtocol::protocol()
{
	return s_protocol;
}



#include "ligprotocol.moc"



