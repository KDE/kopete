/*
    Kopete Oscar Protocol
    eventprotocol.cpp - reads the protocol used by Oscar for BOS connections

    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>

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

#include <qcstring.h>
#include <qdatastream.h>
#include <qmap.h>
#include <qobject.h>

#include <kdebug.h>

#include "ymsgprotocol.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"

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
	kdDebug(14180) << k_funcinfo << " - parsed packet service " << servicenum << endl;
	
	switch (servicenum)
	{
		// TODO add remamining services
		case (Yahoo::ServiceAuth) :
			service = Yahoo::ServiceAuth;
		break;
		case (Yahoo::ServiceVerify) :
			service = Yahoo::ServiceVerify;
		break;
		default:
			kdDebug(14180) << k_funcinfo << " - unknown service " << servicenum << endl;
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
	pos += 4;
	
	// parse data
	int offset = 0;
	int parsedDataPos = 0;
	QString key;
	
	while ( offset < len )
	{
		if ((packet[pos+offset] == 0xc0) && (packet[pos+offset+1] == 0x80))
		{
			kdDebug(14180) << k_funcinfo << " found key [" << key << "]" << endl;
			// jump the second delimiter
			offset++;
			QString value;
			while (!(( (packet[pos+offset] == 0xc0) && (packet[pos+offset+1] == 0x80) )))
			{
				value += packet[pos+offset];
				offset++;
			}
			kdDebug(14180) << k_funcinfo << " value [" << value << "]" << endl;
			params[key] = value;
			// back one so the if can find the next key
			offset--;
		}
		else
		{
			key += packet[pos+offset];
		}
		offset++;
	}
	
	// ok, parsed all data
	pos += offset;
	
	YMSGTransfer *t = new YMSGTransfer();
	t->setService(service);
	t->setId(sessionid);
	t->setStatus(status);
	
	for (QMap<QString, QString>::ConstIterator it = params.begin(); it !=  params.end(); ++it) 
	{
		kdDebug(14180) << k_funcinfo << " setting packet key " << it.key() << endl;
		t->setParam(it.key(), (*it));
	}
	kdDebug(14180) << k_funcinfo << " Returning transfer" << endl;
	// tell them we have parsed offset bytes
	
	bytes = pos;
	return t;
}


#include "ymsgprotocol.moc"
