/*
    aimpresence.cpp  -  AIM online status and presence management

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2006      by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <QHash>
#include <QFlags>

#include <kdebug.h>
#include <klocale.h>
#include <kstaticdeleter.h>

#include <kopeteonlinestatus.h>
#include <kopeteonlinestatusmanager.h>

#include "aimprotocol.h"

#include "aimpresence.h"

namespace AIM
{

//BEGIN class PresenceOverlay

class PresenceOverlay
{
public:
	PresenceOverlay() : mFlags( Presence::None ) {}
	PresenceOverlay( Presence::Flags flags, QString name, QStringList icons )
		: mFlags(flags), mName(name), mIcons(icons) {}

	static PresenceOverlay createForFlags( Presence::Flags flags );

	Presence::Flags overlayFlag() const { return mFlags; }
	QString overlayName() const { return mName; }
	QStringList overlayIcons() const { return mIcons; }

	PresenceOverlay &operator+= ( const PresenceOverlay &other );

private:
	Presence::Flags mFlags;
	QString mName;
	QStringList mIcons;
};

PresenceOverlay PresenceOverlay::createForFlags( Presence::Flags flags )
{
	/**
	 * Here are defined all available overlays. We can define overlay for one Presence::Flags
	 * or for combination of Presence::Flags. If a flags argument in this function is combination
	 * of flags than a PresenceOverlay object is created from combination of defined overlays or
	 * if the combination is defined separately than we return that PresenceOverlay object.
	 */

	const int dataSize = 3;
	static const PresenceOverlay data[dataSize] =
	{
		PresenceOverlay( Presence::Invisible, i18n("Invisible"), QStringList(QString("contact_invisible_overlay")) ),
		PresenceOverlay( Presence::Wireless, i18n("Mobile"), QStringList(QString("contact_phone_overlay")) ),
		PresenceOverlay( Presence::ICQ, i18n("ICQ"), QStringList(QString("icq_overlay")) )
	};

	PresenceOverlay overlay;
	for ( int n = 0; n < dataSize; ++n )
	{
		if ( data[n].overlayFlag() == flags )
			return data[n];
		else if ( data[n].overlayFlag() & flags )
			overlay += data[n];
	}

	return overlay;
}

PresenceOverlay &PresenceOverlay::operator+=( const PresenceOverlay &other )
{
	mFlags |= other.mFlags;
	if ( mName.isEmpty() )
		mName = other.mName;
	else if ( !other.mName.isEmpty() )
		mName += QString( ", " ) + other.mName;

	mIcons << other.mIcons;
	return *this;
}

//END class PresenceOverlay


//BEGIN struct PresenceTypeData

typedef QList<Presence::Flags> FlagsList;
struct PresenceTypeData
{
	Presence::Type type;
	Kopete::OnlineStatus::StatusType onlineStatusType;
	unsigned long setFlag;
	unsigned long getFlag;
	QString caption;
	QString name;
	QStringList overlayIcons;
	Kopete::OnlineStatusManager::Categories categories;
	Kopete::OnlineStatusManager::Options options;
	FlagsList overlayFlagsList;

	static const PresenceTypeData *all();
	static const PresenceTypeData &forType( Presence::Type type );
	static const PresenceTypeData &forStatus( unsigned long status );
	static const PresenceTypeData &forOnlineStatusType( const Kopete::OnlineStatus::StatusType statusType );
};

