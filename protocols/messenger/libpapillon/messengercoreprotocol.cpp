/*
   messengercoreprotocol.cpp - Messenger core protocol for Papillon 

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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
#include "Papillon/NetworkMessage"

namespace Papillon
{

class MessengerCoreProtocol::Private
{
public:
	Private()
		: inNetworkMessage(0), state(MessengerCoreProtocol::NoData)
	{}
	
	~Private()
	{
		delete inNetworkMessage;
	}
	
	// buffer containing unprocessed bytes we received
	QByteArray in;
	// the transfer that is being received
	NetworkMessage *inNetworkMessage;
	// represents the protocol's overall state
	int state;
	// Represend the length of payload data to read.
	int payloadLength;
};

MessengerCoreProtocol::MessengerCoreProtocol() : QObject(), d(new Private)
{}

MessengerCoreProtocol::~MessengerCoreProtocol()
{
	qDebug() << Q_FUNC_INFO;
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
	
	// convert every event in the chunk to a NetworkMessage, signalling it back to the ClientStream
	int parsedBytes = 0;
	int transferCount = 0;
	// while there is data left in the input buffer, and we are able to parse something out of it
	while ( d->in.size() && ( parsedBytes = rawToNetworkMessage( d->in ) ) )
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

NetworkMessage *MessengerCoreProtocol::incomingNetworkMessage()
{
	if ( d->state == Available )
	{
		d->state = NoData;
		return d->inNetworkMessage;
	}
	else
	{
		return 0;
	}
}

void MessengerCoreProtocol::outgoingNetworkMessage(NetworkMessage *outgoing)
{
	emit outgoingData( outgoing->toRawCommand() );
	// Clear the transfer.
	delete outgoing;
}

int MessengerCoreProtocol::rawToNetworkMessage(const QByteArray &raw)
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
			
			NetworkMessage::NetworkMessageType transferType;
			bool dummy, isNumber = false;
			int trId = 0, payloadLength = 0;
			
			command = commandList.at(0);
			
			// Determine the transfer type.
			if(isPayloadCommand(command))
			{
				transferType |= NetworkMessage::PayloadMessage;
				// Remove the last parameter from the command list and set the payload length.
				// So it will not be in the arguments.
				payloadLength = commandList.takeLast().toUInt(&dummy);
				qDebug() << Q_FUNC_INFO << "Begin Payload transfer, length:" << payloadLength;
			}
			
			// Check for a transaction ID.
			// Do not check for a transaction if the commandList size is lower than 2.
			if( commandList.size() >= 2 )
			{
				trId = commandList[1].toUInt(&isNumber);
				if(isNumber)
					transferType |= NetworkMessage::TransactionMessage;
			}
			
			// Begin at the third command arguments if we have a transaction ID.
			int beginAt = isNumber ? 2 : 1;
			// Fill the arguments.
			for(int i = beginAt; i < commandList.size(); ++i)
			{
				arguments << commandList[i];
			}
			
			NetworkMessage *receivedNetworkMessage = new NetworkMessage(transferType);
			receivedNetworkMessage->setCommand(command);
			receivedNetworkMessage->setArguments(arguments);
			
			if(isNumber)
				receivedNetworkMessage->setTransactionId( QString::number(trId) );
			
			d->inNetworkMessage = receivedNetworkMessage;
			
			if(payloadLength > 0)
			{
				d->payloadLength = payloadLength;
				d->state = WaitForPayload;
			}
			else
			{
				// Show parsed line here for non-payload messages.
				qDebug() << Q_FUNC_INFO << parsedLine;
				d->state = Available;
				emit incomingData();
			}
			
			bytesParsed = parsedLine.size() + 2; // 2 is to add \r\n to the size which was trimmed.
		}
		else if(d->state == WaitForPayload || d->state == NeedMore)
		{
			if(raw.size() < d->payloadLength)
			{
				qDebug() << Q_FUNC_INFO << "Raw size:" << raw.size() << "Payload length:" << d->payloadLength;
				d->state = NeedMore;
				return bytesParsed;
			}
			
			// Retrieve the full payload data from raw data (do a shared copy)
			QByteArray payloadData = raw.left(d->payloadLength);
			
			d->inNetworkMessage->setPayloadData(payloadData);
			qDebug() << Q_FUNC_INFO << "Byte data length:" << payloadData.size();
// 			qDebug() << Q_FUNC_INFO << "Payload data read(from CoreProtocol):" << raw;
			// Show full payload command to output
			qDebug() << Q_FUNC_INFO << d->inNetworkMessage->toRawCommand();
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
