/*
    Kopete Yahoo Protocol
    Handles logging into to the Yahoo service

    Copyright (c) 2004 Duncan Mac-Vicar P. <duncan@kde.org>

    Copyright (c) 2005 Andre Duffeck <andre.duffeck@kdemail.net>

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

#include <string>

#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "kdebug.h"
#include <qdatastream.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>


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
	QMap<QString, QStringList> data;
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

YMSGTransfer::YMSGTransfer(Yahoo::Service service, Yahoo::Status status)
{
	d = new YMSGTransferPrivate;
	d->valid = true;
	d->service = service;
	d->id = 0;
	d->status = status;
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

void YMSGTransfer::setStatus(Yahoo::Status status)
{
	d->status = status;
}

unsigned int YMSGTransfer::id()
{
	return d->id;
}

void YMSGTransfer::setId(unsigned int id)
{
	d->id = id;
}

QString YMSGTransfer::firstParam(const QString &index)
{
	return d->data[index].front();
}

QStringList YMSGTransfer::paramList(const QString &index)
{
	return d->data[index];
}

void YMSGTransfer::setParam(const QString &index, const QString &data)
{
	d->data[index].append( data );
}

void YMSGTransfer::setParam( const QString &index, int data )
{
	d->data[index].append( QString::number( data ) );
}

int YMSGTransfer::length()
{
	int len = 0;
	QStringList::ConstIterator listIt = 0;
	for (QMap<QString, QStringList>::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		len += it.key().length();
		len += 2;
		for( listIt = (*it).begin(); listIt != (*it).end(); ++listIt )
			len += (*listIt).length();
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
	
	int pos = 0;
	int packetSize = 20 + length();
	QByteArray buffer(packetSize);
	QStringList::ConstIterator listIt = 0;
	
	memcpy(buffer.data() + pos, "YMSG", 4);
	pos += 4;
	
	yahoo_put16(buffer.data() + pos, 0x000c);
	pos += 2;
	
	yahoo_put16(buffer.data() + pos, 0x0000);
	pos += 2;
	
	int len = length();
	kdDebug(14180) << k_funcinfo << " length is " << len << endl;
	
	
	yahoo_put16(buffer.data() + pos, len);
	pos += 2;
	yahoo_put16(buffer.data() + pos, d->service);
	pos += 2;
	yahoo_put32(buffer.data() + pos, d->status);
	pos += 4;
	yahoo_put32(buffer.data() + pos, d->id);
	pos += 4;
	
	for (QMap<QString, QStringList>::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		for( listIt = (*it).begin(); listIt != (*it).end(); ++listIt )
		{
			kdDebug(14180) << k_funcinfo << " Serializing key " << it.key() << " value " << (*it) << endl;
			memcpy( buffer.data() + pos, it.key().latin1(), it.key().length());
			pos += it.key().length();
			buffer[pos++] = 0xc0;
			buffer[pos++] = 0x80;
			memcpy( buffer.data() + pos, (*listIt).latin1(), (*listIt).length());
			pos += (*listIt).length();
			buffer[pos++] = 0xc0;
			buffer[pos++] = 0x80;
		}
	}
	kdDebug(14180) << k_funcinfo << " pos=" << pos << " (packet size)" << endl;
	return buffer;
}

