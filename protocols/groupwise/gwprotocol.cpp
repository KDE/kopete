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
#include "kopeteglobal.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwcontact.h"
#include "gwprotocol.h"
#include "ui/gwaddcontactpage.h"
#include "ui/gweditaccountwidget.h"

typedef KGenericFactory<GroupWiseProtocol> GroupWiseProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_groupwise, GroupWiseProtocolFactory( "kopete_groupwise" )  )

GroupWiseProtocol *GroupWiseProtocol::s_protocol = 0L;

GroupWiseProtocol::GroupWiseProtocol( QObject* parent, const char *name, const QStringList &/*args*/ )
	: KopeteProtocol( GroupWiseProtocolFactory::instance(), parent, name ),
	  groupwiseUnknown    ( KopeteOnlineStatus::Unknown, 25, this, 0, "status_unknown",
	  		"FIXME: Make this unselectable", i18n( "Unknown" ) ),
	  groupwiseOffline ( KopeteOnlineStatus::Offline,    0,  this, 1, QString::null, 
	  		i18n( "O&ffline" ), i18n( "Offline" ) ),
	  groupwiseAvailable  ( KopeteOnlineStatus::Online,  25, this, 2, QString::null, 
	  		i18n( "A&vailable" ),   i18n( "Available" ) ),
	  groupwiseBusy       ( KopeteOnlineStatus::Away,    20, this, 3, "msn_busy", 
	  		i18n( "&Busy" ), i18n( "Busy" ) ),
	  groupwiseAway       ( KopeteOnlineStatus::Away,    18, this, 4, "msn_away", 
	  		i18n( "Go &Away" ), i18n( "Away" ) ),
	  groupwiseAwayIdle   ( KopeteOnlineStatus::Away,    15, this, 5, "msn_away", 
	  		"FIXME: Make this unselectable", i18n( "Idle" ) ),
	  groupwiseInvalid( KopeteOnlineStatus::Unknown, 25, this, 6, "status_unknown",
	  		"FIXME: Make this unselectable", i18n( "Invalid Status" ) ),
	  groupwiseConnecting( KopeteOnlineStatus::Unknown, 0, this, 99, "status_connecting",
	  		"FIXME: Make this unselectable", i18n( "Connecting" ) ),
	  propGivenName( Kopete::Global::Properties::self()->firstName() ),
	  propLastName( Kopete::Global::Properties::self()->lastName() ),
	  propFullName( Kopete::Global::Properties::self()->fullName() ),
	  propAwayMessage( Kopete::Global::Properties::self()->awayMessage() ),
	  propAutoReply( "groupwiseAutoReply", i18n( "Auto Reply Message" ), QString::null, false, false ),
	  propCN( "groupwiseCommonName", i18n( "Common Name" ), QString::null, true, false )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

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
	int objectId = serializedData[ "objectId" ].toInt();
	int parentId = serializedData[ "parentId" ].toInt();
	int sequence = serializedData[ "sequenceNumber" ].toInt();
	
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );

	KopeteAccount *account = accounts[ accountId ];
	if ( !account )
	{
		kdDebug(GROUPWISE_DEBUG_GLOBAL) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	return new GroupWiseContact(account, contactId, metaContact, displayName, objectId, parentId, sequence );
}

AddContactPage * GroupWiseProtocol::createAddContactWidget( QWidget *parent, KopeteAccount * /* account */ )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "Creating Add Contact Page" << endl;
	return new GroupWiseAddContactPage( parent );
}

KopeteEditAccountWidget * GroupWiseProtocol::createEditAccountWidget( KopeteAccount *account, QWidget *parent )
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << "Creating Edit Account Page" << endl;
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

KopeteOnlineStatus GroupWiseProtocol::gwStatusToKOS( const int gwInternal )
{
	KopeteOnlineStatus status;
	switch ( gwInternal )
	{
		case GroupWise::Unknown:
			status = groupwiseUnknown;
			break;
		case GroupWise::Offline:
			status = groupwiseOffline;
			break;
		case GroupWise::Available:
			status = groupwiseAvailable;
			break;
		case GroupWise::Busy:
			status = groupwiseBusy;
			break;
		case GroupWise::Away:
			status = groupwiseAway;
			break;
		case GroupWise::AwayIdle:
			status = groupwiseAwayIdle;
			break;
		case GroupWise::Invalid:
			status = groupwiseInvalid;
			break;
		default:
			status = groupwiseInvalid;
			kdWarning( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got unrecognised status value" << gwInternal << endl;
	}
	return status;
}


#include "gwprotocol.moc"
