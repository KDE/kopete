/*
    gwprotocol.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
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

#include "gwaccount.h"
#include "gwcontact.h"
#include "gwprotocol.h"
#include "gwaddcontactpage.h"
#include "gweditaccountwidget.h"

typedef KGenericFactory<GroupWiseProtocol> GroupWiseProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_groupwise, GroupWiseProtocolFactory( "kopete_groupwise" )  )

GroupWiseProtocol *GroupWiseProtocol::s_protocol = 0L;

GroupWiseProtocol::GroupWiseProtocol( QObject* parent, const char *name, const QStringList &/*args*/ )
	: KopeteProtocol( GroupWiseProtocolFactory::instance(), parent, name ),
	  groupwiseOnline(  KopeteOnlineStatus::Online, 25, this, 0,  QString::null,  i18n( "Go O&nline" ),   i18n( "Online" ) ),
	  groupwiseAway(  KopeteOnlineStatus::Away, 25, this, 1, "msn_away",  i18n( "Go &Away" ),   i18n( "Away" ) ),
	  groupwiseOffline(  KopeteOnlineStatus::Offline, 25, this, 2,  QString::null,  i18n( "Go O&ffline" ),   i18n( "Offline" ) )

{
	kdDebug( 14210 ) << k_funcinfo << endl;

	s_protocol = this;
}

GroupWiseProtocol::~GroupWiseProtocol()
{
}

KopeteContact *GroupWiseProtocol::deserializeContact(
	KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString contactId = serializedData[ "contactId" ];
	QString accountId = serializedData[ "accountId" ];
	QString displayName = serializedData[ "displayName" ];
	QString type = serializedData[ "contactType" ];

	GroupWiseContact::GroupWiseContactType tbcType;
	if ( type == QString::fromLatin1( "echo" ) )
		tbcType = GroupWiseContact::Echo;
	if ( type == QString::fromLatin1( "null" ) )
		tbcType = GroupWiseContact::Null;
	else
		tbcType = GroupWiseContact::Null;

	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );

	KopeteAccount *account = accounts[ accountId ];
	if ( !account )
	{
		kdDebug(14210) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	return new GroupWiseContact(account, contactId, tbcType, displayName, metaContact);
}

AddContactPage * GroupWiseProtocol::createAddContactWidget( QWidget *parent, KopeteAccount * /* account */ )
{
	kdDebug( 14210 ) << "Creating Add Contact Page" << endl;
	return new GroupWiseAddContactPage( parent );
}

KopeteEditAccountWidget * GroupWiseProtocol::createEditAccountWidget( KopeteAccount *account, QWidget *parent )
{
	kdDebug(14210) << "Creating Edit Account Page" << endl;
	return new GroupWiseEditAccountWidget( parent, account );
}

KopeteAccount *GroupWiseProtocol::createNewAccount( const QString &accountId )
{
	return new GroupWiseAccount( this, accountId );
}

GroupWiseProtocol *GroupWiseProtocol::protocol()
{
	return s_protocol;
}



#include "gwprotocol.moc"
