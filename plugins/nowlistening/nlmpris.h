/*
	nlmpris.h

    Kopete Now Listening To plugin


    Copyright (c) 2010 by Volker Härtel <cyberbeat@gmx.de>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to audacious by
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

#ifndef NLMPRIS_H
#define NLMPRIS_H

#include <QtDBus/QtDBus>
#include "nlmediaplayer.h"

class QDBusInterface;

class NLmpris : public NLMediaPlayer
{
	public:
		NLmpris();
		virtual ~NLmpris();
		virtual void update();
	private:
		QDBusInterface *m_client;
};

#endif
