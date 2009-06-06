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
#include <qstringlist.h>
#ifdef Q_OS_WIN
  // BYTE, WORD, DWORD
  #include <windows.h>
#endif

//! Debug Areas
const int OSCAR_RAW_DEBUG = 14151;
const int OSCAR_GEN_DEBUG = 14150;
const int OSCAR_AIM_DEBUG = 14152;
const int OSCAR_ICQ_DEBUG = 14153;

namespace Oscar
{
//! Capabilities
enum Capability
{
	CAP_CHAT = 0, CAP_VOICE, CAP_SENDFILE, CAP_DIRECT_ICQ_COMMUNICATION, CAP_IMIMAGE, CAP_BUDDYICON, CAP_SAVESTOCKS,
	CAP_GETFILE, CAP_ICQSERVERRELAY, CAP_GAMES, CAP_GAMES2, CAP_SENDBUDDYLIST, CAP_RTFMSGS, CAP_IS_2001,
	CAP_TRILLIAN, CAP_TRILLIANCRYPT, CAP_APINFO, CAP_UTF8, CAP_TYPING, CAP_INTEROPERATE,
	CAP_KOPETE, CAP_MIRANDA, CAP_QIP, CAP_QIPINFIUM, CAP_QIPPDA, CAP_QIPSYMBIAN, CAP_QIPMOBILE,
	CAP_JIMM, CAP_MICQ, CAP_MACICQ, CAP_SIMOLD, CAP_SIMNEW,
	CAP_VMICQ, CAP_LICQ, CAP_ANDRQ, CAP_RANDQ, CAP_MCHAT,
	CAP_XTRAZ, CAP_TZERS, CAP_HTMLMSGS,
	CAP_ICQ_RAMBLER, CAP_ICQ_ABV, CAP_ICQ_NETVIGATOR, CAP_STR_2001, CAP_STR_2002, CAP_XTRAZ_MULTIUSER_CHAT,
	CAP_DEVILS, CAP_NEWCAPS, CAP_UNKNOWN2, CAP_PUSH2TALK, CAP_VIDEO, CAP_LAST
};

Q_DECLARE_FLAGS( Capabilities, Capability )

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
	Guid( QByteArray::fromRawData( "Kopete ICQ      ", 16 ) ),

	// CAP_MARANDA
	Guid( QLatin1String( "4d6972616e64614d0000000000000000" ) ),

	// CAP_QIP
	Guid( QLatin1String( "563fc8090b6f41514950203230303561" ) ),

	// CAP_QIPINFIUM
	Guid( QLatin1String( "7C737502C3BE4F3EA69F015313431E1A" ) ),

	// CAP_QIPPDA
	Guid( QLatin1String( "563FC8090B6F41514950202020202021" ) ),

	// CAP_QIPSYMBIAN
	Guid( QLatin1String( "51ADD1907204473DA1A149F4A397A41F" ) ),

	// CAP_QIPMOBILE
	Guid( QLatin1String( "B08262F67F7C4561ADC11C6D75705EC5" ) ),

	// CAP_JIMM
	Guid( QByteArray::fromRawData( "Jimm ", 16 ) ),

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

	// CAP_VMICQ
	Guid( QLatin1String( "566d4943512000000000000000000000" ) ),

	// CAP_LICQ
	Guid( QLatin1String( "4c69637120636c69656e742000000000" ) ),

	// CAP_ANDRQ
	Guid( QLatin1String( "265251696e7369646500000000000000" ) ),

	// CAP_RANDQ
	Guid( QLatin1String( "522651696e7369646500000000000000" ) ),

	// CAP_MCHAT
	Guid( QLatin1String( "6D436861742069637120000000000000" ) ),

	// CAP_XTRAZ
	Guid( QLatin1String( "1A093C6CD7FD4EC59D51A6474E34F5A0" ) ),

	// CAP_TZERS
	Guid( QLatin1String( "B2EC8F167C6F451BBD79DC58497888B9" ) ),

	// CAP_HTMLMSGS
	Guid( QLatin1String( "0138CA7B769A491588F213FC00979EA8" ) ),

	// CAP_ICQ_RAMBLER
	Guid( QLatin1String( "7e11b778a3534926a80244735208c42a" ) ),

	// CAP_ICQ_ABV
	Guid( QLatin1String( "00E7E0DFA9D04fe19162c8909A132A1B" ) ),

	// CAP_ICQ_NETVIGATOR
	Guid( QLatin1String( "4C6B90A33D2D480E89D62E4B2C10D99F" ) ),

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

	// CAP_UNKNOWN2
	Guid( QLatin1String( "B99708B53A924202B069F1E757BB2E17" ) ),

	// CAP_PUSH2TALK
	Guid( QLatin1String( "E362C1E9121A4B94A6267A74DE24270D" ) ),

	// CAP_VIDEO
	Guid( QLatin1String( "B6074378F50C4AC790925938502D0591" ) ),

	// CAP_LAST,
	Guid( QLatin1String( "00000000000000000000000000000000" ) )
};

enum XStatus
{
	XSTAT_ANGRY = 0, XSTAT_DUCK, XSTAT_TIRED, XSTAT_PARTY, XSTAT_BEER, XSTAT_THINKING, XSTAT_EATING, XSTAT_TV,
	XSTAT_FRIENDS, XSTAT_COFFEE, XSTAT_MUSIC, XSTAT_BUSINESS, XSTAT_CAMERA, XSTAT_FUNNY, XSTAT_PHONE, XSTAT_GAMES,
	XSTAT_STUDYING, XSTAT_SHOPPING, XSTAT_SICK, XSTAT_SLEEPING, XSTAT_SURFING, XSTAT_BROWSING, XSTAT_WORKING,
	XSTAT_TYPING, XSTAT_PICNIC, XSTAT_COOKING, XSTAT_SMOKING, XSTAT_I_AM_HIGH, XSTAT_WC, XSTAT_TO_BE_OR_NOT_TO_BE,
	XSTAT_WATCHING_PRO7, XSTAT_LOVE, XSTAT_LAST

};

