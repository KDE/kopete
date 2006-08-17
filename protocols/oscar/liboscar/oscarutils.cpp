/*
    Kopete Oscar Protocol
    oscarutils.cpp - Oscar Utility Functions

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "oscarutils.h"
#include <qhostaddress.h>
#include <kapplication.h>
#include <kdebug.h>


using namespace Oscar;

QString Oscar::normalize( const QString& contact )
{
	QString normal = contact.lower();
	normal.remove( ' ' );
	return normal;
}

bool Oscar::operator==( TLV a, TLV b )
{
	if ( a.type == b.type && a.length == b.length )
		return true;
	else
		return false;
}

TLV Oscar::findTLV( const QValueList<TLV>& list, int type )
{
	TLV t;
	QValueList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		if ( ( *it ).type == type )
			return ( *it );
	}

	return t;
}

bool Oscar::uptateTLVs( SSI& item, const QValueList<TLV>& list )
{
	bool changed = false;
	QValueList<TLV> tList( item.tlvList() );

	QValueList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		TLV t = Oscar::findTLV( tList, ( *it ).type );
		if ( t && t.length == ( *it ).length &&
		     memcmp( t.data.data(), ( *it ).data.data(), t.length ) == 0 )
			continue;

		if ( t )
			tList.remove( t );

		tList.append( *it );
		changed = true;
	}

	if ( changed )
		item.setTLVList( tList );

	return changed;
}

int Oscar::parseCap( char* cap )
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

const QString Oscar::capToString( char* cap )
{
	QString dbg;

	dbg.sprintf( "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		cap[0], cap[1], cap[2], cap[3], cap[4], cap[5], cap[6], cap[7], cap[8], cap[9],
		cap[10], cap[11], cap[12], cap[13], cap[14], cap[15] );

	return dbg;
}

DWORD Oscar::parseCapabilities( Buffer &inbuf, QString &versionString )
{
	//
	// FIXME: port capabilities array to some qt based list class, makes usage of memcmp obsolete
	//
	DWORD capflags = 0;
	QString dbgCaps = "CAPS: ";
	
	while(inbuf.length() >= 16)
	{
		QByteArray cap;
		cap.duplicate( inbuf.getBlock(16) );
		
		for (int i=0; i < CAP_LAST; i++)
		{
			if (i == CAP_KOPETE)
			{
				if (memcmp(&oscar_caps[i], cap.data(), 12) == 0)
				{
					capflags |= (1 << i);
					versionString.sprintf( "%d.%d.%d%d", cap[12], cap[13], cap[14], cap[15] );
					versionString.insert( 0, "Kopete " );
					kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Kopete version - " << versionString << endl;
				}
			}
			else if (i == CAP_MICQ)
			{
				if (memcmp(&oscar_caps[i], cap.data(), 12) == 0)
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
					versionString.sprintf("%d.%d.%d%d",
					                      cap[12], cap[13], cap[14], cap[15]);
					versionString.insert( 0, "SIM " );
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
					versionString.insert( 0, "SIM " );
					break;
				}
			}
			else if (memcmp(&oscar_caps[i], cap.data(), 16) == 0)
			{
				capflags |= (1 << i);
				dbgCaps += capName(i);
				break;
			} // END if(memcmp...
		} // END for...
	}
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << dbgCaps << endl;
	return capflags;
}

const QString Oscar::capName( int capNumber )
{
	QString capString;

	switch ( capNumber )
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
	case CAP_TYPING:
		capString = "CAP_TYPING ";
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

DWORD Oscar::getNumericalIP(const QString &address)
{
	QHostAddress addr;
	if ( addr.setAddress( address ) == false )
		return 0;

	return (DWORD)addr.toIPv4Address();
}

QString Oscar::getDottedDecimal( DWORD address )
{
	QHostAddress addr;
	addr.setAddress((Q_UINT32)address);
	return addr.toString();
}

	

//kate: tab-width 4; indent-mode csands;
