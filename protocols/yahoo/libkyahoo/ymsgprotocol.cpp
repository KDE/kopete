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
#include <qstringlist.h>

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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << packet << endl;
	
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
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " - parsed packet version " << version1 << " " << version2 << endl;
	
	len = yahoo_get16(packet.data() + pos);
	pos += 2;
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " - parsed packet len " << len << endl;
	
	servicenum = yahoo_get16(packet.data() + pos);
	pos += 2;
	
	switch (servicenum)
	{
		// TODO add remamining services
		case (Yahoo::ServiceAuth) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceAuth " << servicenum << endl;
			service = Yahoo::ServiceAuth;
		break;
		case (Yahoo::ServiceAuthResp) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceAuthResp " << servicenum << endl;
			service = Yahoo::ServiceAuthResp;
		break;
		case (Yahoo::ServiceVerify) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceVerify " << servicenum << endl;
			service = Yahoo::ServiceVerify;
		break;
		case (Yahoo::ServiceList) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceList " << servicenum << endl;
			service = Yahoo::ServiceList;
		break;
		case (Yahoo::ServiceLogon) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceLogon " << servicenum << endl;
			service = Yahoo::ServiceLogon;
		break;
		case (Yahoo::ServicePing) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePing " << servicenum << endl;
			service = Yahoo::ServicePing;
		break;
		case (Yahoo::ServiceNewMail) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceNewMail " << servicenum << endl;
			service = Yahoo::ServiceNewMail;
		break;
		case (Yahoo::ServiceLogoff) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceLogoff " << servicenum << endl;
			service = Yahoo::ServiceLogoff;
		break;
		case (Yahoo::ServiceIsAway) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceIsAway " << servicenum << endl;
			service = Yahoo::ServiceIsAway;
		break;
		case (Yahoo::ServiceIsBack) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceIsBack " << servicenum << endl;
			service = Yahoo::ServiceIsBack;
		break;
		case (Yahoo::ServiceGameLogon) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceGameLogon " << servicenum << endl;
			service = Yahoo::ServiceGameLogon;
		break;
		case (Yahoo::ServiceGameLogoff) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceGameLogoff " << servicenum << endl;
			service = Yahoo::ServiceGameLogoff;
		break;
		case (Yahoo::ServiceIdAct) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceIdAct " << servicenum << endl;
			service = Yahoo::ServiceIdAct;
		break;
		case (Yahoo::ServiceIddeAct) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceIddeAct " << servicenum << endl;
			service = Yahoo::ServiceIddeAct;
		break;
		case (Yahoo::ServiceStatus) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceStatus " << servicenum << endl;
			service = Yahoo::ServiceStatus;
		break;
		case (Yahoo::ServiceMessage) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceMessage " << servicenum << endl;
			service = Yahoo::ServiceMessage;
		break;
		case (Yahoo::ServiceNotify) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceNotify " << servicenum << endl;
			service = Yahoo::ServiceNotify;
		break;
		case (Yahoo::ServiceAddBuddy) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceAddBuddy " << servicenum << endl;
			service = Yahoo::ServiceAddBuddy;
		break;
		case (Yahoo::ServicePictureChecksum) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePictureChecksum " << servicenum << endl;
			service = Yahoo::ServicePictureChecksum;
		break;
		case (Yahoo::ServicePictureStatus) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePictureStatus " << servicenum << endl;
			service = Yahoo::ServicePictureStatus;
		break;
		case (Yahoo::ServicePicture) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePicture " << servicenum << endl;
			service = Yahoo::ServicePicture;
		break;
		case (Yahoo::ServiceStealthOnline) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceStealthOnline " << servicenum << endl;
			service = Yahoo::ServiceStealthOnline;
		break;
		case (Yahoo::ServiceStealthOffline) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceStealthOffline " << servicenum << endl;
			service = Yahoo::ServiceStealthOffline;
		break;
		case (Yahoo::ServicePictureUpload) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePictureUpload " << servicenum << endl;
			service = Yahoo::ServicePictureUpload;
		break;
		case (Yahoo::ServiceWebcam) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceWebcam " << servicenum << endl;
			service = Yahoo::ServiceWebcam;
		break;
		case (Yahoo::ServiceConfInvite) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfInvite " << servicenum << endl;
			service = Yahoo::ServiceConfInvite;
		break;
		case (Yahoo::ServiceConfLogon) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfLogon " << servicenum << endl;
			service = Yahoo::ServiceConfLogon;
		break;
		case (Yahoo::ServiceConfDecline) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfDecline " << servicenum << endl;
			service = Yahoo::ServiceConfDecline;
		break;
		case (Yahoo::ServiceConfLogoff) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfLogoff " << servicenum << endl;
			service = Yahoo::ServiceConfLogoff;
		break;
		case (Yahoo::ServiceConfAddInvite) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfAddInvite " << servicenum << endl;
			service = Yahoo::ServiceConfAddInvite;
		break;
		case (Yahoo::ServiceConfMsg) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceConfMsg " << servicenum << endl;
			service = Yahoo::ServiceConfMsg;
		break;
		case (Yahoo::ServiceAuthorization) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceAuthorization " << servicenum << endl;
			service = Yahoo::ServiceAuthorization;
		break;
		case (Yahoo::ServiceContactDetails) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceContactDetails " << servicenum << endl;
			service = Yahoo::ServiceContactDetails;
		break;
		case (Yahoo::ServiceFileTransfer) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceFileTransfer " << servicenum << endl;
			service = Yahoo::ServiceFileTransfer;
		break;
		case (Yahoo::ServiceFileTransfer7) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceFileTransfer7 " << servicenum << endl;
			service = Yahoo::ServiceFileTransfer7;
		break;
		case (Yahoo::ServiceFileTransfer7Info) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServiceFileTransfer7Info " << servicenum << endl;
			service = Yahoo::ServiceFileTransfer7Info;
		break;
		case (Yahoo::ServicePeerToPeer) :
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Parsed packet service -  This means ServicePeerToPeer " << servicenum << endl;
			service = Yahoo::ServicePeerToPeer;
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
		ServiceChatLogon,
		ServiceChatLogoff,
		ServiceChatMsg = 0x20,
		ServiceGameMsg = 0x2a,
		ServiceFileTransfer = 0x46,
		ServiceVoiceChat = 0x4A,
		ServiceVerify = 76,
		ServiceP2PFileXfer,
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
		ServiceComment = 0xa8
		ServicePictureUpdate = 0xc1,
		ServiceVisibility = 0xc5,	// YMSG13, key 13: 2 = invisible, 1 = visible 
		ServiceStatus = 0xc6,		// YMSG13 
		*/

		default:
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "  Parsed packet service -  This means an unknown service " << servicenum << endl;
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
			kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " - unknown status " << statusnum << endl;
		break;
	}
	
	sessionid = yahoo_get32(packet.data() + pos);
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "  Parsed session id: " << (void *)sessionid << endl;
	pos += 4;
	
	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Setting incoming transfer basic information." << endl;
	YMSGTransfer *t = new YMSGTransfer();
	t->setService(service);
	t->setId(sessionid);
	t->setStatus(status);

	QString d = QString::fromAscii( packet.data() + pos, packet.size() - pos );
	QStringList list;
	list = QStringList::split( "\xc0\x80", d );
	for( uint i = 0; i+1 < list.size() && pos+1 < len+20; i += 2 ) {
		QString key = list[i];
		QString value = QString::fromUtf8( list[i+1].ascii() );
		pos += key.utf8().length() + value.utf8().length() + 4;
		t->setParam( QString(key).toInt(), value.utf8() );
		kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << "Key: " << key << " Value: " << value << endl;
	}	

	while( (uint)pos < packet.size() && packet.data()[pos] == '\x00' )
		pos++;

	kdDebug(YAHOO_RAW_DEBUG) << k_funcinfo << " Returning transfer" << endl;
	// tell them we have parsed offset bytes
	
	bytes = pos;
	return t;
}

#include "ymsgprotocol.moc"
