/*
    nlkaffeine.h

    Kopete Now Listening To plugin


    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to Kaffeine by
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

#ifndef NLKAFFEINE_H
#define NLKAFFEINE_H

#include <dcopclient.h>

class NLKaffeine : public NLMediaPlayer
{
	public:
		NLKaffeine( DCOPClient *client );
		virtual void update();
	protected:
		DCOPClient *m_client;
};

#endif

