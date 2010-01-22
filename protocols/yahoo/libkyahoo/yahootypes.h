/*
    yahootypes.h - Kopete Yahoo Protocol definitions

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOTYPESH
#define YAHOOTYPESH

#include <qglobal.h>
#include <QString>

const int YAHOO_RAW_DEBUG = 14181;
const int YAHOO_GEN_DEBUG = 14180;

namespace Yahoo
{
	enum Service 
	{ 
		/* these are easier to see in hex */
		ServiceLogon = 1,
		ServiceLogoff,
		ServiceIsAway,
		ServiceIsBack,
		ServiceIdle, /* 5 (placemarker) */
		ServiceMessage,
		ServiceIdAct,
		ServiceIddeAct,
		ServiceMailStat,
		ServiceUserStat, /* 0xa */
		ServiceNewMail,
		ServiceChatInvite,
		ServiceCalendar,
		ServiceNewPersonalMail,
		ServiceNewContact,
		ServiceAddIdent, /* 0x10 */
		ServiceAddIgnore,
		ServicePing,
		ServiceGotGroupRename, /* < 1, 36(old), 37(new) */
		ServiceSysMessage = 0x14,
		ServicePassThrough2 = 0x16,
		ServiceConfInvite = 0x18,
		ServiceConfLogon,
		ServiceConfDecline,
		ServiceConfLogoff,
		ServiceConfAddInvite,
		ServiceConfMsg,
		ServiceChatLogon,
		ServiceChatLogoff,
		ServiceChatMsg = 0x20,
		ServiceGameLogon = 0x28,
		ServiceGameLogoff,
		ServiceGameMsg = 0x2a,
		ServiceFileTransfer = 0x46,
		ServiceVoiceChat = 0x4A,
		ServiceNotify,
		ServiceVerify = 76,
		ServiceP2PFileXfer,
		ServicePeerToPeer = 0x4F,	/* Checks if P2P possible */
		ServiceWebcam,
		ServiceAuthResp = 0x54,
		ServiceList = 85,
		ServiceAuth = 0x57,
		ServiceBuddyAdd = 0x83,
		ServiceBuddyRemove = 0x84,
		ServiceIgnoreContact,	/* > 1, 7, 13 < 1, 66, 13, 0*/
		ServiceRejectContact,
		ServiceGroupRename = 0x89, /* > 1, 65(new), 66(0), 67(old) */
		ServicePing7 = 0x8a,
		ServiceChatOnline = 0x96, /* > 109(id), 1, 6(abcde) < 0,1*/
		ServiceChatGoto,
		ServiceChatJoin,	/* > 1 104-room 129-1600326591 62-2 */
		ServiceChatleave,
		ServiceChatExit = 0x9b,
		ServiceChatLogout = 0xa0,
		ServiceChatPing,
		ServiceComment = 0xa8,
		ServiceStealthOffline = 0xb9,
		ServiceStealthOnline = 0xba,
		ServicePictureChecksum = 0xbd,
		ServicePicture = 0xbe,
		ServicePictureUpdate = 0xc1,
		ServicePictureUpload = 0xc2,
		ServiceVisibility = 0xc5,	/* YMSG13, key 13: 2 = invisible, 1 = visible */
		ServiceStatus = 0xc6,		/* YMSG13 */
		ServicePictureStatus = 0xc7,	/* YMSG13, key 213: 0 = none, 1 = avatar, 2 = picture */
		ServiceAnimatedAudibleIcon = 0xd0,	/* YMSG17 * items 230 is a baseaddress, 231 is the text included,232 unknownitem  added by michaelacole*/
		ServiceContactDetails = 0xd3,	/* YMSG13 */
		ServiceChatSession = 0xd4,	
		ServiceAuthorization = 0xd6,	/* YMSG13 */
		ServiceFileTransfer7 = 0xdc,	/* YMSG13 */
		ServiceFileTransfer7Info = 0xdd,	/* YMSG13 */
		ServiceFileTransfer7Accept = 0xde,	/* YMSG13 */
		ServiceBuddyChangeGroup = 0xe7,	/* YMSG13 */
		ServiceBuddyStatus = 0xf0,
		ServiceBuddyList = 0xf1
	};
	
	enum Status 
	{
		StatusConnecting = -2,
		StatusDisconnected = -1,
		StatusAvailable = 0,
		StatusBRB = 1,
		StatusBusy = 2,
		StatusNotAtHome,
		StatusNotAtDesk,
		StatusNotInOffice,
		StatusOnPhone,
		StatusOnVacation,
		StatusOutToLunch,
		StatusSteppedOut,
		StatusOnSMS = 10,
		StatusInvisible = 12,
		StatusCustom = 99,
		StatusIdle = 999,
		StatusWebLogin = 0x5a55aa55,
		StatusOffline = 0x5a55aa56, /* don't ask */
		StatusNotify = 0x16
	};

	enum StatusType
	{
		StatusTypeAvailable = 0,
		StatusTypeAway
	};

	enum LoginStatus {
		LoginOk = 0,
		LoginUname = 3,
		LoginPasswd = 13,
		LoginLock = 14,
		LoginVerify = 29,	// FIXME: Find the reason for this response
		LoginDupl = 99,
		LoginSock = -1
	};

	enum StealthMode {
		StealthOnline,
		StealthOffline,
		StealthPermOffline
	};

	enum StealthStatus {
		StealthActive = 1,
		StealthNotActive = 2,
		StealthClear = 3
	};

	enum Response {
		ResponseAccept,
		ResponseDecline
	};

	enum PictureStatus {
		NoPicture = 0,
		Avatar = 1,
		Picture = 2
	};

	typedef quint8 BYTE;
	typedef quint16 WORD;
	typedef quint32 DWORD;

	struct ChatRoom {
		QString name;
		QString topic;
		int id;
	};

	struct ChatCategory {
		QString name;
		int id;
	};
}

#define yahoo_put16(buf, data) ( \
		(*(buf) = (unsigned char)((data)>>8)&0xff), \
		(*((buf)+1) = (unsigned char)(data)&0xff),  \
		2)
#define yahoo_get16(buf) ((((*(buf))&0xff)<<8) + ((*((buf)+1)) & 0xff))
#define yahoo_put32(buf, data) ( \
		(*((buf)) = (unsigned char)((data)>>24)&0xff), \
		(*((buf)+1) = (unsigned char)((data)>>16)&0xff), \
		(*((buf)+2) = (unsigned char)((data)>>8)&0xff), \
		(*((buf)+3) = (unsigned char)(data)&0xff), \
		4)
#define yahoo_get32(buf) ((((*(buf)   )&0xff)<<24) + \
			 (((*((buf)+1))&0xff)<<16) + \
			 (((*((buf)+2))&0xff)<< 8) + \
			 (((*((buf)+3))&0xff)))

#endif
