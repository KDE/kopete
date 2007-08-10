/*
    oscarpresencesdataclasses.h  -  Oscar data classes for oscar status manager
    
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2007      by Roman Jarosz           <kedgedev@centrum.cz>
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


#ifndef OSCARSPRESENCEDATACLASSES_H
#define OSCARSPRESENCEDATACLASSES_H

#include "oscarpresence.h"

#include <QStringList>

#include <kopeteonlinestatus.h>
#include "kopete_export.h"

namespace Kopete { class OnlineStatus; }


namespace Oscar
{

/**
 * @brief This namespace contains status flags used in OSCAR's on-the-wire format.
 * 
 * The IS_XXX values are bits representing actual status flags. However, the flags
 * are just that -- flags.  Oscar statuses are represented by a combination of these
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
			WIRELESS   = 0x0080
	};
} // end namespace ClassCode

class OSCAR_EXPORT PresenceOverlay
{
public:
	PresenceOverlay();
	PresenceOverlay( Presence::Flags flags, QString name, QStringList icons );
	
	Presence::Flags flags() const { return mFlags; }
	QString description() const { return mDescription; }
	QStringList icons() const { return mIcons; }
	
	PresenceOverlay &operator+= ( const PresenceOverlay &other );
	
private:
	Presence::Flags mFlags;
	QString mDescription;
	QStringList mIcons;
};

class OSCAR_EXPORT PresenceType
{
public:
	typedef QList<Presence::Flags> FlagsList;
	PresenceType( Presence::Type type, Kopete::OnlineStatus::StatusType onlineStatusType,
	              unsigned long setFlag, unsigned long getFlag, QString caption, QString name,
	              QStringList overlayIcons, Kopete::OnlineStatusManager::Categories categories,
	              Kopete::OnlineStatusManager::Options options, FlagsList overlayFlagsList );

	Presence::Type type() const { return mType; }
	Kopete::OnlineStatus::StatusType onlineStatusType() const { return mOnlineStatusType; }
	unsigned long setFlag() const { return mSetFlag; }
	unsigned long getFlag() const { return mGetFlag; }
	QString caption() const { return mCaption; }
	QString name() const { return mName; }
	QStringList overlayIcons() const { return mOverlayIcons; }
	Kopete::OnlineStatusManager::Categories categories() const { return mCategories; }
	Kopete::OnlineStatusManager::Options options() const { return mOptions; }
	FlagsList overlayFlagsList() const { return mOverlayFlagsList; }

private:
	Presence::Type mType;
	Kopete::OnlineStatus::StatusType mOnlineStatusType;
	unsigned long mSetFlag;
	unsigned long mGetFlag;
	QString mCaption;
	QString mName;
	QStringList mOverlayIcons;
	Kopete::OnlineStatusManager::Categories mCategories;
	Kopete::OnlineStatusManager::Options mOptions;
	FlagsList mOverlayFlagsList;
};

}

#endif

