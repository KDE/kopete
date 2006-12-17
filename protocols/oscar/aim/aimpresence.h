/*
    aimpresence.h  -  AIM online status and presence management

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


#ifndef AIMPRESENCE_H
#define AIMPRESENCE_H

#include <kdebug.h>

namespace Kopete { class OnlineStatus; }

namespace AIM
{

class Presence;

/**
 * @brief This namespace contains status flags used in OSCAR's on-the-wire format.
 * 
 * The IS_XXX values are bits representing actual status flags. However, the flags
 * are just that -- flags.  AIM statuses are represented by a combination of these
 * flags rather than just one value.  This seems to be for backwards compatibility
 * reasons -- this way you can add a new status and existing clients should still
 * work correctly.
 * 
 * So, when changing status you need to specify not only what status it is, but
 * also all other status flags that are appropriate. The SET_XXX flags do just that;
 * SET_DND for instance sets the DND, Occupied and Away bits.
 */
namespace StatusCode
{
	enum
	{
		OFFLINE    = 0xFFFFFFFF,
		ONLINE     = 0x00000000,
		INVISIBLE  = 0x00000100,
		
		IS_DND     = 0x00000002, ///< Do Not Disturb
		IS_OCC     = 0x00000010, ///< Occupied
		IS_NA      = 0x00000004, ///< Not Available
		IS_AWAY    = 0x00000001, ///< Away
		IS_FFC     = 0x00000020, ///< Free For Chat
		
		SET_DND    = 0x00000013, //== DND + Occupied + Away
		SET_OCC    = 0x00000011, //== Occupied + Away
		SET_NA     = 0x00000005, //== NA + Away
		SET_AWAY   = 0x00000001,
		SET_FFC    = 0x00000020,
		
		WEBAWARE   = 0x00010000,
		SHOWIP     = 0x00020000
	};
} // end namespace StatusCode

namespace ClassCode
{
	enum
	{
			AWAY       = 0x0020,
			ICQ        = 0x0040,
			WIRELESS   = 0x0080,
	};
} // end namespace ClassCode

/**
 * @brief A manager for AIM's online statuses
 * 
 * Looks after AIM's numerous online statuses, and maps between them and Presence objects.
 * A single instance of this class is held by the AIMProtocol object.
 */
class OnlineStatusManager
{
public:
	OnlineStatusManager();
	~OnlineStatusManager();

	Kopete::OnlineStatus onlineStatusOf( const AIM::Presence &presence );
	Kopete::OnlineStatus connectingStatus();
	Kopete::OnlineStatus unknownStatus();
	Kopete::OnlineStatus waitingForAuth();

private:
	class Private;
	Private *d;
};

/**
 * @brief An AIM online presence object
 */
class Presence
{
public:
	/**
	 * Friendly types this status can be
	 */
	enum Type { Offline = 0x000, DoNotDisturb = 0x001, Occupied = 0x002,
	            NotAvailable = 0x003, Away = 0x004, FreeForChat = 0x005, Online = 0x006 };
	enum { TypeCount = Online + 1 };

	enum Flag { None = 0x000, ICQ = 0x010, Wireless = 0x100, Invisible = 0x200 };
	Q_DECLARE_FLAGS(Flags, Flag)

	Presence( Type type, Flags flags = None );

	Type type() const { return (Type)(_internalStatus & 0x0000000F); }
	Flags flags() const { return (Flags)(_internalStatus & 0xFFFFFFF0); }
	uint internalStatus() const { return _internalStatus; }

	/**
	 * Generate a Presence object from an online status
	 */
	static Presence fromOnlineStatus( const Kopete::OnlineStatus &status );

	/**
	 * Convert this Presence object to an online status
	 */
	Kopete::OnlineStatus toOnlineStatus() const;

	/**
	 * Get the status code to pass to liboscar to set us to this Status.
	 * @note This is not the opposite of fromOnlineStatus(). The set and get codes don't match.
	 */
	unsigned long toOscarStatus() const;

	/**
	 * Get the status a contact is at based on liboscar's view of its status.
	 * @note This is not the opposite of toOnlineStatus().
	 */
	static Presence fromOscarStatus( unsigned long oStatus, int oClass );

	bool operator==( const Presence &other ) const { return other._internalStatus == _internalStatus; }
	bool operator!=( const Presence &other ) const { return !(*this == other); }
	
private:
	Presence( uint internalStatus );

	unsigned long basicOscarStatus() const;
	static Type typeFromOscarStatus( unsigned long status );
private:
	uint _internalStatus;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Presence::Flags)
}

#endif
