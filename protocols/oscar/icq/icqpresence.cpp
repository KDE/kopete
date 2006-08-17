/*
    icqpresence.cpp  -  ICQ online status and presence management
    
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <utility>
#include <vector>

#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>

#include <kopeteonlinestatus.h>
#include <kopeteonlinestatusmanager.h>

#include "icqprotocol.h"

#include "icqpresence.h"

namespace ICQ
{

//BEGIN struct PresenceTypeData

struct PresenceTypeData
{
	Presence::Type type;
	Kopete::OnlineStatus::StatusType onlineStatusType;
	unsigned long setFlag;
	unsigned long getFlag;
	QString caption;
	QString visibleName;
	QString invisibleName;
	const char *visibleIcon;
	const char *invisibleIcon;
	unsigned int categories;
	unsigned int options;

	static const PresenceTypeData *all();
	static const PresenceTypeData &forType( Presence::Type type );
	static const PresenceTypeData &forStatus( unsigned long status );
	static const PresenceTypeData &forOnlineStatusType( const Kopete::OnlineStatus::StatusType statusType );
};

const PresenceTypeData *PresenceTypeData::all()
{
	using namespace Kopete;
	using namespace ICQ::StatusCode;
	/**
	 * The order here is important - this is the order the IS_XXX flags will be checked for in.
	 * That, in particular, means that NA, Occupied and DND must appear before Away, and that
	 * DND must appear before Occupied. Offline (testing all bits) must go first, and Online
	 * (testing no bits - will always match a status) must go last.
	 * 
	 * Free For Chat is currently listed after Away, since if someone is Away + Free For Chat we
	 * want to show them as Away more than we want to show them FFC.
	 */
	static const PresenceTypeData data[] =
	{
		{ Presence::Offline,      OnlineStatus::Offline, OFFLINE,  OFFLINE, i18n( "O&ffline" ),        i18n("Offline"),        i18n("Offline"),                    0,                      "contact_invisible_overlay", Kopete::OnlineStatusManager::Offline,      0 },
		{ Presence::DoNotDisturb, OnlineStatus::Away,    SET_DND,  IS_DND,  i18n( "&Do Not Disturb" ), i18n("Do Not Disturb"), i18n("Do Not Disturb (Invisible)"), "contact_busy_overlay", "contact_invisible_overlay", Kopete::OnlineStatusManager::Busy,         Kopete::OnlineStatusManager::HasAwayMessage },
		{ Presence::Occupied,     OnlineStatus::Away,    SET_OCC,  IS_OCC,  i18n( "O&ccupied" ),       i18n("Occupied"),       i18n("Occupied (Invisible)"),       "contact_busy_overlay", "contact_invisible_overlay", 0,                                         Kopete::OnlineStatusManager::HasAwayMessage },
		{ Presence::NotAvailable, OnlineStatus::Away,    SET_NA,   IS_NA,   i18n( "Not A&vailable" ),  i18n("Not Available"),  i18n("Not Available (Invisible)"),  "contact_xa_overlay",   "contact_invisible_overlay", Kopete::OnlineStatusManager::ExtendedAway, Kopete::OnlineStatusManager::HasAwayMessage },
		{ Presence::Away,         OnlineStatus::Away,    SET_AWAY, IS_AWAY, i18n( "&Away" ),           i18n("Away"),           i18n("Away (Invisible)"),           "contact_away_overlay", "contact_invisible_overlay", Kopete::OnlineStatusManager::Away,         Kopete::OnlineStatusManager::HasAwayMessage },
		{ Presence::FreeForChat,  OnlineStatus::Online,  SET_FFC,  IS_FFC,  i18n( "&Free for Chat" ),  i18n("Free For Chat"),  i18n("Free For Chat (Invisible)"),  "icq_ffc",              "contact_invisible_overlay", Kopete::OnlineStatusManager::FreeForChat,  0 },
		{ Presence::Online,       OnlineStatus::Online,  ONLINE,   ONLINE,  i18n( "O&nline" ),         i18n("Online"),         i18n("Online (Invisible)"),         0,                      "contact_invisible_overlay", Kopete::OnlineStatusManager::Online,       0 }
	};
	return data;
}

