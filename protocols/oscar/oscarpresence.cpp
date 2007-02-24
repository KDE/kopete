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
	if ( mInternalStatus & Oscar::Presence::XStatus )
		return ((mInternalStatus & XtrazMask) >> 24);
	else
		return -1;
}

}
