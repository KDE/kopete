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
	enum Type {
		Offline       = 0x00000000,
		DoNotDisturb  = 0x00000001,
		Occupied      = 0x00000002,
		NotAvailable  = 0x00000003,
		Away          = 0x00000004,
		FreeForChat   = 0x00000005,
		Online        = 0x00000006,

		TypeMask      = 0x0000000F
	};
	enum { TypeCount = Online + 1 };

	enum Flag {
		None          = 0x00000000,
		AIM           = 0x00000010,
		ICQ           = 0x00000020,
		Wireless      = 0x00000100,
		Invisible     = 0x00000200,
		XStatus       = 0x00001000,
		ExtStatus     = 0x00002000,
		ExtStatus2    = 0x00004000,

		FlagsMask     = 0x0000FFF0,
		StatusTypeMask= 0x0000F000
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	enum {
		XtrazMask     = 0xFF000000
	};

	explicit Presence( Type type, Flags flags = None );

	void setType( Type type );
	Type type() const { return (Type)(mInternalStatus & TypeMask); }

	void setFlags( Flags flags );
	Flags flags() const { return (Flags)(mInternalStatus & FlagsMask); }

	/**
	 * Returns internal status
	 * @note Internal status is 32-bit int with a following structure XX00FFFT where
	 * T is status type, FFF are Presence::Flags, XX is Xtraz status,
	 * 0 are always null
	 */
	uint internalStatus() const { return mInternalStatus; }

	bool operator==( const Presence &other ) const { return other.mInternalStatus == mInternalStatus; }
	bool operator!=( const Presence &other ) const { return !(*this == other); }

	/**
	 * Sets Xtraz status
	 */
	void setXtrazStatus( int xtraz );

	/**
	 * Returns Xtraz status
	 * @note If Presence::XStatus or Presence::ExtStatus2 flags are not set function returns -1.
	 */
	int xtrazStatus() const;

	/**
	 * Sets mood (ExtStatus2 status icon)
	 */
	void setMood( int mood );

	/**
	 * Returns mood
	 * @note If Presence::XStatus or Presence::ExtStatus2 flags are not set function returns -1.
	 */
	int mood() const;

private:
	friend class ::OscarStatusManager;
	Presence( uint internalStatus );

	uint mInternalStatus;
	static const int moodToXtraz[];
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Presence::Flags)

}

#endif
