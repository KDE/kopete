/*
    nlqmmp.h

    Kopete Now Listening To plugin


    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>
    Copyright (c) 2009 by Attila Herman <attila589/at/gmail.com>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

	Purpose:
	This class abstracts the interface to qmmp by
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

#ifndef NLQMMP_H
#define NLQMMP_H

class QDBusInterface;

class NLqmmp : public NLMediaPlayer
{
	public:
		NLqmmp();
		virtual ~NLqmmp();
		virtual void update();
	private:
		QDBusInterface *m_client;
};

#endif
