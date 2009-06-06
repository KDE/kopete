/*
    oscarpresencesdataclasses.cpp  -  Oscar data classes for oscar status manager
    
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

#include "oscarpresencesdataclasses.h"

namespace Oscar
{

//BEGIN class PresenceOverlay

PresenceOverlay::PresenceOverlay()
: mFlags( Presence::None )
{
}

PresenceOverlay::PresenceOverlay( Presence::Flags flags, QString description, QStringList icons )
	: mFlags(flags), mDescription(description), mIcons(icons)
{
}

PresenceOverlay &PresenceOverlay::operator+=( const PresenceOverlay &other )
{
	mFlags |= other.mFlags;
	if ( mDescription.isEmpty() )
		mDescription = other.mDescription;
	else if ( !other.mDescription.isEmpty() )
		mDescription += QString( ", " ) + other.mDescription;

	mIcons << other.mIcons;
	return *this;
}

//END class PresenceOverlay

//BEGIN class PresenceType

PresenceType::PresenceType( Presence::Type type, Kopete::OnlineStatus::StatusType onlineStatusType, unsigned long setFlag,
              unsigned long getFlag, QString caption, QString name, QStringList overlayIcons,
              Kopete::OnlineStatusManager::Categories categories, Kopete::OnlineStatusManager::Options options,
              FlagsList overlayFlagsList )
: mType(type), mOnlineStatusType(onlineStatusType), mSetFlag(setFlag), mGetFlag(getFlag), mCaption(caption),
  mName(name), mOverlayIcons(overlayIcons), mCategories(categories), mOptions(options), mOverlayFlagsList(overlayFlagsList)
{
}

//END class PresenceType


} // end namespace Oscar

