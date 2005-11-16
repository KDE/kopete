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
	ParamList data;
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

ParamList YMSGTransfer::paramList()
{
	return d->data;
}

int YMSGTransfer::paramCount( int index )
{
	int cnt = 0;
	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		if( (*it).first == index )
			cnt++;
	}
	return cnt;
}


const QString &YMSGTransfer::nthParam( int index, int occurence )
{
	int cnt = 0;
	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		if( (*it).first == index && cnt++ == occurence)
			return (*it).second;
	}
	return QString::null;
}

const QString &YMSGTransfer::nthParamSeparated( int index, int occurence, int separator )
{

	int cnt = -1;
	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		if( (*it).first == separator )
			cnt++;
		if( (*it).first == index && cnt == occurence)
			return (*it).second;
	}
	return QString::null;
}

const QString &YMSGTransfer::firstParam( int index )
{
	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		if( (*it).first == index )
			return (*it).second;
	}
	return QString::null;
}

void YMSGTransfer::setParam(int index, const QString &data)
{
	d->data.append( Param( index, data ) );
}

void YMSGTransfer::setParam( int index, int data )
{
	d->data.append( Param( index, QString::number( data ) ) );
}

int YMSGTransfer::length()
{
	int len = 0;
	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
		len += QString::number( (*it).first ).length();
		len += 2;
		len += (*it).second.length();
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
	QStringList::ConstIterator listIt = 0;
	QByteArray buffer;
	QDataStream stream( buffer, IO_WriteOnly );
	
	stream << (Q_INT8)'Y' << (Q_INT8)'M' << (Q_INT8)'S' << (Q_INT8)'G';
// 	memcpy(buffer.data(), "YMSG", 4);
// 	pos += 4;
// 	
	if( d->service == Yahoo::ServicePictureUpload )
		stream << (Q_INT16)0x0d00;
	else
		stream << (Q_INT16)0x000d;
// 	yahoo_put16(buffer.data() + pos, 0x000d);
// 	pos += 2;
// 	
	stream << (Q_INT16)0x0000;
// 	yahoo_put16(buffer.data() + pos, 0x0000);
// 	pos += 2;
// 	
// 	int len = length();
// 	kdDebug(14180) << k_funcinfo << " length is " << len << endl;
// 	
// 	
	if( d->service == Yahoo::ServicePictureUpload )
		stream << (Q_INT16)(length()+4);
	else
		stream << (Q_INT16)length();
// 	yahoo_put16(buffer.data() + pos, len);
// 	pos += 2;
	stream << (Q_INT16)d->service;
// 	yahoo_put16(buffer.data() + pos, d->service);
// 	pos += 2;
	stream << (Q_INT32)d->status;
// 	yahoo_put32(buffer.data() + pos, d->status);
// 	pos += 4;
	stream << (Q_INT32)d->id;
// 	yahoo_put32(buffer.data() + pos, d->id);
// 	pos += 4;
// 	
 	for (ParamList::ConstIterator it = d->data.begin(); it !=  d->data.end(); ++it) 
	{
 		kdDebug(14180) << k_funcinfo << " Serializing key " << (*it).first << " value " << (*it).second << endl;
		stream.writeRawBytes ( QString::number( (*it).first ).local8Bit(), QString::number( (*it).first ).length() );
		stream << (Q_INT8)0xc0 << (Q_INT8)0x80;
// 		memcpy( buffer.data() + pos, QString::number( (*it).first ).latin1(), QString::number( (*it).first ).length());
// 		pos += QString::number( (*it).first ).length();
// 		buffer[pos++] = 0xc0;
// 		buffer[pos++] = 0x80;
		stream.writeRawBytes( (*it).second.local8Bit(), (*it).second.length() );
		stream << (Q_INT8)0xc0 << (Q_INT8)0x80;
// 		memcpy( buffer.data() + pos, (*it).second.latin1(), (*it).second.length());
// 		pos += (*it).second.length();
// 		buffer[pos++] = 0xc0;
// 		buffer[pos++] = 0x80;
	}
	kdDebug(14180) << k_funcinfo << " pos=" << pos << " (packet size)" << buffer << endl;
	return buffer;
}