const PresenceTypeData &PresenceTypeData::forType( Presence::Type type )
{
	const PresenceTypeData *array = all();
	for ( uint n = 0; n < Presence::TypeCount; ++n )
		if ( array[n].type == type )
			return array[n];
	kdWarning(14153) << k_funcinfo << "type " << (int)type << " not found! Returning Offline" << endl;
	return array[0];
}

const PresenceTypeData &PresenceTypeData::forStatus( unsigned long status )
{
	const PresenceTypeData *array = all();
	for ( uint n = 0; n < Presence::TypeCount; ++n )
	{
		//kdDebug(14153) << k_funcinfo << "array[n].getFlag is " << array[n].getFlag << ", status is " << status << ", & is " << (array[n].getFlag & status) << endl;
		if ( (array[n].getFlag & status) == array[n].getFlag )
			return array[n];
	}
	kdWarning(14153) << k_funcinfo << "status " << (int)status << " not found! Returning Offline. This should not happen." << endl;
	return array[0];
}

const PresenceTypeData &PresenceTypeData::forOnlineStatusType( const Kopete::OnlineStatus::StatusType statusType )
{
	const PresenceTypeData *array = all();
	for ( int n = Presence::TypeCount - 1; n >= 0; --n )
	{
		if ( array[n].onlineStatusType == statusType )
			return array[n];
	}
	kdWarning(14153) << k_funcinfo << "online status " << (int)statusType << " not found! Returning Offline. This should not happen." << endl;
	return array[0];
}

//END struct PresenceTypeData

//BEGIN class OnlineStatusManager

class OnlineStatusManager::Private
{
public:
	typedef std::vector<Kopete::OnlineStatus> StatusList;
	
	// connecting and unknown should have the same internal status as offline, so converting to a Presence gives an Offline one
	Private()
		: connecting(     Kopete::OnlineStatus::Connecting, 99, ICQProtocol::protocol(),
					      99,                "icq_connecting", i18n("Connecting...") )
		, unknown(        Kopete::OnlineStatus::Unknown,     0, ICQProtocol::protocol(),
					      Presence::Offline, "status_unknown", i18n("Unknown") )
		, waitingForAuth( Kopete::OnlineStatus::Unknown,     1, ICQProtocol::protocol(),
				          Presence::Offline, "button_cancel",  i18n("Waiting for Authorization") )
		, invisible(      Kopete::OnlineStatus::Invisible,   2, ICQProtocol::protocol(),
						  Presence::Offline, QString::null,    QString::null,
						  QString::null, Kopete::OnlineStatusManager::Invisible,
						  Kopete::OnlineStatusManager::HideFromMenu )

	{
		createStatusList( false, 0, visibleStatusList );
		createStatusList( true, Presence::TypeCount, invisibleStatusList );
	}
	void createStatusList( bool invisible, const uint invisibleOffset, StatusList &statusList )
	{
		//weight 0, 1 and 2 are used by KOS unknown, waitingForAuth and invisible
		const uint firstUsableWeight = 3;
		statusList.reserve( Presence::TypeCount );
		for ( uint n = 0; n < Presence::TypeCount; ++n )
		{
			const PresenceTypeData &data = PresenceTypeData::forType( static_cast<Presence::Type>(n) );
			const uint weight = n + firstUsableWeight;
			const uint internalStatus = n + invisibleOffset;
			QStringList overlayIcons( data.visibleIcon );
			QString description( data.visibleName );
			Kopete::OnlineStatus status;
			if ( invisible )
			{
				overlayIcons << data.invisibleIcon;
				description = data.invisibleName;
				//don't add invisible KOS to account's context menu
				status = Kopete::OnlineStatus( data.onlineStatusType, weight,
											   ICQProtocol::protocol(), internalStatus,
											   overlayIcons, description );
			}
			else
			{
				//add visible KOS
				status = Kopete::OnlineStatus( data.onlineStatusType, weight,
											   ICQProtocol::protocol(), internalStatus,
											   overlayIcons, description,
											   data.caption, data.categories, data.options );
			}
			statusList.push_back( status );
		}
	}
	
