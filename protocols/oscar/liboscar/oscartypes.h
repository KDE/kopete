/*
    Kopete Oscar Protocol
    oscartypes.h - Oscar Type Definitions

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

#ifndef _OSCARTYPES_H_
#define _OSCARTYPES_H_

#include "oscartypeclasses.h"
#include "oscarguid.h"
#include <qglobal.h>
#include <qdatetime.h>
#include <qstring.h>

//! Debug Areas
const int OSCAR_RAW_DEBUG = 14151;
const int OSCAR_GEN_DEBUG = 14150;
const int OSCAR_AIM_DEBUG = 14152;
const int OSCAR_ICQ_DEBUG = 14153;

namespace Oscar
{
//! Capabilities
enum Capabilities
{
	CAP_CHAT = 0, CAP_VOICE, CAP_SENDFILE, CAP_DIRECT_ICQ_COMMUNICATION, CAP_IMIMAGE, CAP_BUDDYICON, CAP_SAVESTOCKS,
	CAP_GETFILE, CAP_ICQSERVERRELAY, CAP_GAMES, CAP_GAMES2, CAP_SENDBUDDYLIST, CAP_RTFMSGS, CAP_IS_2001,
	CAP_TRILLIAN, CAP_TRILLIANCRYPT, CAP_APINFO, CAP_UTF8, CAP_TYPING, CAP_INTEROPERATE, CAP_KOPETE, CAP_MICQ,
	CAP_MACICQ, CAP_SIMOLD, CAP_SIMNEW, CAP_XTRAZ, CAP_STR_2001, CAP_STR_2002, CAP_XTRAZ_MULTIUSER_CHAT,
	CAP_DEVILS, CAP_NEWCAPS, CAP_UNKNOWN1, CAP_UNKNOWN2, CAP_UNKNOWN3, CAP_LAST
};

const Guid oscar_caps[] =
{
	//CAP_CHAT,
	Guid( QLatin1String( "748f2420628711d18222444553540000" ) ),

	//CAP_VOICE,
	Guid( QLatin1String( "094613414c7f11d18222444553540000" ) ),

	// CAP_SENDFILE,
	Guid( QLatin1String( "094613434c7f11d18222444553540000" ) ),

	// CAP_DIRECT_ICQ_COMMUNICATION,
	Guid( QLatin1String( "094613444c7f11d18222444553540000" ) ),

	// CAP_IMIMAGE,
	Guid( QLatin1String( "094613454c7f11d18222444553540000" ) ),

	// CAP_BUDDYICON,
	Guid( QLatin1String( "094613464c7f11d18222444553540000" ) ),

	// CAP_SAVESTOCKS,
	Guid( QLatin1String( "094613474c7f11d18222444553540000" ) ),

	// CAP_GETFILE,
	Guid( QLatin1String( "094613484c7f11d18222444553540000" ) ),

	// CAP_ICQSERVERRELAY,
	Guid( QLatin1String( "094613494c7f11d18222444553540000" ) ),

	// CAP_GAMES,
	Guid( QLatin1String( "0946134a4c7f11d18222444553540000" ) ),

	// CAP_GAMES2,
	Guid( QLatin1String( "0946134a4c7f11d12282444553540000" ) ),

	// CAP_SENDBUDDYLIST,
	Guid( QLatin1String( "0946134b4c7f11d18222444553540000" ) ),

	// CAP_RTFMSGS,
	Guid( QLatin1String( "97b12751243c4334ad22d6abf73f1492" ) ),

	// CAP_IS_2001,
	Guid( QLatin1String( "2e7a6475fadf4dc8886fea3595fdb6df" ) ),

	// CAP_TRILLIAN
	Guid( QLatin1String( "97b12751243c4334ad22d6abf73f1409" ) ),

	// CAP_TRILLIANCRYPT
	Guid( QLatin1String( "f2e7c7f4fead4dfbb23536798bdf0000" ) ),

	// CAP_APINFO,
	Guid( QLatin1String( "AA4A32B5F88448c6A3D78C509719FD5B" ) ),

	// CAP_UTF8,
	Guid( QLatin1String( "0946134E4C7F11D18222444553540000" ) ),

	// CAP_TYPING - client supports mini typing notifications
	Guid( QLatin1String( "563FC8090B6f41BD9F79422609DFA2F3" ) ),

	// CAP_INTEROPERATE,
	Guid( QLatin1String( "0946134D4C7F11D18222444553540000" ) ),

	// CAP_KOPETE,
	// last 4 bytes determine version
	// NOTE change with each Kopete Release!
	// first number, major version
	// second number,  minor version
	// third number, point version 100+
	// fourth number,  point version 0-99
	Guid( QByteArray::fromRawData( "Kopete ICQ  \0\xc\0\x1", 16 ) ),

	// CAP_MICQ
	// last 4 bytes determine version
	Guid( QLatin1String( "6d49435120a920522e4b2e2000000000" ) ),

	// CAP_MACICQ
	Guid( QLatin1String( "DD16F20284E611D490DB00104B9B4B7D" ) ),

	// CAP_SIMOLD
	// last byte determines version
	// (major + 1) << 6 + minor
	Guid( QLatin1String( "97B12751243C4334AD22D6ABF73F1400" ) ),

	// CAP_SIMNEW
	// last 4 bytes determine version (US-ASCII encoded)
	Guid( QByteArray::fromRawData( "SIM client  \0\0\0\0", 16 ) ),

	// CAP_XTRAZ
	Guid( QLatin1String( "1A093C6CD7FD4EC59D51A6474E34F5A0" ) ),

	// CAP_STR_2001
	Guid( QLatin1String( "A0E93F374C7F11D18222444553540000" ) ),

	// CAP_STR_2002
	Guid( QLatin1String( "10CF40D14C7F11D18222444553540000" ) ),

	// CAP_XTRAZ_MULTIUSER_CHAT
	Guid( QLatin1String( "67361515612D4C078F3DBDE6408EA041" ) ),

	// CAP_DEVILS
	Guid( QLatin1String( "0946134C4C7F11D18222444553540000" ) ),

	// CAP_NEWCAPS
	Guid( QLatin1String( "094600004C7F11D18222444553540000" ) ),

	// CAP_UNKNOWN1
	Guid( QLatin1String( "B2EC8F167C6F451BBD79DC58497888B9" ) ),

	// CAP_UNKNOWN2
	Guid( QLatin1String( "B99708B53A924202B069F1E757BB2E17" ) ),

	// CAP_UNKNOWN3
	Guid( QLatin1String( "E362C1E9121A4B94A6267A74DE24270D" ) ),

	// CAP_LAST,
	Guid( QLatin1String( "00000000000000000000000000000000" ) )
};

//! Oscar Data Types
typedef quint8 BYTE;
typedef quint16 WORD;
typedef quint32 DWORD;


struct FLAP
{
	BYTE channel;
	WORD sequence;
	WORD length;
};

struct SNAC
{
	WORD family;
	WORD subtype;
	WORD flags;
	DWORD id;
};

struct RateInfo
{
	WORD classId;
	DWORD windowSize;
	DWORD initialLevel;
	DWORD clearLevel;
	DWORD alertLevel;
	DWORD limitLevel;
	DWORD disconnectLevel;
	DWORD currentLevel;
	DWORD maxLevel;
	DWORD lastTime;
	BYTE currentState;
};

struct ChatExchangeInfo
{
	WORD number;
    WORD maxRooms;
    WORD maxRoomNameLength;
	WORD maxMsgLength;
	BYTE flags;
	QString description;
	BYTE canCreate;
	QString charset1;
	QString charset2;
	QString lang1;
	QString lang2;
};

struct ChatRoomInfo
{
	WORD exchange;
	QByteArray cookie;
	WORD instance;
	QString description;
	WORD maxMsgLength;
	QString name;
};

struct OFT
{
	WORD type;
	QByteArray cookie;
	DWORD fileSize;
	DWORD modTime;
	DWORD checksum;
	DWORD bytesSent;
	DWORD sentChecksum;
	BYTE flags;
	QString fileName;
};

struct ClientVersion
{
	QString clientString;
	WORD clientId;
	WORD major;
	WORD minor;
	WORD point;
	WORD build;
	DWORD other;
	QString country;
	QString lang;
};

	/* ICQ Version Characteristics */
	const unsigned char ICQ_TCP_VERSION 	= 0x0008;

	/* AIM Version Characteristics */
	const char AIM_MD5_STRING[]     = "AOL Instant Messenger (SM)";

	/* SSI types */
	const WORD ROSTER_CONTACT       = 0x0000; // a normal contact
	const WORD ROSTER_GROUP         = 0x0001; // a group of contacts
	const WORD ROSTER_VISIBLE       = 0x0002; // a contact on the visible list
	const WORD ROSTER_INVISIBLE     = 0x0003; // a contact on the invisible list
	const WORD ROSTER_VISIBILITY    = 0x0004; // this entry contains visibility setting TLV(0xca)=TLV(202)
	const WORD ROSTER_PRESENCE      = 0x0005; // Presence info (if others can see your idle status, etc)
	const WORD ROSTER_ICQSHORTCUT   = 0x0009; // Unknown or ICQ2k shortcut bar items
	const WORD ROSTER_IGNORE        = 0x000e; // a contact on the ignore list
	const WORD ROSTER_LASTUPDATE    = 0x000F; // Last update date (name: "LastUpdateDate")
	const WORD ROSTER_NONICQ        = 0x0010; // a non-icq contact, no UIN, used to send SMS
	const WORD ROSTER_IMPORTTIME    = 0x0013; // roster import time (name: "Import time")
	const WORD ROSTER_BUDDYICONS    = 0x0014; // Buddy icon info. (names: from "0" and incrementing by one)

    /* User classes/statuses */
    const WORD CLASS_UNCONFIRMED    = 0x0001; // AOL Unconfirmed user
    const WORD CLASS_ADMINISTRATOR  = 0x0002; // AOL Administrator
    const WORD CLASS_AOL            = 0x0004; // AOL Staff
    const WORD CLASS_COMMERCIAL     = 0x0008; // AOL commercial account
    const WORD CLASS_FREE           = 0x0010; // ICQ non-commercial account
    const WORD CLASS_AWAY           = 0x0020; // Away status
    const WORD CLASS_ICQ            = 0x0040; // ICQ user
    const WORD CLASS_WIRELESS       = 0x0080; // AOL wireless user
    const WORD CLASS_UNKNOWN100     = 0x0100; // Unknown
    const WORD CLASS_UNKNOWN400     = 0x0400; // Unknown
    const WORD CLASS_UNKNOWN800     = 0x0800; // Unknown

    const WORD STATUS_ONLINE        = 0x0000; // Online
    const WORD STATUS_AWAY          = 0x0001; // Away
    const WORD STATUS_DND           = 0x0002; // Do not Disturb
    const WORD STATUS_NA            = 0x0004; // Not Available
    const WORD STATUS_OCCUPIED      = 0x0010; // Occupied (BUSY/BISY)
    const WORD STATUS_FREE4CHAT     = 0x0020; // Free for chat
    const WORD STATUS_INVISIBLE     = 0x0100; // Invisible
}

#endif

//kate: tab-width 4; indent-mode csands;
