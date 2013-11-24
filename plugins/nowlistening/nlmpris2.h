/*
	nlmpris2.h

    Kopete Now Listening To plugin


    Copyright (c) 2012 by Volker Härtel <cyberbeat@gmx.de>

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

#ifndef NLMPRIS2_H
#define NLMPRIS2_H

#include <QtDBus/QtDBus>
#include "nlmediaplayer.h"

class QDBusInterface;

class NLmpris2 : public NLMediaPlayer
{
	public:
		NLmpris2();
		virtual ~NLmpris2();
		virtual void update();
	private:
		QDBusInterface *m_client;
};

#endif
