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

#include <qcstring.h>
#include <qdatastream.h>
#include <qmap.h>
#include <qobject.h>

#include <kdebug.h>

#include "ymsgprotocol.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"

using namespace Yahoo;

YMSGProtocol::YMSGProtocol(QObject *parent, const char *name)
 : InputProtocolBase(parent, name)
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
	kdDebug(14180) << k_funcinfo << packet << endl;
	
	int pos = 0;
	int len = 0;
	
	Yahoo::Status status;
	int statusnum = 0;
	int sessionid = 0;
	Yahoo::Service service;
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
	kdDebug(14180) << k_funcinfo << " - parsed packet version " << version1 << " " << version2 << endl;
	
	len = yahoo_get16(packet.data() + pos);
	pos += 2;
	kdDebug(14180) << k_funcinfo << " - parsed packet len " << len << endl;
	
	servicenum = yahoo_get16(packet.data() + pos);
	pos += 2;
	
	switch (servicenum)
	{
		// TODO add remamining services
		case (Yahoo::ServiceAuth) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceAuth " << servicenum << endl;
			service = Yahoo::ServiceAuth;
		break;
		case (Yahoo::ServiceAuthResp) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceAuthResp " << servicenum << endl;
			service = Yahoo::ServiceAuthResp;
		break;
		case (Yahoo::ServiceVerify) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceVerify " << servicenum << endl;
			service = Yahoo::ServiceVerify;
		break;
		case (Yahoo::ServiceList) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceList " << servicenum << endl;
			service = Yahoo::ServiceList;
		break;
		case (Yahoo::ServiceLogon) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceLogon " << servicenum << endl;
			service = Yahoo::ServiceLogon;
		break;
		case (Yahoo::ServicePing) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServicePing " << servicenum << endl;
			service = Yahoo::ServicePing;
		break;
		case (Yahoo::ServiceNewMail) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceNewMail " << servicenum << endl;
			service = Yahoo::ServiceNewMail;
		break;
		case (Yahoo::ServiceLogoff) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceLogoff " << servicenum << endl;
			service = Yahoo::ServiceLogoff;
		break;
		case (Yahoo::ServiceIsAway) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceIsAway " << servicenum << endl;
			service = Yahoo::ServiceIsAway;
		break;
		case (Yahoo::ServiceIsBack) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceIsBack " << servicenum << endl;
			service = Yahoo::ServiceIsBack;
		break;
		case (Yahoo::ServiceGameLogon) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceGameLogon " << servicenum << endl;
			service = Yahoo::ServiceGameLogon;
		break;
		case (Yahoo::ServiceGameLogoff) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceGameLogoff " << servicenum << endl;
			service = Yahoo::ServiceGameLogoff;
		break;
		case (Yahoo::ServiceIdAct) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceIdAct " << servicenum << endl;
			service = Yahoo::ServiceIdAct;
		break;
		case (Yahoo::ServiceIddeAct) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceIddeAct " << servicenum << endl;
			service = Yahoo::ServiceIddeAct;
		break;
		case (Yahoo::ServiceStatus) :
			kdDebug(14180) << k_funcinfo << " Parsed packet service -  This means ServiceStatus " << servicenum << endl;
			service = Yahoo::ServiceStatus;
		break;
		/*
		ServiceIdle, // 5 (placemarker)
		ServiceMessage,
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
		ServiceConfInvite = 0x18,
		ServiceConfLogon,
		ServiceConfDecline,
		ServiceConfLogoff,
		ServiceConfAddInvite,
		ServiceConfMsg,
		ServiceChatLogon,
		ServiceChatLogoff,
		ServiceChatMsg = 0x20,
		ServiceGameMsg = 0x2a,
		ServiceFileTransfer = 0x46,
		ServiceVoiceChat = 0x4A,
		ServiceNotify,
		ServiceVerify = 76,
		ServiceP2PFileXfer,
		ServicePeerToPeer = 0x4F,	// Checks if P2P possible 
		ServiceWebcam,
		ServiceAddBuddy = 0x83,
		ServiceRemBuddy,
		ServiceIgnoreContact,	// > 1, 7, 13 < 1, 66, 13, 0
		ServiceRejectContact,
		ServiceGroupRename = 0x89, // > 1, 65(new), 66(0), 67(old) 
		ServiceChatOnline = 0x96, // > 109(id), 1, 6(abcde) < 0,1
		ServiceChatGoto,
		ServiceChatJoin,	// > 1 104-room 129-1600326591 62-2
		ServiceChatleave,
		ServiceChatExit = 0x9b,
		ServiceChatLogout = 0xa0,
		ServiceChatPing,
		ServiceComment = 0xa8*/


		default:
			kdDebug(14180) << k_funcinfo << "  Parsed packet service -  This means an unknown service " << servicenum << endl;
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
			kdDebug(14180) << k_funcinfo << " - unknown status " << statusnum << endl;
		break;
	}
	
	sessionid = yahoo_get32(packet.data() + pos);
	kdDebug(14180) << k_funcinfo << "  Parsed session id: " << (void *)sessionid << endl;
	pos += 4;
	
	kdDebug(14180) << k_funcinfo << " Setting incoming transfer basic information." << endl;
	YMSGTransfer *t = new YMSGTransfer();
	t->setService(service);
	t->setId(sessionid);
	t->setStatus(status);
	
	// taken almost as is from libyahoo ;-)
	
	char *data = packet.data();
	while (pos + 1 < len + 20 /*header*/)
	{
		char *key = 0L, *value = 0L;
		int accept;
		int x;
		key = (char *) malloc(len + 1);
		x = 0;
		while (pos + 1 < len +20) {
			if ((BYTE) data[pos] == (BYTE)0xc0 && (BYTE) data[pos + 1] == (BYTE)0x80)
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
			kdDebug(14180) << k_funcinfo << " setting packet key [" << QString(key) << "] to " << QString(value) << endl;
			t->setParam(QString(key), QString(value));
			free(value);
			free(key);
		}
		else
		{
			kdDebug(14180) << k_funcinfo << " key not accepted" << endl;
		}
	}

	// Packets consisting of several YMSG-packets sometimes contain padding chars (0x00) -> filter out
	while( (BYTE)data[pos] == (BYTE) 0x00 )
		pos++;

	kdDebug(14180) << k_funcinfo << " Returning transfer" << endl;
	// tell them we have parsed offset bytes
	
	bytes = pos;
	return t;
}

#include "ymsgprotocol.moc"
