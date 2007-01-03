/*
   messengercoreprotocol.cpp - Messenger core protocol for Papillon 

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Http/CoreProtocol"

// Qt includes
#include <QtDebug>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>
#include <QtCore/QLatin1String>
#include <QtCore/QStringList>
#include <QtNetwork/QHttpResponseHeader>

// Papillon includes
#include "Papillon/Http/Transfer"

namespace Papillon
{

class HttpCoreProtocol::Private
{
public:
	Private()
		: inTransfer(0), state(HttpCoreProtocol::NoData)
	{}
	
	// buffer containing unprocessed bytes we received
	QByteArray in;
	// the transfer that is being received
	HttpTransfer *inTransfer;
	// represents the protocol's overall state
	HttpCoreProtocol::State state;
	// Represend the length of payload data to read.
	int contentLength;
};

HttpCoreProtocol::HttpCoreProtocol() : QObject(), d(new Private)
{}

HttpCoreProtocol::~HttpCoreProtocol()
{
	delete d;
}

int HttpCoreProtocol::state()
{
	return (int)d->state;
}

void HttpCoreProtocol::addIncomingData(const QByteArray &incomingBytes )
{
	// Append incoming bytes to incoming buffer
	d->in += incomingBytes;

	// convert every event in the chunk to a Transfer, signalling it back to the clientstream
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	while ( d->in.size() && ( parsedBytes = rawToTransfer( d->in ) ) )
	{
		transferCount++;
		int size =  d->in.size();
		if ( parsedBytes < size )
		{
			// Remove the parsed bytes and keep the old ones.
			d->in = d->in.right( size - parsedBytes );
		}
		else
		{
			d->in.clear();
		}
	}

}

HttpTransfer *HttpCoreProtocol::incomingTransfer()
{
	if ( d->state == Available )
	{
		d->state = NoData;
		return d->inTransfer;
	}
	else
	{
		return 0;
	}
}

void HttpCoreProtocol::outgoingTransfer(HttpTransfer *outgoing)
{
	emit outgoingData( outgoing->toRawCommand() );
	// Clear the transfer.
	delete outgoing;
}

int HttpCoreProtocol::rawToTransfer(const QByteArray &raw)
{
	uint bytesParsed = 0;

	if ( raw.size() < 4 )
	{
		d->state = NeedMore;
		return bytesParsed;
	}
	
	QByteArray tempRaw = raw;
	QDataStream din( &tempRaw, QIODevice::ReadOnly );
	
	// look at first four bytes and decide what to do with the chunk
	if ( okToProceed( din ) )
	{
		if(d->state == NoData)
		{
			int endHeaderPos = tempRaw.lastIndexOf("\r\n\r\n");
			if(endHeaderPos != -1)
			{
				// lastIndexOf return the index before the 2 \r\n\r\n, so we add their size manually.
				endHeaderPos += 4;
			}
			else
			{
				qDebug() << PAPILLON_FUNCINFO << "End of HTTP header wasn't found.";
				d->state = NeedMore;
				return bytesParsed;
			}
			
			// Strip to get only the raw header.
			QByteArray rawHttpHeader = tempRaw.left(endHeaderPos);
			QString httpHeader(rawHttpHeader);
// 			qDebug() << PAPILLON_FUNCINFO << "HTTP header: " << httpHeader;
			
			// Parse HTTP header
			QHttpResponseHeader responseHeader(httpHeader);
			d->inTransfer = new HttpTransfer(HttpTransfer::HttpResponse);
			d->inTransfer->setHttpHeader(responseHeader);
			
			// The transfer has a body, get the body.
			if( responseHeader.hasContentLength() )
			{
				d->state = WaitForContent;
				d->contentLength = responseHeader.contentLength();
				
				qDebug() << PAPILLON_FUNCINFO << "Begin content transfer. Length:" << d->contentLength << "State:" << (int)d->state;
			}
			// This transfer has no body, so we are available.
			else
			{
				d->state = Available;
				qDebug() << PAPILLON_FUNCINFO << d->inTransfer->toRawCommand();
				emit incomingData();
			}
			
			bytesParsed = rawHttpHeader.size();
		}
		else if(d->state == WaitForContent || d->state == NeedMore)
		{
			if(raw.size() < d->contentLength)
			{
				qDebug() << PAPILLON_FUNCINFO << "Raw size:" << raw.size() << "Content length:" << d->contentLength;
				d->state = NeedMore;
				return bytesParsed;
			}

			// Retrieve the full content data from raw data (do a shared copy)
			QByteArray bodyData = tempRaw.left(d->contentLength);
			
			d->inTransfer->setBody(bodyData);
			qDebug() << PAPILLON_FUNCINFO << "Byte data length:" << bodyData.size();
// 			qDebug() << PAPILLON_FUNCINFO << "Payload data read(from Transfer):" << d->inTransfer->payloadLength();
			// Show full payload command to output
			qDebug() << PAPILLON_FUNCINFO << d->inTransfer->toRawCommand();
			d->state = Available;
			emit incomingData();
			
			bytesParsed = bodyData.length();
		}
	}
	return bytesParsed;
}

void HttpCoreProtocol::reset()
{
	d->in.clear();
}

bool HttpCoreProtocol::okToProceed(const QDataStream &din)
{
	if( din.atEnd() )
	{
		d->state = NeedMore;
		return false;
	}
	else
		return true;
}

}

#include "httpcoreprotocol.moc"
//kate: indent-mode csands; tab-width 4;
