/*
    oscarstatusmanager.h  -  Oscar status manager
    
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


#ifndef OSCARSTATUSMANAGER_H
#define OSCARSTATUSMANAGER_H

#include "oscarpresence.h"
#include <kopeteonlinestatus.h>

#include "kopete_export.h"

namespace Kopete { class OnlineStatus; }

namespace Oscar
{
	class PresenceType;
	class PresenceOverlay;
}

class OscarProtocol;

/**
 * @brief A manager for Oscar's online statuses
 * 
 * Looks after Oscar's numerous online statuses, and maps between them and Presence objects.
 * A single instance of this class is held by the AIMProtocol and ICQProtocol object.
 */
class OSCAR_EXPORT OscarStatusManager
{
public:
	OscarStatusManager( OscarProtocol* protocol );
	virtual ~OscarStatusManager();

	/**
	 * Create KOS from PresenceType and PresenceOverlay lists.
	 */
	void initialize( uint firstUsableWeight );

	/**
	 * Set PresenceType list.
	 */
	void setPresenceType( const QList<Oscar::PresenceType>& list );

	/**
	 * Set available overlays.
	 * We can define overlay for one flag from Oscar::Presence::Flags or
	 * for combination of Oscar::Presence::Flags.
	 */
	void setPresenceOverlay( const QList<Oscar::PresenceOverlay>& list );

	/**
	 * Set Presence::Flags mask. This will filter unnecessary overlays.
	 * E.g. ICQ flag for ICQ account or contacts presence.
	 */
	void setPresenceFlagsMask( Oscar::Presence::Flags mask );

	/**
	 * Generate an online status from a Presence object
	 */
	Kopete::OnlineStatus onlineStatusOf( const Oscar::Presence &presence ) const;

	/**
	 * Generate a Presence object from an online status
	 */
	Oscar::Presence presenceOf( const Kopete::OnlineStatus &status ) const;

	/**
	 * Get the status code to pass to liboscar to set us to this Status.
	 * @note This is not the opposite of onlineStatusOf().
	 */
	unsigned long oscarStatusOf( const Oscar::Presence &presence ) const;

	/**
	 * Get the status a contact is at based on liboscar's view of its status.
	 */
	Oscar::Presence presenceOf( unsigned long oStatus, int oClass ) const;

	virtual Kopete::OnlineStatus connectingStatus() const = 0;
	virtual Kopete::OnlineStatus unknownStatus() const = 0;
	virtual Kopete::OnlineStatus waitingForAuth() const = 0;

private:
	unsigned long basicOscarStatus( Oscar::Presence::Type type ) const;
	Oscar::Presence::Type pscTypeForOscarStatus( unsigned long status ) const;
	Oscar::PresenceOverlay pscOverlayForFlags( Oscar::Presence::Flags flags ) const;

	const Oscar::PresenceType &pscTypeForType( Oscar::Presence::Type type ) const;
	const Oscar::PresenceType &pscTypeForStatus( unsigned long status ) const;
	const Oscar::PresenceType &pscTypeForOnlineStatusType( const Kopete::OnlineStatus::StatusType statusType ) const;

	QString kosDescription( const Oscar::Presence &presence ) const;

	class Private;
	Private * const d;
};

#endif