const PresenceTypeData *PresenceTypeData::all()
{
	using namespace Kopete;
	using namespace AIM::StatusCode;
	/**
	 * The order here is important - this is the order the IS_XXX flags will be checked for in.
	 * That, in particular, means that NA, Occupied and DND must appear before Away, and that
	 * DND must appear before Occupied. Offline (testing all bits) must go first, and Online
	 * (testing no bits - will always match a status) must go last.
	 *
	 * Free For Chat is currently listed after Away, since if someone is Away + Free For Chat we
	 * want to show them as Away more than we want to show them FFC.
	 *
	 * OverlayFlagsList should contain all possible flags combinations that can occur in AIM protocol.
	 * If overlayFlagsList contains flag None than this KOS will be in account's context menu.
	 */
	static const PresenceTypeData data[] =
	{
		{ Presence::Offline, OnlineStatus::Offline, OFFLINE, OFFLINE, i18n( "O&ffline" ), i18n("Offline"), QStringList(), Kopete::OnlineStatusManager::Offline, 0, FlagsList() << Presence::None << Presence::ICQ << Presence::Invisible },

		{ Presence::DoNotDisturb, OnlineStatus::Away, SET_DND, IS_DND, i18n( "&Do Not Disturb" ), i18n("Do Not Disturb"), QStringList(QString("contact_busy_overlay")), Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage, FlagsList() << Presence::ICQ << (Presence::ICQ | Presence::Invisible) },

		{ Presence::Occupied, OnlineStatus::Away, SET_OCC, IS_OCC, i18n( "O&ccupied" ), i18n("Occupied"), QStringList(QString("contact_busy_overlay")), 0, Kopete::OnlineStatusManager::HasStatusMessage, FlagsList() << Presence::ICQ << (Presence::ICQ | Presence::Invisible) },

		{ Presence::NotAvailable, OnlineStatus::Away, SET_NA, IS_NA, i18n( "Not A&vailable" ), i18n("Not Available"), QStringList(QString("contact_xa_overlay")), Kopete::OnlineStatusManager::ExtendedAway, Kopete::OnlineStatusManager::HasStatusMessage, FlagsList() << Presence::ICQ << (Presence::ICQ | Presence::Invisible) },

		{ Presence::Away, OnlineStatus::Away, SET_AWAY, IS_AWAY, i18n( "&Away" ), i18n("Away"), QStringList(QString("contact_away_overlay")), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasStatusMessage, FlagsList() << Presence::None << Presence::Invisible << Presence::ICQ << (Presence::ICQ | Presence::Invisible) << Presence::Wireless << (Presence::Wireless | Presence::Invisible) },

		{ Presence::FreeForChat,  OnlineStatus::Online,  SET_FFC,  IS_FFC,  i18n( "&Free for Chat" ),  i18n("Free For Chat"), QStringList(QString("icq_ffc")), Kopete::OnlineStatusManager::FreeForChat,  0, FlagsList() << Presence::ICQ << (Presence::ICQ | Presence::Invisible) },

		{ Presence::Online, OnlineStatus::Online, ONLINE, ONLINE, i18n( "O&nline" ), i18n("Online"), QStringList(), Kopete::OnlineStatusManager::Online, 0, FlagsList() << Presence::None << Presence::Invisible << Presence::ICQ << (Presence::ICQ | Presence::Invisible) << Presence::Wireless << (Presence::Wireless | Presence::Invisible) }
	};
	return data;
}

const PresenceTypeData &PresenceTypeData::forType( Presence::Type type )
{
	const PresenceTypeData *array = all();
	for ( uint n = 0; n < Presence::TypeCount; ++n )
		if ( array[n].type == type )
			return array[n];
	kWarning(14153) << k_funcinfo << "type " << (int)type << " not found! Returning Offline" << endl;
	return array[0];
}

const PresenceTypeData &PresenceTypeData::forStatus( unsigned long status )
{
	const PresenceTypeData *array = all();
	for ( uint n = 0; n < Presence::TypeCount; ++n )
	{
		if ( (array[n].getFlag & status) == array[n].getFlag )
			return array[n];
	}
	kWarning(14153) << k_funcinfo << "status " << (int)status << " not found! Returning Offline. This should not happen." << endl;
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
	kWarning(14153) << k_funcinfo << "online status " << (int)statusType << " not found! Returning Offline. This should not happen." << endl;
	return array[0];
}

//END struct PresenceTypeData

//BEGIN class OnlineStatusManager

class OnlineStatusManager::Private
{
public:
	typedef QHash<int, Kopete::OnlineStatus> StatusHash;
	
	// connecting and unknown should have the same internal status as offline, so converting to a Presence gives an Offline one
	Private()
		: connecting(     Kopete::OnlineStatus::Connecting, 99, AIMProtocol::protocol(),
					      99,                QStringList(QString("aim_connecting")), i18n("Connecting...") )
		, unknown(        Kopete::OnlineStatus::Unknown,     0, AIMProtocol::protocol(),
					      Presence::Offline, QStringList(QString("status_unknown")), i18n("Unknown") )
		, waitingForAuth( Kopete::OnlineStatus::Unknown,     1, AIMProtocol::protocol(),
				          Presence::Offline, QStringList(QString("button_cancel")),  i18n("Waiting for Authorization") )
		, invisible(      Kopete::OnlineStatus::Invisible,   2, AIMProtocol::protocol(),
						  Presence::Offline, QStringList(),    QString(),
						  QString(), Kopete::OnlineStatusManager::Invisible,
						  Kopete::OnlineStatusManager::HideFromMenu )

