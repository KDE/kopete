/*
    oscarpresence.h  -  Oscar presence class
    
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


#ifndef OSCARPRESENCE_H
#define OSCARPRESENCE_H

#include <QFlags>
#include <QString>

#include "kopete_export.h"

class OscarStatusManager;

namespace Oscar
{

/**
 * @brief An Oscar presence object
 */
class OSCAR_EXPORT Presence
{
public:
	/**
	 * Friendly types this status can be
	 */
	enum Type { Offline = 0x000, DoNotDisturb = 0x001, Occupied = 0x002,
	            NotAvailable = 0x003, Away = 0x004, FreeForChat = 0x005, Online = 0x006 };
	enum { TypeCount = Online + 1 };

	enum Flag { None = 0x000, AIM = 0x010, ICQ = 0x020, Wireless = 0x100, Invisible = 0x200, XStatus = 0x400 };
	Q_DECLARE_FLAGS(Flags, Flag)

	Presence( Type type, Flags flags = None );

	void setType( Type type );
	Type type() const { return (Type)(mInternalStatus & 0x0000000F); }

	void setFlags( Flags flags );
	Flags flags() const { return (Flags)(mInternalStatus & 0xFFFFFFF0); }

	uint internalStatus() const { return mInternalStatus; }

	bool operator==( const Presence &other ) const { return other.mInternalStatus == mInternalStatus; }
	bool operator!=( const Presence &other ) const { return !(*this == other); }

	/**
	 * XStatus functions.
	 */
	void setDescription( const QString& desc ) { mDescription = desc; }
	QString description() const { return mDescription; }

	void setIcon( const QString& icon ) { mIcon = icon; }
	QString icon() const { return mIcon; }

private:
	friend class ::OscarStatusManager;
	Presence( uint internalStatus );

	uint mInternalStatus;

	// For XStatus
	QString mDescription;
	QString mIcon;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Presence::Flags)

}

#endif
