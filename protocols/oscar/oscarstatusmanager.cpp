/*
    oscarstatusmanager.cpp  -  Oscar status manager
    
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2006,2007 by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "oscarstatusmanager.h"

#include <QHash>

#include <kdebug.h>

#include "oscarprotocol.h"
#include "oscarpresencesdataclasses.h"

class OscarStatusManager::Private
{
public:
	OscarProtocol* protocol;

	typedef QHash<int, Kopete::OnlineStatus> StatusHash;
	StatusHash statusHash;

	QList<Oscar::PresenceType> presenceTypeList;
	QList<Oscar::PresenceOverlay> presenceOverlayList;

	Oscar::Presence::Flags mask;
};

OscarStatusManager::OscarStatusManager( OscarProtocol* protocol )
	: d( new Private() )
{
	d->protocol = protocol;
	d->mask = ~(Oscar::Presence::Flags)Oscar::Presence::None;
}

OscarStatusManager::~OscarStatusManager()
{
	delete d;
}

void OscarStatusManager::initialize( uint firstUsableWeight )
{
	for ( uint i = 0; i < Oscar::Presence::TypeCount; ++i )
	{
		const Oscar::PresenceType &data =  pscTypeForType( static_cast<Oscar::Presence::Type>(i) );
		const uint weight = i + firstUsableWeight;
		for ( int j = 0; j < data.overlayFlagsList().count(); ++j )
		{
			const uint internalStatus = data.overlayFlagsList().at(j) | data.type();
			
			Kopete::OnlineStatus status;
			if ( data.overlayFlagsList().at(j) != Oscar::Presence::None )
			{
				Oscar::PresenceOverlay overlay = pscOverlayForFlags( data.overlayFlagsList().at(j) );
				//don't add KOS to account's context menu
				status = Kopete::OnlineStatus( data.onlineStatusType(), weight,
				                               d->protocol, internalStatus,
				                               data.overlayIcons() + overlay.icons(),
				                               data.name() + QString(" (%1)").arg( overlay.description() ) );
			}
			else
			{
				//add KOS
				status = Kopete::OnlineStatus( data.onlineStatusType(), weight,
				                               d->protocol, internalStatus,
				                               data.overlayIcons(), data.name(),
				                               data.caption(), data.categories(), data.options() );
			}
			d->statusHash[internalStatus] = status;
		}
	}
}

void OscarStatusManager::setPresenceType( const QList<Oscar::PresenceType>& list )
{
	d->presenceTypeList = list;
}

void OscarStatusManager::setPresenceOverlay( const QList<Oscar::PresenceOverlay>& list )
{
	d->presenceOverlayList = list;
}

void OscarStatusManager::setPresenceFlagsMask( Oscar::Presence::Flags mask )
{
	d->mask = mask;
}

Kopete::OnlineStatus OscarStatusManager::onlineStatusOf( const Oscar::Presence &presence ) const
{
	uint internalStatus = presence.internalStatus() & d->mask;
	if ( (internalStatus & Oscar::Presence::XStatus) == Oscar::Presence::XStatus )
	{
		kDebug() << k_funcinfo << "Creating Kopete::OnlineStatus for XStatus, internal status: " << internalStatus << endl;
		// XStatus, we have to create new KOS
		Oscar::PresenceOverlay overlay = pscOverlayForFlags( (Oscar::Presence::Flags)(internalStatus & Oscar::Presence::FlagsMask) );

		QString desc = presence.description();
		if ( !overlay.description().isEmpty() )
			desc += QString(" (%1)").arg( overlay.description() );

		QString xtrazIcon = QString( "icq_xstatus%1" ).arg( presence.xtrazStatus() );
		return Kopete::OnlineStatus( Kopete::OnlineStatus::Online, 0, d->protocol, internalStatus,
		                             QStringList( xtrazIcon ) + overlay.icons(), desc );
	}
	else
	{
		if ( d->statusHash.contains( internalStatus ) )
		{
			return d->statusHash.value( internalStatus );
		}
		else if ( d->statusHash.contains( presence.type() ) )
		{
			kWarning() << k_funcinfo << "Kopete::OnlineStatus doesn't exists for internal status " << internalStatus
					<< " Using basic status for type " << presence.type() << endl;
			return d->statusHash.value( presence.type() );
		}
		else
		{
			kWarning() << k_funcinfo << "Kopete::OnlineStatus doesn't exists for internal status " << internalStatus << endl;
			return unknownStatus();
		}
	}
}

Oscar::Presence OscarStatusManager::presenceOf( const Kopete::OnlineStatus &status ) const
{
	if ( status.protocol() == d->protocol )
	{
		Oscar::Presence presence( status.internalStatus() );
		if ( (presence.flags() & Oscar::Presence::XStatus) == Oscar::Presence::XStatus )
		{
			// XStatus, we have to filter out description
			Oscar::PresenceOverlay overlay = pscOverlayForFlags( presence.flags() );

			QString desc = status.description();
			if ( !overlay.description().isEmpty() )
				desc.remove( QString(" (%1)").arg( overlay.description() ) );

			presence.setDescription( desc );
		}
		return presence;
	}
	else
	{
		//status is a libkopete builtin status object
		//don't even think about converting it to Oscar::Presence using presenceOf!
		return Oscar::Presence( pscTypeForOnlineStatusType( status.status() ).type(),
		                        Oscar::Presence::None );
	}
}

unsigned long OscarStatusManager::oscarStatusOf( const Oscar::Presence &presence ) const
{
	unsigned long basicStatus = basicOscarStatus( presence.type() );
	if ( (presence.internalStatus() & Oscar::Presence::Invisible) == Oscar::Presence::Invisible )
		basicStatus |= Oscar::StatusCode::INVISIBLE;
	return basicStatus;
}

Oscar::Presence OscarStatusManager::presenceOf( unsigned long oStatus, int oClass ) const
{
	using namespace Oscar;
	Presence::Type type = pscTypeForOscarStatus( oStatus );

	//Hack for aim away contacts
	if ( type == Presence::Online && (oClass & ClassCode::AWAY) == ClassCode::AWAY )
		type = Presence::Away;

	Presence::Flags flags = Presence::None;
	if ( (oClass & ClassCode::ICQ) == ClassCode::ICQ )
		flags |= Presence::ICQ;
	else
		flags |= Presence::AIM;

	if ( (oClass & ClassCode::WIRELESS) == ClassCode::WIRELESS )
		flags |= Presence::Wireless;

	if ( (oStatus & StatusCode::INVISIBLE) == StatusCode::INVISIBLE )
		flags |= Presence::Invisible;

	return Presence( type, flags );
}

unsigned long OscarStatusManager::basicOscarStatus( Oscar::Presence::Type type ) const
{
	const Oscar::PresenceType &data = pscTypeForType( type );
	return data.setFlag();
}

Oscar::Presence::Type OscarStatusManager::pscTypeForOscarStatus( unsigned long status ) const
{
	const Oscar::PresenceType &data = pscTypeForStatus( status & 0xff );
	return data.type();
}

Oscar::PresenceOverlay OscarStatusManager::pscOverlayForFlags( Oscar::Presence::Flags flags ) const
{
	Oscar::PresenceOverlay overlay;
	int size = d->presenceOverlayList.size();
	for ( int i = 0; i < size; i++ )
	{
		Oscar::PresenceOverlay data = d->presenceOverlayList.at(i);
		if ( data.flags() == flags )
			return data;
		else if ( data.flags() & flags )
			overlay += data;
	}

	return overlay;
}

const Oscar::PresenceType &OscarStatusManager::pscTypeForType( Oscar::Presence::Type type ) const
{
	int size = d->presenceTypeList.size();
	for ( int n = 0; n < size; ++n )
	{
		if ( d->presenceTypeList.at(n).type() == type )
			return d->presenceTypeList.at(n);
	}

	kWarning(14153) << k_funcinfo << "type " << (int)type << " not found! Returning Offline" << endl;
	return d->presenceTypeList.at(0);
}

const Oscar::PresenceType &OscarStatusManager::pscTypeForStatus( unsigned long status ) const
{
	int size = d->presenceTypeList.size();
	for ( int n = 0; n < size; ++n )
	{
		if ( (d->presenceTypeList.at(n).getFlag() & status) == d->presenceTypeList.at(n).getFlag() )
			return d->presenceTypeList.at(n);
	}

	kWarning(14153) << k_funcinfo << "status " << (int)status << " not found! Returning Offline. This should not happen." << endl;
	return d->presenceTypeList.at(0);
}

const Oscar::PresenceType &OscarStatusManager::pscTypeForOnlineStatusType( const Kopete::OnlineStatus::StatusType statusType ) const
{
	int size = d->presenceTypeList.size();
	for ( int n = size - 1; n >= 0; --n )
	{
		if ( d->presenceTypeList.at(n).onlineStatusType() == statusType )
			return d->presenceTypeList.at(n);
	}

	kWarning(14153) << k_funcinfo << "online status " << (int)statusType << " not found! Returning Offline. This should not happen." << endl;
	return d->presenceTypeList.at(0);
}