	{
		//weight 0, 1 and 2 are used by KOS unknown, waitingForAuth and invisible
		const uint firstUsableWeight = 3;
		for ( uint i = 0; i < Presence::TypeCount; ++i )
		{
			const PresenceTypeData &data = PresenceTypeData::forType( static_cast<Presence::Type>(i) );
			const uint weight = i + firstUsableWeight;
			for ( int j = 0; j < data.overlayFlagsList.count(); ++j )
			{
				const uint internalStatus = data.overlayFlagsList.at(j) | data.type;

				Kopete::OnlineStatus status;
				if ( data.overlayFlagsList.at(j) != Presence::None )
				{
					PresenceOverlay overlay = PresenceOverlay::createForFlags( data.overlayFlagsList.at(j) );
					//don't add KOS to account's context menu
					status = Kopete::OnlineStatus( data.onlineStatusType, weight,
					                               AIMProtocol::protocol(), internalStatus,
					                               data.overlayIcons + overlay.overlayIcons(),
					                               data.name + QString(" (%1)").arg( overlay.overlayName() ) );
				}
				else
				{
					//add KOS
					status = Kopete::OnlineStatus( data.onlineStatusType, weight,
					                               AIMProtocol::protocol(), internalStatus,
					                               data.overlayIcons, data.name,
					                               data.caption, data.categories, data.options );
				}
				statusHash[internalStatus] = status;
			}
		}
	}

	StatusHash statusHash;
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

Kopete::OnlineStatus OnlineStatusManager::onlineStatusOf( const Presence &presence )
{
	if ( d->statusHash.contains( presence.internalStatus() ) )
	{
		return d->statusHash.value( presence.internalStatus() );
	}
	else if ( d->statusHash.contains( presence.type() ) )
	{
		kWarning() << k_funcinfo << "Kopete::OnlineStatus doesn't exists for internal status" << presence.internalStatus()
		           << "Using basic status for type" << presence.type() << endl;
		return d->statusHash.value( presence.type() );
	}
	else
	{
		kWarning() << k_funcinfo << "Kopete::OnlineStatus doesn't exists for internal status" << presence.internalStatus() << endl;
		return d->unknown;
	}
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

Presence::Presence( Type type, Flags flags )
{
	_internalStatus = type | flags;
}

Presence::Presence( uint internalStatus )
{
	_internalStatus = internalStatus;
}

Presence Presence::fromOnlineStatus( const Kopete::OnlineStatus &status )
{
	if ( status.protocol() == AIMProtocol::protocol() )
	{
		return Presence( status.internalStatus() );
	}
	else
	{
		//status is a libkopete builtin status object
		//don't even think about converting it to AIM::Presence using presenceOf!
		return Presence( PresenceTypeData::forOnlineStatusType( status.status() ).type,
						 Presence::None );
	}
}

Kopete::OnlineStatus Presence::toOnlineStatus() const
{
	OnlineStatusManager *store = AIMProtocol::protocol()->statusManager();
	return store->onlineStatusOf( *this );
}


unsigned long Presence::toOscarStatus() const
{
	unsigned long basicStatus = basicOscarStatus();
	if ( (_internalStatus & Invisible) == Invisible )
		basicStatus |= StatusCode::INVISIBLE;
	return basicStatus;
}

Presence Presence::fromOscarStatus( unsigned long oStatus, int oClass )
{
	Type type = typeFromOscarStatus( oStatus );

	//Hack for aim away contacts
	if ( type == Online && (oClass & ClassCode::AWAY) == ClassCode::AWAY )
		type = Away;

	Flags flags = None;	
	if ( ( oClass & ClassCode::ICQ) == ClassCode::ICQ )
		flags |= ICQ;

	if ( ( oClass & ClassCode::WIRELESS) == ClassCode::WIRELESS )
		flags |= Wireless;

	if ( ( oStatus & StatusCode::INVISIBLE) == StatusCode::INVISIBLE )
		flags |= Invisible;

	return Presence( type, flags );
}

unsigned long Presence::basicOscarStatus() const
{
	const PresenceTypeData &data = PresenceTypeData::forType( type() );
	return data.setFlag;
}

Presence::Type Presence::typeFromOscarStatus( unsigned long status )
{
	const PresenceTypeData &data = PresenceTypeData::forStatus( status & 0xff );
	return data.type;
}

//END class Presence

} // end namespace AIM
