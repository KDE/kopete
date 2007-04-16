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
#include <kdebug.h>


using namespace Oscar;

QString Oscar::normalize( const QString& contact )
{
	QString normal = contact.toLower();
	normal.remove( QChar( ' ' ) );
	return normal;
}

bool Oscar::operator==( TLV a, TLV b )
{
	if ( a.type == b.type && a.length == b.length )
		return true;
	else
		return false;
}

TLV Oscar::findTLV( const QList<TLV>& list, int type )
{
	TLV t;
	QList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		if ( ( *it ).type == type )
			return ( *it );
	}

	return t;
}

bool Oscar::updateTLVs( OContact& item, const QList<TLV>& list )
{
	bool changed = false;
	QList<TLV> tList( item.tlvList() );

	QList<TLV>::const_iterator it;
	for ( it = list.begin(); it != list.end(); ++it )
	{
		TLV t = Oscar::findTLV( tList, ( *it ).type );
		if ( t && t.length == ( *it ).length &&
		     memcmp( t.data.data(), ( *it ).data.data(), t.length ) == 0 )
			continue;

		if ( t )
			tList.removeAll( t );

		tList.append( *it );
		changed = true;
	}

	if ( changed )
		item.setTLVList( tList );

	return changed;
}

Oscar::DWORD Oscar::parseCapabilities( Buffer &inbuf, QString &versionString, int &xStatus )
{
	Oscar::DWORD capflags = 0;
	xStatus = -1;
	QString dbgCaps = "CAPS: ";

	while(inbuf.bytesAvailable() >= 16)
	{
		bool found = false;
		Guid cap( inbuf.getGuid() );

		for (int i=0; i < CAP_LAST; i++)
		{
			if (i == CAP_KOPETE)
			{
				if ( oscar_caps[i].data().left(12) == cap.data().left(12) )
				{
					capflags |= (1 << i);
					versionString.sprintf( "%d.%d.%d%d", cap.data().at(12), cap.data().at(13), cap.data().at(14), cap.data().at(15) );
					versionString.insert( 0, "Kopete " );
					kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Kopete version - " << versionString << endl;
					found = true;
					break;
				}
			}
			else if (i == CAP_MICQ)
			{
				if ( oscar_caps[i].data().left(12) == cap.data().left(12) )
				{
					kDebug(14150) << k_funcinfo << "MICQ version : <" <<
						(int)cap.data()[12] << ":" << (int)cap.data()[13] << ":" <<
						(int)cap.data()[14] << ":" << (int)cap.data()[15] << ">" << endl;

					capflags |= (1 << i);

						// FIXME: how to decode this micq version mess? [mETz - 08.06.2004]
						/*versionString.sprintf("%d.%d.%d%d",
							cap[12], cap[13], cap[14], cap[15]);*/
					found = true;
					break;
				}
			}
			else if (i == CAP_SIMNEW)
			{
				if ( oscar_caps[i].data().left(12) == cap.data().left(12) )
				{
					kDebug(14150) << k_funcinfo << "SIM version : <" <<
						(unsigned int)cap.data()[12] << ":" << (unsigned int)cap.data()[13] << ":" <<
						(unsigned int)cap.data()[14] << ":" << (unsigned int)cap.data()[15] << ">" << endl;
					capflags |= (1 << i);
					versionString.sprintf("%d.%d.%d%d",
					                      cap.data().at(12), cap.data().at(13), cap.data().at(14), cap.data().at(15));
					versionString.insert( 0, "SIM " );
					found = true;
					break;
				}
			}
			else if (i == CAP_SIMOLD)
			{
				if ( oscar_caps[i].data().left(15) == cap.data().left(15) )
				{
					int hiVersion = (cap.data()[15] >> 6) - 1;
					unsigned loVersion = cap.data()[15] & 0x1F;
					kDebug(14150) << k_funcinfo << "OLD SIM version : <" <<
						hiVersion << ":" << loVersion << endl;
					capflags |= (1 << i);
					versionString.sprintf("%d.%d", (unsigned int)hiVersion, loVersion);
					versionString.insert( 0, "SIM " );
					found = true;
					break;
				}
			}
			else if ( oscar_caps[i] == cap )
			{
				capflags |= (1 << i);
				dbgCaps += capName(i);
				found = true;
				break;
			} // END if...
		} // END for...
		if ( !found && xStatus == -1 )
		{
			for ( int i = 0; i < XSTAT_LAST; i++ )
			{
				if ( oscar_xStatus[i] == cap )
				{
					xStatus = i;
					found = true;
					break;
				}
			}
		}
	}
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << dbgCaps << endl;
	return capflags;
}

Oscar::DWORD Oscar::parseNewCapabilities( Buffer &inbuf )
{
	Oscar::DWORD capflags = 0;
	QString dbgCaps = "NEW CAPS: ";

	QByteArray cap = Guid( QLatin1String( "094600004c7f11d18222444553540000" ) );
	while( inbuf.bytesAvailable() >= 2 )
	{
		cap[2] = inbuf.getByte();
		cap[3] = inbuf.getByte();

		for ( int i = 0; i < CAP_LAST; i++ )
		{
			if ( oscar_caps[i].data() == cap )
			{
				capflags |= (1 << i);
				dbgCaps += capName(i);
				break;
			}
		}
	}

	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << dbgCaps << endl;
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
	case CAP_DIRECT_ICQ_COMMUNICATION:
		capString = "CAP_DIRECT_ICQ_COMMUNICATION ";
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
	case CAP_XTRAZ_MULTIUSER_CHAT:
		capString = "CAP_XTRAZ_MULTIUSER_CHAT ";
		break;
	case CAP_DEVILS:
		capString = "CAP_DEVILS ";
		break;
	case CAP_NEWCAPS:
		capString = "CAP_NEWCAPS ";
		break;
	case CAP_UNKNOWN1:
		capString = "CAP_UNKNOWN1 ";
		break;
	case CAP_UNKNOWN2:
		capString = "CAP_UNKNOWN2 ";
		break;
	case CAP_PUSH2TALK:
		capString = "CAP_PUSH2TALK ";
		break;
	case CAP_VIDEO:
		capString = "CAP_VIDEO ";
		break;
	default:
		capString = "UNKNOWN CAP ";
	} // END switch

	return capString;
}

Oscar::DWORD Oscar::getNumericalIP(const QString &address)
{
	QHostAddress addr;
	if ( addr.setAddress( address ) == false )
		return 0;

	return (DWORD)addr.toIPv4Address();
}

QString Oscar::getDottedDecimal( Oscar::DWORD address )
{
	QHostAddress addr;
	addr.setAddress((quint32)address);
	return addr.toString();
}



//kate: tab-width 4; indent-mode csands;
