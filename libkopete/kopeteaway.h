/*
    kopeteaway.h  -  Kopete Away

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

#ifndef KOPETEAWAY_HI
#define KOPETEAWAY_HI

#include <qstring.h>
#include <kconfig.h>

/**
 * @author Hendrik vom Lehn <hvl@linux-4-ever.de>
 *
 */

class KopeteAway
{
friend class KopeteAwayDialog;
	
	public:
	static KopeteAway *getInstance();
	
	static QString message();
	static void show();
	static void setGlobalAway(bool status);
	static bool globalAway();

	private:
	KopeteAway();
	static KopeteAway *instance;
	QString mAwayMessage;
	bool mGlobalAway;
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

