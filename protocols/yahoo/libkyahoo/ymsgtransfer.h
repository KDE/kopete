/*
    YMSG - Yahoo Protocol

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

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

#ifndef YAHOOYMSG_H
#define YAHOOYMSG_H

#include "transfer.h"

enum Service 
{ 
	/* these are easier to see in hex */
	SERVICE_LOGON = 1,
	SERVICE_LOGOFF,
	SERVICE_ISAWAY,
	SERVICE_ISBACK,
	SERVICE_IDLE, /* 5 (placemarker) */
	SERVICE_MESSAGE,
	SERVICE_IDACT,
	SERVICE_IDDEACT,
	SERVICE_MAILSTAT,
	SERVICE_USERSTAT, /* 0xa */
	SERVICE_NEWMAIL,
	SERVICE_CHATINVITE,
	SERVICE_CALENDAR,
	SERVICE_NEWPERSONALMAIL,
	SERVICE_NEWCONTACT,
	SERVICE_ADDIDENT, /* 0x10 */
	SERVICE_ADDIGNORE,
	SERVICE_PING,
	SERVICE_GOTGROUPRENAME, /* < 1, 36(old), 37(new) */
	SERVICE_SYSMESSAGE = 0x14,
	SERVICE_PASSTHROUGH2 = 0x16,
	SERVICE_CONFINVITE = 0x18,
	SERVICE_CONFLOGON,
	SERVICE_CONFDECLINE,
	SERVICE_CONFLOGOFF,
	SERVICE_CONFADDINVITE,
	SERVICE_CONFMSG,
	SERVICE_CHATLOGON,
	SERVICE_CHATLOGOFF,
	SERVICE_CHATMSG = 0x20,
	SERVICE_GAMELOGON = 0x28,
	SERVICE_GAMELOGOFF,
	SERVICE_GAMEMSG = 0x2a,
	SERVICE_FILETRANSFER = 0x46,
	SERVICE_VOICECHAT = 0x4A,
	SERVICE_NOTIFY,
	SERVICE_VERIFY,
	SERVICE_P2PFILEXFER,
	SERVICE_PEERTOPEER = 0x4F,	/* Checks if P2P possible */
	SERVICE_WEBCAM,
	SERVICE_AUTHRESP = 0x54,
	SERVICE_LIST,
	SERVICE_AUTH = 0x57,
	SERVICE_ADDBUDDY = 0x83,
	SERVICE_REMBUDDY,
	SERVICE_IGNORECONTACT,	/* > 1, 7, 13 < 1, 66, 13, 0*/
	SERVICE_REJECTCONTACT,
	SERVICE_GROUPRENAME = 0x89, /* > 1, 65(new), 66(0), 67(old) */
	SERVICE_CHATONLINE = 0x96, /* > 109(id), 1, 6(abcde) < 0,1*/
	SERVICE_CHATGOTO,
	SERVICE_CHATJOIN,	/* > 1 104-room 129-1600326591 62-2 */
	SERVICE_CHATLEAVE,
	SERVICE_CHATEXIT = 0x9b,
	SERVICE_CHATLOGOUT = 0xa0,
	SERVICE_CHATPING,
	SERVICE_COMMENT = 0xa8
};

/*class Buffer;*/

class YMSGTransferPrivate;

/**
@author Duncan Mac-Vicar Prett
*/
class YMSGTransfer : public Transfer
{
public:
	YMSGTransfer();
	~YMSGTransfer();


	TransferType type();

	//! Get the validity of the transfer object
	bool isValid();
private:

	YMSGTransferPrivate* d;
};

#endif
