/*
    kopeteaway.cpp  -  Kopete Away

	Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteaway.h"

#include <qstring.h>
#include <kconfig.h>
#include <qmultilineedit.h>

#include <kopete.h>
#include "kopeteawaydialog.h"

KopeteAway *KopeteAway::instance = 0L;

KopeteAway::KopeteAway()
{
	mAwayMessage = "";
	mGlobalAway = false;
}

QString KopeteAway::message()
{
	return getInstance()->mAwayMessage;
}

void KopeteAway::show()
{
	KopeteAwayDialog awaydialog;
	awaydialog.exec();
}

KopeteAway *KopeteAway::getInstance()
{
	if (instance == 0L)
	{
		instance = new KopeteAway;
	}
	return instance;
}

bool KopeteAway::globalAway()
{
	return getInstance()->mGlobalAway;
}

void KopeteAway::setGlobalAway(bool status)
{
	getInstance()->mGlobalAway = status;	
}



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

