/*
    oscarpresence.cpp  -  Oscar presence class
    
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

#include "oscarpresence.h"

namespace Oscar
{

const int Presence::moodToXtraz[] = { 17, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 19, 20, 21, 22, 23, 0 };

Presence::Presence( Type type, Flags flags )
{
	mInternalStatus = type | flags;
}

Presence::Presence( uint internalStatus )
{
	mInternalStatus = internalStatus;
}

void Presence::setType( Type type )
{
	mInternalStatus &= ~TypeMask;
	mInternalStatus |= type;
}

void Presence::setFlags( Flags flags )
{
	mInternalStatus &= ~FlagsMask;
	mInternalStatus |= flags;
}

void Presence::setXtrazStatus( int xtraz )
{
	mInternalStatus &= ~XtrazMask; 
	mInternalStatus |= (xtraz << 24);
}

int Presence::xtrazStatus() const
{
	if ( (mInternalStatus & Oscar::Presence::XStatus) || (mInternalStatus & Oscar::Presence::ExtStatus2) )
		return ((mInternalStatus & XtrazMask) >> 24);
	else
		return -1;
}


void Presence::setMood( int mood )
{
	if ( -1 < mood && mood <= 23 )
	{
		int xtraz = moodToXtraz[mood];
		mInternalStatus &= ~XtrazMask; 
		mInternalStatus |= (xtraz << 24);
	}
}

int Presence::mood() const
{
	int xtraz = xtrazStatus();
	if ( -1 < xtraz && xtraz <= 23 )
	{
		for ( int i = 0; i < 24; i++)
		{
			if ( moodToXtraz[i] == xtraz )
				return i;
		}
	}

	return -1;
}

}
