/*
    nlxmms.h

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to the X Multimedia System (xmms) by
	implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NLXMMS_H
#define NLXMMS_H

#ifdef HAVE_XMMS

#include "nlmediaplayer.h"

class NLXmms : public NLMediaPlayer
{
	public:
		NLXmms();
		virtual void update();
};

#endif

#endif

// vim: set noet ts=4 sts=4 sw=4:
