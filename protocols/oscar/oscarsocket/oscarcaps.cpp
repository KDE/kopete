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

int OscarSocket::parseCap(char *cap)
{
	int capflag = -1;
	for (int i = 0; i < CAP_LAST; i++)
	{
		if (memcmp(&oscar_caps[i], cap, 16) == 0)
		{
			capflag = i;
			break; // should only match once...
		}
	}
	return capflag;
}

const DWORD OscarSocket::parseCapabilities(Buffer &inbuf, QString &versionString)
{
//
// FIXME: port capabilities array to some qt based list class, makes usage of memcmp obsolete
//
	DWORD capflags = 0;

#ifdef OSCAR_CAP_DEBUG
	QString dbgCaps = "CAPS: ";
#endif

	while(inbuf.length() >= 16)
	{
		char *cap;
		cap = inbuf.getBlock(16);

		for (int i=0; i < CAP_LAST; i++)
		{
			if (i == CAP_KOPETE)
			{
				if (memcmp(&oscar_caps[i], cap, 12) == 0)
				{

					kdDebug(14150) << k_funcinfo << "KOPETE version : <" <<
						(int)cap[12] << ":" << (int)cap[13] << ":" <<
						(int)cap[14] << ":" << (int)cap[15] << ">" << endl;

					capflags |= (1 << i);

					// Did a bad mistake in CVS :(
					if (
						((int)cap[12] == 0 &&
						(int)cap[13] == 8 &&
						(int)cap[14] == 90 &&
						(int)cap[15] == 0) || ((int)cap[14] + (int)cap[15] == 0))
					{
						versionString.sprintf("%d.%d.%d",
							cap[12], cap[13], cap[14]);
					}
					else
					{
						versionString.sprintf("%d.%d.%d%d",
							cap[12], cap[13], cap[14], cap[15]);
					}
					break;
				}
			}
			else if (i == CAP_MICQ)
			{
				if (memcmp(&oscar_caps[i], cap, 12) == 0)
				{
					kdDebug(14150) << k_funcinfo << "MICQ version : <" <<
						(int)cap[12] << ":" << (int)cap[13] << ":" <<
						(int)cap[14] << ":" << (int)cap[15] << ">" << endl;

					capflags |= (1 << i);

					// FIXME: how to decode this micq version mess? [mETz - 08.06.2004]
					/*versionString.sprintf("%d.%d.%d%d",
						cap[12], cap[13], cap[14], cap[15]);*/
					break;
				}
			}
			else if (i == CAP_SIMNEW)
			{
				if (memcmp(&oscar_caps[i], cap, 12) == 0)
				{
					kdDebug(14150) << k_funcinfo << "SIM version : <" <<
						(unsigned int)cap[12] << ":" << (unsigned int)cap[13] << ":" <<
						(unsigned int)cap[14] << ":" << (unsigned int)cap[15] << ">" << endl;
					capflags |= (1 << i);
					/*if ((unsigned int)cap[15] > 0)
					{*/
						versionString.sprintf("%d.%d.%d%d",
							cap[12], cap[13], cap[14], cap[15]);
					/*}
					else
					{
						versionString.sprintf("%d.%d.%d",
							cap[12], cap[13], cap[14]);
					}*/
					break;
				}
			}
			else if (i == CAP_SIMOLD)
			{
				if (memcmp(&oscar_caps[i], cap, 15) == 0)
				{
					int hiVersion = (cap[15] >> 6) - 1;
					unsigned loVersion = cap[15] & 0x1F;
					kdDebug(14150) << k_funcinfo << "OLD SIM version : <" <<
						hiVersion << ":" << loVersion << endl;
					capflags |= (1 << i);
					versionString.sprintf("%d.%d", (unsigned int)hiVersion, loVersion);
					break;
				}
			}
			else if (memcmp(&oscar_caps[i], cap, 16) == 0)
			{
				capflags |= (1 << i);

#ifdef OSCAR_CAP_DEBUG
				dbgCaps += capName(i);
#endif // OSCAR_CAP_DEBUG

				break;
			} // END if(memcmp...
		} // END for...
		delete [] cap;
	}
	#ifdef OSCAR_CAP_DEBUG
	kdDebug(14150) << k_funcinfo << dbgCaps << endl;
	#endif
	return capflags;
}

/*
DWORD OscarSocket::parseCapString(char *cap)
{
	QString source, dest;
	source = QString::fromLatin1(cap);

	DWORD capflag = 0x00000000;
	for (int i = 0; i < CAP_LAST); i++)
	{
		dest.sprintf("{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		&oscar_caps[i][0],
		&oscar_caps[i][1],
		&oscar_caps[i][2],
		&oscar_caps[i][3],
		&oscar_caps[i][4],
		&oscar_caps[i][5],
		&oscar_caps[i][6],
		&oscar_caps[i][7],
		&oscar_caps[i][8],
		&oscar_caps[i][9],
		&oscar_caps[i][10],
		&oscar_caps[i][11],
		&oscar_caps[i][12],
		&oscar_caps[i][13],
		&oscar_caps[i][14],
		&oscar_caps[i][15]);

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
}
*/

const QString OscarSocket::capToString(char *cap)
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


const QString OscarSocket::capName(int capNumber)
{
	QString capString;

	switch(capNumber)
	{
		case CAP_VOICE:
			capString = "CAP_VOICE ";
			break;
		case CAP_BUDDYICON:
			capString = "CAP_BUDDYICON ";
			break;
		case CAP_IMIMAGE:
			capString = "CAP_IMIMAGE ";
			break;
		case CAP_CHAT:
			capString = "CAP_CHAT ";
			break;
		case CAP_GETFILE:
			capString = "CAP_GETFILE ";
			break;
		case CAP_SENDFILE:
			capString = "CAP_SENDFILE ";
			break;
		case CAP_GAMES2:
		case CAP_GAMES:
			capString = "CAP_GAMES ";
			break;
		case CAP_SAVESTOCKS:
			capString = "CAP_SAVESTOCKS ";
			break;
		case CAP_SENDBUDDYLIST:
			capString = "CAP_SENDBUDDYLIST ";
			break;
		case CAP_ISICQ:
			capString = "CAP_ISICQ ";
			break;
		case CAP_APINFO:
			capString = "CAP_APINFO ";
			break;
		case CAP_RTFMSGS:
			capString = "CAP_RTFMSGS ";
			break;
		case CAP_ICQSERVERRELAY:
			capString = "CAP_ICQSERVERRELAY ";
			break;
		case CAP_IS_2001:
			capString = "CAP_IS_2001 ";
			break;
		case CAP_TRILLIAN:
			capString = "CAP_TRILLIAN ";
			break;
		case CAP_TRILLIANCRYPT:
			capString = "CAP_TRILLIANCRYPT ";
			break;
		case CAP_UTF8:
			capString = "CAP_UTF8 ";
			break;
		case CAP_IS_WEB:
			capString = "CAP_IS_WEB ";
			break;
		case CAP_INTEROPERATE:
			capString = "CAP_INTEROPERATE ";
			break;
		case CAP_KOPETE:
			capString = "CAP_KOPETE ";
			break;
		case CAP_MICQ:
			capString = "CAP_MICQ ";
			break;
		case CAP_MACICQ:
			capString = "CAP_MACICQ ";
			break;
		case CAP_SIMOLD:
			capString = "CAP_SIMOLD ";
			break;
		case CAP_SIMNEW:
			capString = "CAP_SIMNEW ";
			break;
		case CAP_XTRAZ:
			capString = "CAP_XTRAZ ";
			break;
		case CAP_STR_2001:
			capString = "CAP_STR_2001 ";
			break;
		case CAP_STR_2002:
			capString = "CAP_STR_2002 ";
			break;
		default:
			capString = "UNKNOWN CAP ";
	} // END switch
	return capString;
}
