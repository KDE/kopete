/*
    YMSGtransfer - Kopete Yahoo Protocol

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

#include "ymsgtransfer.h"
#include "kdebug.h"
#include <qmap.h>
#include <qdatastream.h>
#include <qstring.h>

using namespace Yahoo;

class YMSGTransferPrivate
{
public:
	int yflag;
	int version;
	int packetLength;
	Yahoo::Service service;
	Yahoo::Status status;
	unsigned int id;	
	QMap<int, QString> data;
	bool valid;
};

YMSGTransfer::YMSGTransfer()
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->id = 0;
	d-> status = Yahoo::StatusAvailable;
}

YMSGTransfer::YMSGTransfer(Yahoo::Service service)
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->service = service;
	d->id = 0;
	d->status = Yahoo::StatusAvailable;
}

YMSGTransfer::~YMSGTransfer()
{
	delete d;
}

Transfer::TransferType YMSGTransfer::type()
{
	return Transfer::YMSGTransfer;
}

bool YMSGTransfer::isValid()
{
	return d->valid;
}

Yahoo::Service YMSGTransfer::service()
{
	return d->service;
}

void YMSGTransfer::setService(Yahoo::Service service)
{
	d->service = service;
}

Yahoo::Status YMSGTransfer::status()
{
	return d->status;
}

unsigned int YMSGTransfer::id()
{
	return d->id;
}

QString YMSGTransfer::param(int index)
{
	return d->data[index];
}

void YMSGTransfer::setParam(int index, QString data)
{
	d->data[index] = data;
}

int YMSGTransfer::length()
{
	int len = 0;
	for (QMap<int, QString>::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		int tmp = it.key();
		do 
		{
			tmp /= 10;
			len++;
		} while (tmp);
		
		len += 2;
		len += (*it).length();
		len += 2;
	}
	return len;
}


QByteArray YMSGTransfer::serialize()
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
	
	QByteArray packet;
	QDataStream bs(packet, IO_WriteOnly);
	bs << 'Y' << 'M' << 'S' << 'G'; // flag
	//bs << 0x000b << 0x0000; // version
	bs << 0x09 << 0x00 << 0x00 << 0x00;
	
	int len = length();
	kdDebug(14180) << k_funcinfo << " length is " << len << endl;
	bs << (WORD) len ;
	bs << (WORD) d->service;
	bs << (DWORD) d->status;
	bs << (DWORD) d->id;
	
	for (QMap<int, QString>::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		kdDebug(14180) << k_funcinfo << " Serializing key " << it.key() << " value " << (*it) << endl;
		bs << it.key();
		bs << 0xc080;
		bs << (*it);
		bs << 0xc080;
	}
	return packet;
}

