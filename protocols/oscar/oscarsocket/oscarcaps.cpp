/***************************************************************************
                          oscarcaps.cpp  -  part of OscarSocket class
                             -------------------
    begin                : Sat Sep 27 2003

    Copyright (c) 2003 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscarsocket.h"
#include <kdebug.h>

DWORD OscarSocket::parseCap(char *cap)
{
	DWORD capflag = 0x00000000;
	for (int i = 0; !(oscar_caps[i].flag & AIM_CAPS_LAST); i++)
	{
		if (memcmp(&oscar_caps[i].data, cap, 0x10) == 0)
		{
			capflag |= oscar_caps[i].flag;
			break; // should only match once...
		}
	}
	return capflag;
}

/*
DWORD OscarSocket::parseCapString(char *cap)
{
	QString source, dest;
	source = QString::fromLatin1(cap);

	DWORD capflag = 0x00000000;
	for (int i = 0; !(oscar_caps[i].flag & AIM_CAPS_LAST); i++)
	{
		dest.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		&oscar_caps[i].data[0], &oscar_caps[i].data[1], &oscar_caps[i].data[2], &oscar_caps[i].data[3],
		&oscar_caps[i].data[4], &oscar_caps[i].data[5],
		&oscar_caps[i].data[6], &oscar_caps[i].data[7],
		&oscar_caps[i].data[8], &oscar_caps[i].data[9],
		&oscar_caps[i].data[10], &oscar_caps[i].data[11], &oscar_caps[i].data[12], &oscar_caps[i].data[13], &oscar_caps[i].data[14], &oscar_caps[i].data[15]);

		kdDebug(14150) << k_funcinfo <<
			"source=" << source <<
			"   dest=" << dest << "" << endl;

		if(source == dest)
		{
			kdDebug(14150) << k_funcinfo << "found cap" << endl;
			capflag |= oscar_caps[i].flag;
			break; // should only match once...
		}
	}
	return capflag;
}*/

const QString OscarSocket::CapToString(char *cap)
{
	QString dbg;
	dbg.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		cap[0], cap[1], cap[2], cap[3],
		cap[4], cap[5],
		cap[6], cap[7],
		cap[8], cap[9],
		cap[10], cap[11], cap[12], cap[13], cap[14], cap[15]);
	return dbg;
}
