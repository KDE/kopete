/*
    Kopete Yahoo Protocol

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
		default:
			kdDebug(14180) << k_funcinfo << "  Parsed packet service -  This means an unknown service " << servicenum << endl;
			return 0L;
		break;
	}
	
	statusnum = yahoo_get32(packet.data() + pos);
	pos += 4;
	
	switch (statusnum)
	{
		// TODO add remaining status
		case (Yahoo::StatusAvailable) :
			status = Yahoo::StatusAvailable;
		case (Yahoo::StatusBRB) :
			status = Yahoo::StatusBRB;
		break;
		default:
			kdDebug(14180) << k_funcinfo << " - unknown status " << statusnum << endl;
			return 0L;
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

	kdDebug(14180) << k_funcinfo << " Returning transfer" << endl;
	// tell them we have parsed offset bytes
	
	bytes = pos;
	return t;
}

#include "ymsgprotocol.moc"
