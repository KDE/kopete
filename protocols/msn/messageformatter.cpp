/*
    messageformatter.cpp - msn p2p protocol

    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "messageformatter.h"
#include "p2p.h"

// Qt includes
#include <qdatastream.h>
#include <qregexp.h>

// Kde includes
#include <kdebug.h>

using P2P::MessageFormatter;
using P2P::Message;

MessageFormatter::MessageFormatter(QObject *parent, const char *name) : QObject(parent, name)
{}

MessageFormatter::~MessageFormatter()
{}

Message MessageFormatter::readMessage(const QByteArray& stream, bool compact)
{
	Message inbound;
	
	Q_UINT32 index = 0;
	if(compact == false)
	{
		// Determine the end position of the message header.
		while(index < stream.size())
		{
			if(stream[index++] == '\n'){
				if(stream[index - 3] == '\n')
					break;
			}
		}

		// Retrieve the message header.
		QString messageHeader = QCString(stream.data(), index);

		// Retrieve the message mime version, content type,
		// and p2p destination.
		QRegExp regex("Content-Type: ([A-Za-z0-9$!*/\\-]*)");
		regex.search(messageHeader);
		QString contentType = regex.cap(1);

		if(contentType != "application/x-msnmsgrp2p")
			return inbound;

//		kdDebug(14140) << k_funcinfo << endl;
	
		regex = QRegExp("MIME-Version: (\\d.\\d)");
		regex.search(messageHeader);
		inbound.mimeVersion = regex.cap(1);
		inbound.contentType = contentType;
		regex = QRegExp("P2P-Dest: ([^\r\n]*)");
		regex.search(messageHeader);
		QString destination = regex.cap(1);
	}
	
	QDataStream reader(stream, IO_ReadOnly);
	reader.setByteOrder(QDataStream::LittleEndian);
	// Seek to the start position of the message
	// transport header.
	reader.device()->at(index);

	// Read the message transport headers from the stream.
	reader >> inbound.header.sessionId;
	reader >> inbound.header.identifier;
	reader >> inbound.header.dataOffset;
	reader >> inbound.header.totalDataSize;
	reader >> inbound.header.dataSize;
	reader >> inbound.header.flag;
	reader >> inbound.header.ackSessionIdentifier;
	reader >> inbound.header.ackUniqueIdentifier;
	reader >> inbound.header.ackDataSize;

	/*kdDebug(14140)
		<< "session id, "             << inbound.header.sessionId << endl
		<< "identifier, "             << inbound.header.identifier << endl
		<< "data offset, "            << inbound.header.dataOffset << endl
		<< "total size, "             << inbound.header.totalDataSize << endl
		<< "data size, "              << inbound.header.dataSize << endl
		<< "flag, "                   << inbound.header.flag << endl
		<< "ack session identifier, " << inbound.header.ackSessionIdentifier << endl
		<< "ack unique identifier, "  << inbound.header.ackUniqueIdentifier << endl
		<< "ack data size, "          << inbound.header.ackDataSize
	<< endl;*/

	// Read the message body from the stream.
	if(inbound.header.dataSize > 0){
		inbound.body.resize(inbound.header.dataSize);
		reader.readRawBytes(inbound.body.data(), inbound.header.dataSize);
	}

	if(compact == false)
	{
		reader.setByteOrder(QDataStream::BigEndian);
		// Read the message application identifier from the stream.
		reader >> inbound.applicationIdentifier;

/*		kdDebug(14140)
			<< "application identifier, " << inbound.applicationIdentifier
		<< endl;*/
	}
		
	return inbound;
}

void MessageFormatter::writeMessage(const Message& message, QByteArray& stream, bool compact)
{
//	kdDebug(14140) << k_funcinfo << endl;

	QDataStream writer(stream, IO_WriteOnly);
	writer.setByteOrder(QDataStream::LittleEndian);

	if(compact == false)
	{
		const QCString messageHeader = QString("MIME-Version: 1.0\r\n"
			"Content-Type: application/x-msnmsgrp2p\r\n"
			"P2P-Dest: " + message.destination + "\r\n"
			"\r\n").utf8();
		// Set the capacity of the message buffer.
		stream.resize(messageHeader.length() + 48 + message.body.size() + 4);
		// Write the message header to the stream
		writer.writeRawBytes(messageHeader.data(), messageHeader.length());
	}
	else
	{
		// Set the capacity of the message buffer.
		stream.resize(4 + 48 + message.body.size());
		// Write the message size to the stream.
		writer << (Q_INT32)(48+message.body.size());
	}

	
	// Write the transport headers to the stream.
	writer << message.header.sessionId;
	writer << message.header.identifier;
	writer << message.header.dataOffset;
	writer << message.header.totalDataSize;
	writer << message.header.dataSize;
	writer << message.header.flag;
	writer << message.header.ackSessionIdentifier;
	writer << message.header.ackUniqueIdentifier;
	writer << message.header.ackDataSize;

/*	kdDebug(14140)
		<< "session id, "             << message.header.sessionId << endl
		<< "identifier, "             << message.header.identifier << endl
		<< "data offset, "            << message.header.dataOffset << endl
		<< "total size, "             << message.header.totalDataSize << endl
		<< "data size, "              << message.header.dataSize << endl
		<< "flag, "                   << message.header.flag << endl
		<< "ack session identifier, " << message.header.ackSessionIdentifier << endl
		<< "ack unique identifier, "  << message.header.ackUniqueIdentifier << endl
		<< "ack data size, "          << message.header.ackDataSize
		<< endl;
*/
	if(message.body.size() > 0){
		// Write the messge body to the stream.
		writer.writeRawBytes(message.body.data(), message.body.size());
	}

	if(compact == false)
	{
		// Seek to the message application identifier section.
		writer.setByteOrder(QDataStream::BigEndian);
		// Write the message application identifier to the stream.
		writer << message.applicationIdentifier;

/*		kdDebug(14140)
			<< "application identifier, " << message.applicationIdentifier
			<< endl;
		*/
	}
}

#include "messageformatter.moc"
