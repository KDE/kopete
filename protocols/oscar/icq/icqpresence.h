/*
    icqpresence.h  -  ICQ online status and presence management
    
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


#ifndef ICQCOMMON_H
#define ICQCOMMON_H

#include <kdebug.h>

namespace Kopete { class OnlineStatus; }

namespace ICQ
{

class Presence;

/**
 * @brief This namespace contains status flags used in OSCAR's on-the-wire format.
 * 
 * The IS_XXX values are bits representing actual status flags. However, the flags
 * are just that -- flags.  ICQ statuses are represented by a combination of these
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

/**
 * @brief A manager for ICQ's online statuses
 * 
 * Looks after ICQ's numerous online statuses, and maps between them and Presence objects.
 * A single instance of this class is held by the ICQProtocol object.
 */
class OnlineStatusManager
{
public:
	OnlineStatusManager();
	~OnlineStatusManager();
	ICQ::Presence presenceOf( uint internalStatus );
	Kopete::OnlineStatus onlineStatusOf( const ICQ::Presence &presence );
	Kopete::OnlineStatus connectingStatus();
	Kopete::OnlineStatus unknownStatus();
	Kopete::OnlineStatus waitingForAuth();
	
private:
	class Private;
	Private *d;
};

/**
 * @brief An ICQ online presence object
 */
class Presence
{
public:
	/**
	 * Friendly types this status can be
	 */
	enum Type { Offline, DoNotDisturb, Occupied, NotAvailable, Away, Online, FreeForChat };
	enum { TypeCount = FreeForChat + 1 };
	
	enum Visibility { Invisible, Visible };
	
	Presence( Type type, Visibility vis ) : _type(type), _visibility(vis) {}
	
	Type type() const { return _type; }
	Visibility visibility() const { return _visibility; }
	
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
	static Presence fromOscarStatus( unsigned long code );
	
	bool operator==( const Presence &other ) const { return other._type == _type && other._visibility == _visibility; }
	bool operator!=( const Presence &other ) const { return !(*this == other); }
	
private:
	unsigned long basicOscarStatus() const;
	static Type typeFromOscarStatus( unsigned long status );
private:
	Type _type;
	Visibility _visibility;
};

}

#if 0
const unsigned int ICQ_PORT  = 5190;


const unsigned short ICQ_SEARCHSTATE_OFFLINE   = 0;
const unsigned short ICQ_SEARCHSTATE_ONLINE    = 1;
const unsigned short ICQ_SEARCHSTATE_DISABLED  = 2;


// Taken from libicq, not sure if we ever support these requests
const unsigned char PHONEBOOK_SIGN[16] =
{
	0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D, 0xD3, 0x11,
	0xAD, 0xEB, 0x00, 0x04, 0xAC, 0x96, 0xAA, 0xB2
};

const unsigned char PLUGINS_SIGN[16] =
{
	0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71, 0xD3, 0x11,
	0x8D, 0xD2, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
};

/*
const unsigned char SHARED_FILES_SIGN[16] =
{
	0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91, 0xD3, 0x11,
	0x8D, 0xD7, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
};
*/
#endif

#endif
// vim: set noet ts=4 sts=4 sw=4:
// kate: indent-mode: csands