	StatusList visibleStatusList, invisibleStatusList;
	Kopete::OnlineStatus connecting;
	Kopete::OnlineStatus unknown;
	Kopete::OnlineStatus waitingForAuth;
	Kopete::OnlineStatus invisible;
};

OnlineStatusManager::OnlineStatusManager()
	: d( new Private )
{
}

OnlineStatusManager::~OnlineStatusManager()
{
	delete d;
}

Presence OnlineStatusManager::presenceOf( uint internalStatus )
{
	if ( internalStatus < Presence::TypeCount )
	{
		return Presence( static_cast<Presence::Type>( internalStatus ), Presence::Visible );
	}
	else if ( internalStatus < 2 * Presence::TypeCount )
	{
		return Presence( static_cast<Presence::Type>( internalStatus - Presence::TypeCount ), Presence::Invisible );
	}
	else
	{
		kdWarning(14153) << k_funcinfo << "No presence exists for internal status " << internalStatus << "! Returning Offline" << endl;
		return Presence( Presence::Offline, Presence::Visible );
	}	
}

Kopete::OnlineStatus OnlineStatusManager::onlineStatusOf( const Presence &presence )
{
	if ( presence.visibility() == Presence::Visible )
		return d->visibleStatusList[ presence.type() ];
	else
		return d->invisibleStatusList[ presence.type() ];
}

Kopete::OnlineStatus OnlineStatusManager::connectingStatus()
{
	return d->connecting;
}

Kopete::OnlineStatus OnlineStatusManager::unknownStatus()
{
	return d->unknown;
}

Kopete::OnlineStatus OnlineStatusManager::waitingForAuth()
{
	return d->waitingForAuth;
}

//END class OnlineStatusManager

//BEGIN class Presence

Presence Presence::fromOnlineStatus( const Kopete::OnlineStatus &status )
{
	if ( status.protocol() == ICQProtocol::protocol() )
	{
		OnlineStatusManager *store = ICQProtocol::protocol()->statusManager();
		return store->presenceOf( status.internalStatus() );
	}
	else
	{
		//status is a libkopete builtin status object
		//don't even think about converting it to ICQ::Presence using presenceOf!
		return Presence( PresenceTypeData::forOnlineStatusType( status.status() ).type,
						 Presence::Visible );
	}
}

Kopete::OnlineStatus Presence::toOnlineStatus() const
{
	OnlineStatusManager *store = ICQProtocol::protocol()->statusManager();
	return store->onlineStatusOf( *this );
}


unsigned long Presence::toOscarStatus() const
{
	unsigned long basicStatus = basicOscarStatus();
	if ( _visibility == Invisible )
		basicStatus |= StatusCode::INVISIBLE;
	return basicStatus;
}

Presence Presence::fromOscarStatus( unsigned long code )
{
	Type type = typeFromOscarStatus( code & ~StatusCode::INVISIBLE );
	bool invisible = (code & StatusCode::INVISIBLE) == StatusCode::INVISIBLE;
	return Presence( type, invisible ? Invisible : Visible );
}


unsigned long Presence::basicOscarStatus() const
{
	const PresenceTypeData &data = PresenceTypeData::forType( _type );
	return data.setFlag;
}

Presence::Type Presence::typeFromOscarStatus( unsigned long status )
{
	const PresenceTypeData &data = PresenceTypeData::forStatus( status );
	return data.type;
}

//END class Presence

} // end namespace ICQ

// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode: csands; space-indent off; tab-width 4;
