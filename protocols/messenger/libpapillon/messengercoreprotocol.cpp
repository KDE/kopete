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
#include "Papillon/MessengerCoreProtocol"

// Qt includes
#include <QtDebug>
#include <QtCore/QDataStream>
#include <QtCore/QTextStream>
#include <QtCore/QLatin1String>
#include <QtCore/QStringList>

// Papillon includes
#include "Papillon/Transfer"

namespace Papillon
{

class MessengerCoreProtocol::Private
{
public:
	Private()
		: inTransfer(0), state(MessengerCoreProtocol::NoData)
	{}
	
	~Private()
	{
		delete inTransfer;
	}
	
	// buffer containing unprocessed bytes we received
	QByteArray in;
	// the transfer that is being received
	Transfer *inTransfer;
	// represents the protocol's overall state
	int state;
	// Represend the length of payload data to read.
	int payloadLength;
};

MessengerCoreProtocol::MessengerCoreProtocol() : QObject(), d(new Private)
{}

MessengerCoreProtocol::~MessengerCoreProtocol()
{
	qDebug() << PAPILLON_FUNCINFO;
	delete d;
}

int MessengerCoreProtocol::state()
{
	return d->state;
}

void MessengerCoreProtocol::addIncomingData(const QByteArray &incomingBytes )
{
	// Append incoming bytes to incoming buffer
	d->in += incomingBytes;
	
	// convert every event in the chunk to a Transfer, signalling it back to the ClientStream
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

Transfer *MessengerCoreProtocol::incomingTransfer()
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

void MessengerCoreProtocol::outgoingTransfer(Transfer *outgoing)
{
	emit outgoingData( outgoing->toRawCommand() );
	// Clear the transfer.
	delete outgoing;
}

int MessengerCoreProtocol::rawToTransfer(const QByteArray &raw)
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
		if(d->state != WaitForPayload && d->state != NeedMore)
		{
			QTextStream lineStream(tempRaw);
			
			QString parsedLine = lineStream.readLine();
			
			QStringList commandList = parsedLine.split(" ");
			
			QString command;
			QStringList arguments;
			
			Transfer::TransferType transferType;
			bool dummy, isNumber;
			int trId = 0, payloadLength = 0;
			
			command = commandList.at(0);
			
			// Determine the transfer type.
			if(isPayloadCommand(command))
			{
				transferType |= Transfer::PayloadTransfer;
				// Remove the last parameter from the command list and set the payload length.
				// So it will not be in the arguments.
				payloadLength = commandList.takeLast().toUInt(&dummy);
				qDebug() << PAPILLON_FUNCINFO << "Begin Payload transfer, length:" << payloadLength;
			}
			
			// Check for a transaction ID.
			// Do not check for a transaction if the commandList size is lower than 2.
			if( commandList.size() >= 2 )
			{
				trId = commandList[1].toUInt(&isNumber);
				if(isNumber)
					transferType |= Transfer::TransactionTransfer;
			}
			
			// Begin at the third command arguments if we have a transaction ID.
			int beginAt = isNumber ? 2 : 1;
			// Fill the arguments.
			for(int i = beginAt; i < commandList.size(); ++i)
			{
				arguments << commandList[i];
			}
			
			Transfer *receivedTransfer = new Transfer(transferType);
			receivedTransfer->setCommand(command);
			receivedTransfer->setArguments(arguments);
			
			if(isNumber)
				receivedTransfer->setTransactionId( QString::number(trId) );
			
			d->inTransfer = receivedTransfer;
			
			if(payloadLength > 0)
			{
				d->payloadLength = payloadLength;
				d->state = WaitForPayload;
			}
			else
			{
				// Show parsed line here for non-payload messages.
				qDebug() << PAPILLON_FUNCINFO << parsedLine;
				d->state = Available;
				emit incomingData();
			}
			
			bytesParsed = parsedLine.size() + 2; // 2 is to add \r\n to the size which was trimmed.
		}
		else if(d->state == WaitForPayload || d->state == NeedMore)
		{
			if(raw.size() < d->payloadLength)
			{
				qDebug() << PAPILLON_FUNCINFO << "Raw size:" << raw.size() << "Payload length:" << d->payloadLength;
				d->state = NeedMore;
				return bytesParsed;
			}
			
			// Retrieve the full payload data from raw data (do a shared copy)
			QByteArray payloadData = raw.left(d->payloadLength);
			
			d->inTransfer->setPayloadData(payloadData);
			qDebug() << PAPILLON_FUNCINFO << "Byte data length:" << payloadData.size();
// 			qDebug() << PAPILLON_FUNCINFO << "Payload data read(from CoreProtocol):" << raw;
			// Show full payload command to output
			qDebug() << PAPILLON_FUNCINFO << d->inTransfer->toRawCommand();
			d->state = Available;
			
			bytesParsed = payloadData.size();
			
			emit incomingData();
		}
	}
	return bytesParsed;
}

void MessengerCoreProtocol::reset()
{
	d->in.clear();
}

bool MessengerCoreProtocol::okToProceed(const QDataStream &din)
{
	if( din.atEnd() )
	{
		d->state = NeedMore;
		return false;
	}
	else
		return true;
}

bool MessengerCoreProtocol::isPayloadCommand(const QString &command)
{
	if( command == QLatin1String("ADL") ||
	    command == QLatin1String("GCF") ||
	    command == QLatin1String("MSG") ||
	    command == QLatin1String("QRY") ||
	    command == QLatin1String("RML") ||
	    command == QLatin1String("UBX") ||
	    command == QLatin1String("UBN") ||
	    command == QLatin1String("UUN") ||
	    command == QLatin1String("UUX")
	  )
		return true;
	else
		return false;
}

}

#include "messengercoreprotocol.moc"
//kate: indent-mode csands; tab-width 4;
