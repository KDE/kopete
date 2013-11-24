/*
    nljuk.h

    Kopete Now Listening To plugin


    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2003 by Ismail Donmez <ismail.donmez@boun.edu.tr>

    Kopete    (c) 2002,2003 by the Kopete developers  <kopete-devel@kde.org>
	
	Purpose: 
	This class abstracts the interface to JuK by
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

#ifndef NLJUK_H
#define NLJUK_H

#include <QtDBus/QtDBus>
#include "nlmediaplayer.h"

class QDBusInterface;

class NLJuk : public NLMediaPlayer
{
	public:
		NLJuk();
		virtual ~NLJuk();
		virtual void update();

	private:
		QDBusInterface *m_client;
};

#endif

