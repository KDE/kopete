// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//////////////////////////////////////////////////////////////////////////////
// gaduprotocol.cpp																													//
//																																					//
// Copyright (C)	2002	Zack Rusin <zack@kde.org>														//
//																																					//
// This program is free software; you can redistribute it and/or						//
// modify it under the terms of the GNU General Public License							//
// as published by the Free Software Foundation; either version 2						//
// of the License, or (at your option) any later version.										//
//																																					//
// This program is distributed in the hope that it will be useful,					//
// but WITHOUT ANY WARRANTY; without even the implied warranty of						//
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the						//
// GNU General Public License for more details.															//
//																																					//
// You should have received a copy of the GNU General Public License				//
// along with this program; if not, write to the Free Software							//
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA								//
// 02111-1307, USA.																													//
//////////////////////////////////////////////////////////////////////////////

#include <kdebug.h>
#include <kgenericfactory.h>


#include "gaduprotocol.h"
#include "gaduaccount.h"
#include "gadueditaccount.h"
#include "gaduaddcontactpage.h"
#include "gadupreferences.h"

#include <libgadu.h>

K_EXPORT_COMPONENT_FACTORY( kopete_gadu, KGenericFactory<GaduProtocol> );

GaduProtocol* GaduProtocol::protocolStatic_ = 0L;

GaduProtocol::GaduProtocol( QObject* parent, const char* name, const QStringList & )
	:		KopeteProtocol( parent, name ),
			gaduStatusOffline_(				KopeteOnlineStatus::Offline, 0, this, GG_STATUS_NOT_AVAIL,
																"gg_offline", i18n( "Go O&ffline" ),	 i18n( "Offline" ) ),
			gaduStatusOfflineDescr_(	KopeteOnlineStatus::Away,		 2, this, GG_STATUS_NOT_AVAIL_DESCR,
																"gg_offline_d",		 i18n( "Go A&way" ),			i18n( "Offline With Description" ) ),
			gaduStatusMaybeOffline_( KopeteOnlineStatus::Offline, 4, this, 0x0,
															 "gg_away", i18n( "Go O&ffline" ),	 i18n( "Offline (maybe)" ) ),
			gaduStatusBusy_(					KopeteOnlineStatus::Away,		 20, this, GG_STATUS_BUSY,
																"gg_busy",		i18n( "Go B&usy" ),			 i18n( "Busy" ) ),
			gaduStatusBusyDescr_(			KopeteOnlineStatus::Away,		 25, this, GG_STATUS_BUSY_DESCR,
																"gg_busy_d",		i18n( "Go B&usy" ),			 i18n( "Busy With Description" ) ),
			gaduStatusInvisible_(			KopeteOnlineStatus::Away,			5, this, GG_STATUS_INVISIBLE,
																"gg_invi",		i18n( "Go I&nvisible" ), i18n( "Invisible" ) ),
			gaduStatusInvisibleDescr_(KopeteOnlineStatus::Away,		 10, this, GG_STATUS_INVISIBLE_DESCR,
																"gg_invi_d",		i18n( "Go I&nvisible" ), i18n( "Invisible With Description" ) ),
			gaduStatusAvail_(					KopeteOnlineStatus::Online,	 30, this, GG_STATUS_AVAIL,
																"gg_online",	i18n( "Go &Online" ),		 i18n( "Online" ) ),
			gaduStatusAvailDescr_(		KopeteOnlineStatus::Online,	 30, this, GG_STATUS_AVAIL_DESCR,
																"gg_online_d",	i18n( "Go &Online" ),		 i18n( "Online With Description" ) ),
			defaultAccount_( 0 )
{
	if ( protocolStatic_ )
		kdDebug(14100)<<"####"<<"GaduProtocol already initialized"<<endl;
	else
		protocolStatic_ = this;

	prefs_ = new GaduPreferences( "gadu_protocol", this );
	QObject::connect( prefs_, SIGNAL(saved()), this, SLOT(settingsChanged()) );

	addAddressBookField( "messaging/gadu", KopetePlugin::MakeIndexField );
}

GaduProtocol::~GaduProtocol()
{
	protocolStatic_ = 0L;
}

GaduProtocol*
GaduProtocol::protocol()
{
	return protocolStatic_;
}

AddContactPage*
GaduProtocol::createAddContactWidget( QWidget* parent, KopeteAccount* account )
{
	return new GaduAddContactPage( static_cast<GaduAccount*>( account ), parent );
}

void
GaduProtocol::settingsChanged()
{

}

void
GaduProtocol::deserializeContact( KopeteMetaContact *metaContact,
																	const QMap<QString, QString> &serializedData,
																	const QMap<QString, QString> & /* addressBookData */ )
{
	kdDebug(14100)<<"Adding "<<serializedData[ "contactId" ]<<" || "<< serializedData[ "displayName" ] <<endl;
	if ( defaultAccount_ )
		defaultAccount_->addContact( serializedData[ "contactId" ], serializedData[ "displayName" ], metaContact );
	else
		kdWarning(14100)<<"CONTACTS ARE BEING DESERIALIZED BEFORE AN ACCOUNT!!!"<<endl;
}

KopeteOnlineStatus
GaduProtocol::convertStatus( uint status ) const
{
	switch( status )
	{
	case GG_STATUS_NOT_AVAIL:
		return gaduStatusOffline_;
	case GG_STATUS_NOT_AVAIL_DESCR:
		return gaduStatusOfflineDescr_;
	case GG_STATUS_BUSY:
		return gaduStatusBusy_;
	case GG_STATUS_BUSY_DESCR:
		return gaduStatusBusyDescr_;
	case GG_STATUS_INVISIBLE:
		return gaduStatusInvisible_;
	case GG_STATUS_INVISIBLE_DESCR:
		return gaduStatusInvisibleDescr_;
	case GG_STATUS_AVAIL:
		return gaduStatusAvail_;
	case GG_STATUS_AVAIL_DESCR:
		return gaduStatusAvailDescr_;
	default:
		return gaduStatusOffline_;
	}
}

KopeteAccount*
GaduProtocol::createNewAccount( const QString& accountId )
{
	GaduAccount *account = new GaduAccount( this, accountId );
	defaultAccount_ = account;
	return account;
}

EditAccountWidget*
GaduProtocol::createEditAccountWidget( KopeteAccount *account, QWidget *parent )
{
	return	(new GaduEditAccount( this, account, parent ) );
}

#include "gaduprotocol.moc"