const Guid oscar_xStatus[] =
{
	// XSTAT_ANGRY
	Guid( QLatin1String( "01D8D7EEAC3B492AA58DD3D877E66B92" ) ),

	// XSTAT_DUCK
	Guid( QLatin1String( "5A581EA1E580430CA06F612298B7E4C7" ) ),

	// XSTAT_TIRED
	Guid( QLatin1String( "83C9B78E77E74378B2C5FB6CFCC35BEC" ) ),

	// XSTAT_PARTY
	Guid( QLatin1String( "E601E41C33734BD1BC06811D6C323D81" ) ),

	// XSTAT_BEER
	Guid( QLatin1String( "8C50DBAE81ED4786ACCA16CC3213C7B7" ) ),

	// XSTAT_THINKING
	Guid( QLatin1String( "3FB0BD36AF3B4A609EEFCF190F6A5A7F" ) ),

	// XSTAT_EATING
	Guid( QLatin1String( "F8E8D7B282C4414290F810C6CE0A89A6" ) ),

	// XSTAT_TV
	Guid( QLatin1String( "80537DE2A4674A76B3546DFD075F5EC6" ) ),

	// XSTAT_FRIENDS
	Guid( QLatin1String( "F18AB52EDC57491D99DC6444502457AF" ) ),

	// XSTAT_COFFEE
	Guid( QLatin1String( "1B78AE31FA0B4D3893D1997EEEAFB218" ) ),

	// XSTAT_MUSIC
	Guid( QLatin1String( "61BEE0DD8BDD475D8DEE5F4BAACF19A7" ) ),

	// XSTAT_BUSINESS
	Guid( QLatin1String( "488E14898ACA4A0882AA77CE7A165208" ) ),

	// XSTAT_CAMERA
	Guid( QLatin1String( "107A9A1812324DA4B6CD0879DB780F09" ) ),

	// XSTAT_FUNNY
	Guid( QLatin1String( "6F4930984F7C4AFFA27634A03BCEAEA7" ) ),

	// XSTAT_PHONE
	Guid( QLatin1String( "1292E5501B644F66B206B29AF378E48D" ) ),

	// XSTAT_GAMES
	Guid( QLatin1String( "D4A611D08F014EC09223C5B6BEC6CCF0" ) ),

	// XSTAT_STUDYING
	Guid( QLatin1String( "609D52F8A29A49A6B2A02524C5E9D260" ) ),

	// XSTAT_SHOPPING
	Guid( QLatin1String( "63627337A03F49FF80E5F709CDE0A4EE" ) ),

	// XSTAT_SICK
	Guid( QLatin1String( "1F7A4071BF3B4E60BC324C5787B04CF1" ) ),

	// XSTAT_SLEEPING
	Guid( QLatin1String( "785E8C4840D34C65886F04CF3F3F43DF" ) ),

	// XSTAT_SURFING
	Guid( QLatin1String( "A6ED557E6BF744D4A5D4D2E7D95CE81F" ) ),

	// XSTAT_BROWSING
	Guid( QLatin1String( "12D07E3EF885489E8E97A72A6551E58D" ) ),

	// XSTAT_WORKING
	Guid( QLatin1String( "BA74DB3E9E24434B87B62F6B8DFEE50F" ) ),

	// XSTAT_TYPING
	Guid( QLatin1String( "634F6BD8ADD24AA1AAB9115BC26D05A1" ) ),

	// XSTAT_PICNIC
	Guid( QLatin1String( "2CE0E4E57C6443709C3A7A1CE878A7DC" ) ),

	// XSTAT_COOKING
	Guid( QLatin1String( "101117C9A3B040F981AC49E159FBD5D4" ) ),

	// XSTAT_SMOKING
	Guid( QLatin1String( "160C60BBDD4443F39140050F00E6C009" ) ),

	// XSTAT_I_AM_HIGH
	Guid( QLatin1String( "6443C6AF22604517B58CD7DF8E290352" ) ),

	// XSTAT_WC
	Guid( QLatin1String( "16F5B76FA9D240358CC5C084703C98FA" ) ),

	// XSTAT_TO_BE_OR_NOT_TO_BE
	Guid( QLatin1String( "631436FF3F8A40D0A5CB7B66E051B364" ) ),

	// XSTAT_WATCHING_PRO7
	Guid( QLatin1String( "B70867F538254327A1FFCF4CC1939797" ) ),

	// XSTAT_LOVE
	Guid( QLatin1String( "DDCF0EA971954048A9C6413206D6F280" ) ),

	// XSTAT_LAST
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
	WORD fileCount;
	WORD filesLeft;
	WORD partCount;
	WORD partsLeft;
	DWORD totalSize;
};

struct OFTRendezvous
{
	QByteArray cookie;
	WORD fileCount;
	DWORD totalSize;
	QString fileName;

	QStringList files;
	QString dir;
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
	const unsigned char ICQ_TCP_VERSION 	= 0x0009; // 9 for rtf support

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

Q_DECLARE_OPERATORS_FOR_FLAGS(Oscar::Capabilities)

#endif

//kate: tab-width 4; indent-mode csands;
