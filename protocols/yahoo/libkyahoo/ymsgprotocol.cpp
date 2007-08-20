/*
    Kopete Yahoo Protocol

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>
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

#include <stdlib.h>

#include <qdatastream.h>
#include <qmap.h>
#include <qobject.h>

#include <kdebug.h>

#include "ymsgprotocol.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"

using namespace Yahoo;

YMSGProtocol::YMSGProtocol(QObject *parent)
 : InputProtocolBase(parent)
{
}

YMSGProtocol::~YMSGProtocol()
{
}

Transfer* YMSGProtocol::parse( const QByteArray & packet, uint& bytes )
{
	/*
	<------- 4B -------><------- 4B -------><---2B--->
	+-------------------+-------------------+---------+
	|   Y   M   S   G   |      version      | pkt_len |
	+---------+---------+---------+---------+---------+
	| service |      status       |    session_id     |
	+---------+-------------------+-------------------+
	|                                                 |
	:                    D A T A                      :
	/                   0 - 65535*                   |
	+-------------------------------------------------+
	*/
	kDebug(YAHOO_RAW_DEBUG) << packet;
	
	int pos = 0;
	int len = 0;
	
	Yahoo::Status status = Yahoo::StatusAvailable;
	Yahoo::Service service = Yahoo::ServiceAuth;
	int statusnum = 0;
	int sessionid = 0;
	int servicenum;
	int version1, version2;
	
	QMap<QString, QString> params;
	
	// Skip the YMSG header
	pos += 4;
	
	// Skip the version
	version1 = yahoo_get16(packet.data() + pos);
	pos += 2;
	version2 = yahoo_get16(packet.data() + pos);
	pos += 2;
	kDebug(YAHOO_RAW_DEBUG) << " - parsed packet version " << version1 << " " << version2;
	
	len = yahoo_get16(packet.data() + pos);
	pos += 2;
	kDebug(YAHOO_RAW_DEBUG) << " - parsed packet len " << len;
	
	servicenum = yahoo_get16(packet.data() + pos);
	pos += 2;
	
	switch (servicenum)
	{
		// TODO add remamining services
		case (Yahoo::ServiceAuth) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceAuth " << servicenum;
			service = Yahoo::ServiceAuth;
		break;
		case (Yahoo::ServiceAuthResp) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceAuthResp " << servicenum;
			service = Yahoo::ServiceAuthResp;
		break;
		case (Yahoo::ServiceVerify) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceVerify " << servicenum;
			service = Yahoo::ServiceVerify;
		break;
		case (Yahoo::ServiceList) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceList " << servicenum;
			service = Yahoo::ServiceList;
		break;
		case (Yahoo::ServiceLogon) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceLogon " << servicenum;
			service = Yahoo::ServiceLogon;
		break;
		case (Yahoo::ServicePing) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePing " << servicenum;
			service = Yahoo::ServicePing;
		break;
		case (Yahoo::ServiceNewMail) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceNewMail " << servicenum;
			service = Yahoo::ServiceNewMail;
		break;
		case (Yahoo::ServiceLogoff) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceLogoff " << servicenum;
			service = Yahoo::ServiceLogoff;
		break;
		case (Yahoo::ServiceIsAway) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceIsAway " << servicenum;
			service = Yahoo::ServiceIsAway;
		break;
		case (Yahoo::ServiceIsBack) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceIsBack " << servicenum;
			service = Yahoo::ServiceIsBack;
		break;
		case (Yahoo::ServiceGameLogon) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceGameLogon " << servicenum;
			service = Yahoo::ServiceGameLogon;
		break;
		case (Yahoo::ServiceGameLogoff) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceGameLogoff " << servicenum;
			service = Yahoo::ServiceGameLogoff;
		break;
		case (Yahoo::ServiceIdAct) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceIdAct " << servicenum;
			service = Yahoo::ServiceIdAct;
		break;
		case (Yahoo::ServiceIddeAct) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceIddeAct " << servicenum;
			service = Yahoo::ServiceIddeAct;
		break;
		case (Yahoo::ServiceStatus) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceStatus " << servicenum;
			service = Yahoo::ServiceStatus;
		break;
		case (Yahoo::ServiceMessage) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceMessage " << servicenum;
			service = Yahoo::ServiceMessage;
		break;
		case (Yahoo::ServiceNotify) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceNotify " << servicenum;
			service = Yahoo::ServiceNotify;
		break;
		case (Yahoo::ServiceAddBuddy) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceAddBuddy " << servicenum;
			service = Yahoo::ServiceAddBuddy;
		break;
		case (Yahoo::ServicePictureChecksum) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePictureChecksum " << servicenum;
			service = Yahoo::ServicePictureChecksum;
		break;
		case (Yahoo::ServicePictureStatus) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePictureStatus " << servicenum;
			service = Yahoo::ServicePictureStatus;
		break;
		case (Yahoo::ServicePicture) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePicture " << servicenum;
			service = Yahoo::ServicePicture;
		break;
		case (Yahoo::ServiceStealthOnline) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceStealthOnline " << servicenum;
			service = Yahoo::ServiceStealthOnline;
		break;
		case (Yahoo::ServiceStealthOffline) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceStealthOffline " << servicenum;
			service = Yahoo::ServiceStealthOffline;
		break;
		case (Yahoo::ServicePictureUpload) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePictureUpload " << servicenum;
			service = Yahoo::ServicePictureUpload;
		break;
		case (Yahoo::ServiceWebcam) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceWebcam " << servicenum;
			service = Yahoo::ServiceWebcam;
		break;
		case (Yahoo::ServiceConfInvite) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfInvite " << servicenum;
			service = Yahoo::ServiceConfInvite;
		break;
		case (Yahoo::ServiceConfLogon) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfLogon " << servicenum;
			service = Yahoo::ServiceConfLogon;
		break;
		case (Yahoo::ServiceConfDecline) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfDecline " << servicenum;
			service = Yahoo::ServiceConfDecline;
		break;
		case (Yahoo::ServiceConfLogoff) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfLogoff " << servicenum;
			service = Yahoo::ServiceConfLogoff;
		break;
		case (Yahoo::ServiceConfAddInvite) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfAddInvite " << servicenum;
			service = Yahoo::ServiceConfAddInvite;
		break;
		case (Yahoo::ServiceConfMsg) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceConfMsg " << servicenum;
			service = Yahoo::ServiceConfMsg;
		break;
		case (Yahoo::ServiceAuthorization) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceAuthorization " << servicenum;
			service = Yahoo::ServiceAuthorization;
		break;
		case (Yahoo::ServiceContactDetails) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceContactDetails " << servicenum;
			service = Yahoo::ServiceContactDetails;
		break;
		case (Yahoo::ServiceFileTransfer) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceFileTransfer " << servicenum;
			service = Yahoo::ServiceFileTransfer;
		break;
		case (Yahoo::ServiceFileTransfer7) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceFileTransfer7 " << servicenum;
			service = Yahoo::ServiceFileTransfer7;
		break;
		case (Yahoo::ServiceFileTransfer7Info) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceFileTransfer7Info " << servicenum;
			service = Yahoo::ServiceFileTransfer7Info;
		break;
		case (Yahoo::ServicePeerToPeer) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePeerToPeer " << servicenum;
			service = Yahoo::ServicePeerToPeer;
		break;
		case (Yahoo::ServiceChatOnline) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatOnline " << servicenum;
			service = Yahoo::ServiceChatOnline;
		break;
		case (Yahoo::ServiceChatGoto) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatGoto " << servicenum;
			service = Yahoo::ServiceChatGoto;
		break;
		case (Yahoo::ServiceChatJoin) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatJoin " << servicenum;
			service = Yahoo::ServiceChatJoin;
		break;
		case (Yahoo::ServiceChatleave) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatleave " << servicenum;
			service = Yahoo::ServiceChatleave;
		break;
		case (Yahoo::ServiceChatExit) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatExit " << servicenum;
			service = Yahoo::ServiceChatExit;
		break;
		case (Yahoo::ServiceChatLogout) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatLogout " << servicenum;
			service = Yahoo::ServiceChatLogout;
		break;
		case (Yahoo::ServiceChatPing) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServicePeerToPeer " << servicenum;
			service = Yahoo::ServiceChatPing;
		break;
		case (Yahoo::ServiceChatLogon) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatLogon " << servicenum;
			service = Yahoo::ServiceChatLogon;
		break;
		case (Yahoo::ServiceChatLogoff) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatLogoff " << servicenum;
			service = Yahoo::ServiceChatLogoff;
		break;
		case (Yahoo::ServiceChatMsg) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceChatMsg " << servicenum;
			service = Yahoo::ServiceChatMsg;
		break;
		case (Yahoo::ServiceComment) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceComment " << servicenum;
			service = Yahoo::ServiceComment;
		break;
		case (Yahoo::ServiceBuddyStatus) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceBuddyStatus " << servicenum;
			service = Yahoo::ServiceBuddyStatus;
		break;
		case (Yahoo::ServiceBuddyList) :
			kDebug(YAHOO_RAW_DEBUG) << " Parsed packet service -  This means ServiceBuddyList " << servicenum;
			service = Yahoo::ServiceBuddyList;
		break;
		/*
		ServiceIdle, // 5 (placemarker)
		ServiceMailStat,
		ServiceUserStat, // 0xa
		ServiceChatInvite,
		ServiceCalendar,
		ServiceNewPersonalMail,
		ServiceNewContact,
		ServiceAddIdent, // 0x10
		ServiceAddIgnore,
		ServiceGotGroupRename, // < 1, 36(old), 37(new)
		ServiceSysMessage = 0x14,
		ServicePassThrough2 = 0x16,
		ServiceGameMsg = 0x2a,
		ServiceFileTransfer = 0x46,
		ServiceVoiceChat = 0x4A,
		ServiceVerify = 76,
		ServiceP2PFileXfer,
		ServiceRemBuddy,
		ServiceIgnoreContact,	// > 1, 7, 13 < 1, 66, 13, 0
		ServiceRejectContact,
		ServiceGroupRename = 0x89, // > 1, 65(new), 66(0), 67(old) 
		ServicePictureUpdate = 0xc1,
		ServiceVisibility = 0xc5,	// YMSG13, key 13: 2 = invisible, 1 = visible 
		ServiceStatus = 0xc6,		// YMSG13 
		*/

		default:
			kDebug(YAHOO_RAW_DEBUG) << "  Parsed packet service -  This means an unknown service " << servicenum;
		break;
	}
	
	statusnum = yahoo_get32(packet.data() + pos);
	pos += 4;
	
	switch (statusnum)
	{
		// TODO add remaining status
		case (Yahoo::StatusAvailable) :
			status = Yahoo::StatusAvailable;
		break;
		case (Yahoo::StatusBRB) :
			status = Yahoo::StatusBRB;
		break;
		case (Yahoo::StatusDisconnected) :
			status = Yahoo::StatusDisconnected;
		break;
		/*StatusBusy
		StatusNotAtHome
		StatusNotAtDesk
		StatusNotInOffice
		StatusOnPhone
		StatusOnVacation
		StatusOutToLunch
		StatusSteppedOut
		StatusInvisible
		StatusCustom
		StatusIdle
		StatusOffline
		StatusNotify*/
		default:
			kDebug(YAHOO_RAW_DEBUG) << " - unknown status " << statusnum;
		break;
	}
	
	sessionid = yahoo_get32(packet.data() + pos);
	kDebug(YAHOO_RAW_DEBUG) << "  Parsed session id: " << (void *)sessionid;
	pos += 4;
	
	kDebug(YAHOO_RAW_DEBUG) << " Setting incoming transfer basic information.";
	YMSGTransfer *t = new YMSGTransfer();
	t->setService(service);
	t->setId(sessionid);
	t->setStatus(status);
	t->setPacketLength(len);
	
	// taken almost as is from libyahoo ;-)
	
	char *data = (char*)packet.data();
	while (pos + 1 < len + 20 /*header*/)
	{
		if( (BYTE) data[pos] == (BYTE)0x00  )
			break;
	
		char *key = 0L, *value = 0L;
		int accept;
		int x;
		key = (char *) malloc(len + 1);
		x = 0;
		while (pos + 1 < len +20) {
			if ( ((BYTE) data[pos] == (BYTE)0xc0 && (BYTE) data[pos + 1] == (BYTE)0x80) )
				break;
			key[x++] = data[pos++];
		}
		key[x] = 0;
		pos += 2;

		accept = x;
		
		/* if x is 0 there was no key, so don't accept it */
		if (accept)
			value = (char *) malloc(len - pos + 20 + 1);
		
		x = 0;
		while (pos + 1 < len + 20 /* header */)
		{
			if ((BYTE) data[pos] == (BYTE) 0xc0 && (BYTE) data[pos + 1] == (BYTE) 0x80)
				break;
			if (accept)
				value[x++] = data[pos++];
		}
		if (accept)
			value[x] = 0;
		pos += 2;

		if (accept) 
		{
			kDebug(YAHOO_RAW_DEBUG) << " setting packet key [" << QString(key) << "] to " << QString(value);
			t->setParam(QString(key).toInt(), value);
			free(value);
		}
		else
		{
			kDebug(YAHOO_RAW_DEBUG) << " key not accepted";
		}
		free(key);
	}

	// Packets consisting of several YMSG-packets sometimes contain padding chars (0x00) -> filter out
 	while( (BYTE)data[pos] == (BYTE) 0x00 && pos <= len + 20)
 		pos++;

// 	kDebug(YAHOO_RAW_DEBUG) << " Returning transfer";
	// tell them we have parsed offset bytes
	
	bytes = pos;
	return t;
}

#include "ymsgprotocol.moc"
