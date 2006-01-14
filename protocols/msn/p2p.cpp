/*
    p2p.cpp - msn p2p protocol

    Copyright (c) 2003-2005 by Olivier Goffart        <ogoffart@ kde.org>
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

#include "p2p.h"
#include "dispatcher.h"
using P2P::TransferContext;
using P2P::Message;
using P2P::MessageType;
using P2P::TransferType;

#include <stdlib.h>

// Kde includes
#include <kbufferedsocket.h>
#include <kdebug.h>
// Qt includes
#include <qfile.h>

// Kopete includes
#include <kopetetransfermanager.h>

QString P2P::Uid::createUid()
{
	return (QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number(rand()%0xAAFF+0x1111, 16) + "-"
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)
			+ QString::number((unsigned long int)rand()%0xAAFF+0x1111, 16)).upper();
}

TransferContext::TransferContext(const QString &contact, P2P::Dispatcher *dispatcher, Q_UINT32 sessionId)
	: QObject(dispatcher) ,
	   m_sessionId(sessionId) ,
	   m_identifier(0) ,
	   m_file(0) ,
	   m_transactionId (0),
	   m_ackSessionIdentifier (0) ,
	   m_ackUniqueIdentifier ( 0 ),
	   m_transfer ( 0l)  ,

	   m_baseIdentifier(rand()%0x0FFFFFF0 + 4),
	   m_dispatcher (dispatcher),
	   m_isComplete (false) ,
	   m_offset(0),
	   m_totalDataSize(0),
	   m_recipient(contact),
	   m_sender(dispatcher->localContact()),
	   m_socket(0),
	   m_state ( Invitation)
{
	m_type = File ;   //uh,  why??? -Olivier
}

TransferContext::~TransferContext()
{
	m_transfer = 0l;

	if(m_file){
		delete m_file;
		m_file = 0l;
	}
}

void TransferContext::acknowledge(const Message& message)
{
	kdDebug(14140) << k_funcinfo << m_dispatcher<< endl;

	Message outbound;
	outbound.header.sessionId = message.header.sessionId;

	if(m_identifier == 0){
		m_identifier = m_baseIdentifier;
	}
// 	else if(m_state == Finished && m_direction == Incoming){
// 		m_identifier = m_baseIdentifier - 1;
// 	}
	else if(m_state == Finished && m_direction == Outgoing){
		m_identifier = m_baseIdentifier + 1;
	}
	else
		++m_identifier;

	outbound.header.identifier    = m_identifier;
	outbound.header.dataOffset    = 0l;
	outbound.header.totalDataSize = message.header.totalDataSize;
	outbound.header.dataSize      = 0;
// 	if(m_type == UserDisplayIcon && m_state == Finished){
// 		if(m_direction == Outgoing){
// 			outbound.header.flag = 0x40;
// 		}
// 	}
// 	else
		outbound.header.flag = 2;

	outbound.header.ackSessionIdentifier = message.header.identifier;
	outbound.header.ackUniqueIdentifier  = message.header.ackSessionIdentifier;
	outbound.header.ackDataSize   = message.header.totalDataSize;
	// NOTE outbound.body is null by default
	outbound.applicationIdentifier = 0l;
	outbound.destination = m_recipient;

	QByteArray stream;
	// Write the acknowledge message to the stream.
	m_messageFormatter.writeMessage(outbound, stream, (m_socket != 0l));
	if(!m_socket)
	{
		// Send the acknowledge message.
		m_dispatcher->callbackChannel()->send(stream);
	}
	else
	{
		// Send acknowledge message directly.
		m_socket->writeBlock(stream.data(), stream.size());
	}
}

void TransferContext::error()
{
	kdDebug(14140) << k_funcinfo << endl;
	sendMessage(ERROR);
	m_dispatcher->detach(this);
}

void TransferContext::sendData(const QByteArray& bytes)
{
	Message outbound;
	outbound.header.sessionId  = m_sessionId;
	outbound.header.identifier = m_identifier;
	outbound.header.dataOffset = m_offset;
	if(m_file){
		outbound.header.totalDataSize = m_file->size();
	}
	else
		outbound.header.totalDataSize = m_totalDataSize;

	outbound.header.dataSize = bytes.size();
	if(m_type == UserDisplayIcon){
		outbound.header.flag = 0x20;
	}
	else if(m_type == P2P::File){
		outbound.header.flag = 0x01000030;
	}
	else  outbound.header.flag = 0;

	outbound.header.ackSessionIdentifier = rand()%0x8FFFFFF0 + 4;
	outbound.header.ackUniqueIdentifier  = 0;
	outbound.header.ackDataSize = 0l;
	outbound.body = bytes;
	outbound.applicationIdentifier = (uint)m_type;

	outbound.destination = m_recipient;

	QByteArray stream;
	m_messageFormatter.writeMessage(outbound, stream, (m_socket != 0l));
	if(!m_socket)
	{
		// Send the data message.
 		m_transactionId = m_dispatcher->callbackChannel()->send(stream);
 	}
 	else
 	{
 		// Send data directly.
 		m_socket->writeBlock(stream.data(), stream.size());
 	}
}

void TransferContext::sendDataPreparation()
{
	kdDebug(14140) << k_funcinfo << endl;

	Message outbound;
	outbound.header.sessionId  = m_sessionId;
	outbound.header.identifier = ++m_identifier;
	outbound.header.dataOffset = 0;
	outbound.header.totalDataSize = 4;
	outbound.header.dataSize = 4;
	outbound.header.flag = 0;
	outbound.header.ackSessionIdentifier = rand()%0x8FFFFFF0 + 4;
	outbound.header.ackUniqueIdentifier  = 0;
	outbound.header.ackDataSize   = 0l;
	QByteArray bytes(4);
	bytes.fill('\0');
	outbound.body = bytes;
	outbound.applicationIdentifier = 1;
	outbound.destination = m_recipient;

	QByteArray stream;
	m_messageFormatter.writeMessage(outbound, stream);
	// Send the receiving client the data prepartion message.
 	m_dispatcher->callbackChannel()->send(stream);
}

void TransferContext::sendMessage(MessageType type, const QString& content, Q_INT32 flag, Q_INT32 appId)
{
	Message outbound;
	if(appId != 0){
		outbound.header.sessionId = m_sessionId;
	}
	else
		outbound.header.sessionId = 0;

	if(m_identifier == 0){
		m_identifier = m_baseIdentifier;
	}
	else if(m_state == Invitation && m_direction == P2P::Outgoing && m_type == UserDisplayIcon)
	{
			m_identifier -= 3;
	}
	else if(m_state == Invitation && m_direction == P2P::Incoming && m_type == File)
	{
			m_identifier -= 3;
	}
	else
		++m_identifier;

	outbound.header.identifier = m_identifier;
	outbound.header.flag = flag;
	outbound.header.ackSessionIdentifier = m_ackSessionIdentifier;
	outbound.header.ackUniqueIdentifier  = m_ackUniqueIdentifier;
	outbound.header.ackDataSize = 0l;
	outbound.applicationIdentifier = appId;
	outbound.destination = m_recipient;

	QString contentType, cSeq, method;

	switch(m_state)
	{
		case DataTransfer:
			contentType = "application/x-msnmsgr-transreqbody";
			if(m_type == File && m_direction == Incoming)
			{
				contentType = "application/x-msnmsgr-transrespbody";
			}
			break;
		case Finished:
			contentType = "application/x-msnmsgr-sessionclosebody";
			break;
		default:
			contentType = "application/x-msnmsgr-sessionreqbody";
			if(m_type == File && m_direction == Outgoing)
			{
				if(m_state == Negotiation){
					contentType = "application/x-msnmsgr-transreqbody";
				}
			}
			if(m_type == P2P::WebcamType && type==P2P::INVITE && m_state == Negotiation)
			{
				contentType = "application/x-msnmsgr-transreqbody";
			}
			break;
	}

	switch(type)
	{
		case BYE:
			method = "BYE MSNMSGR:" + m_recipient + " MSNSLP/1.0";
			cSeq   = "0";
			break;

		case DECLINE:
			method = "MSNSLP/1.0 603 DECLINE";
			cSeq   = "1";
			break;

		case ERROR:
			contentType = "null";
			method = "MSNSLP/1.0 500 Internal Error";
			cSeq   = "1";
			break;

		case INVITE:
			method = "INVITE MSNMSGR:" + m_recipient + " MSNSLP/1.0";
			cSeq   = "0";
			break;

		case OK:
			method = "MSNSLP/1.0 200 OK";
			cSeq   = "1";
			break;
	}

	QCString body = QString(method + "\r\n"
		"To: <msnmsgr:" + m_recipient + ">\r\n"
		"From: <msnmsgr:" + m_sender  + ">\r\n"
		"Via: MSNSLP/1.0/TLP ;branch={" + m_branch.upper() + "}\r\n"
		"CSeq: "+ cSeq +"\r\n"
		"Call-ID: {" + m_callId.upper() + "}\r\n"
		"Max-Forwards: 0\r\n"
		"Content-Type: " + contentType + "\r\n"
		"Content-Length: "+ QString::number(content.length() + 1) + "\r\n"
		"\r\n" +
		content).utf8();

	// NOTE The body must have a null character at the end.
	// QCString by chance automatically adds a \0 to the
	// end of the string.

	outbound.header.totalDataSize = body.size();
	// Send the outbound message.
	sendMessage(outbound, body);
}

void TransferContext::sendMessage(Message& outbound, const QByteArray& body)
{
	Q_INT64 offset = 0L, bytesLeft = outbound.header.totalDataSize;
	Q_INT16 chunkLength = 1202;

	// Split the outbound message if necessary.
	while(bytesLeft > 0L)
	{
		if(bytesLeft < chunkLength)
		{
			// Copy the last chunk of the multipart message.
			outbound.body.duplicate(body.data() + offset, bytesLeft);
			outbound.header.dataSize = bytesLeft;
			outbound.header.dataOffset = offset;
			bytesLeft = 0L;
		}
		else
		{
			// Copy the next chunk of the multipart message in the sequence.
			outbound.body.duplicate(body.data() + offset, chunkLength);
			outbound.header.dataSize = chunkLength;
			outbound.header.dataOffset = offset;
			offset += chunkLength;
			bytesLeft -= offset;
		}

		kdDebug(14140) << k_funcinfo <<
			QCString(outbound.body.data(), outbound.body.size())
			<< endl;

		QByteArray stream;
		// Write the outbound message to the stream.
		m_messageFormatter.writeMessage(outbound, stream, (m_socket != 0l));
		if(!m_socket)
		{
			// Send the outbound message.
			m_dispatcher->callbackChannel()->send(stream);
		}
		else
		{
			// Send outbound message directly.
			m_socket->writeBlock(stream.data(), stream.size());
		}
	}
}

void TransferContext::setType(TransferType type)
{
	m_type = type;
}

void TransferContext::abort()
{
	kdDebug(14140) << k_funcinfo << endl;
	if(m_transfer)
	{
		if(m_transfer->error() == KIO::ERR_ABORTED)
		{
			switch(m_direction)
			{
				case P2P::Outgoing:
					if(m_type == File)
					{
						// Do nothing.
					}
					break;

				case P2P::Incoming:
					if(m_type == File)
					{
						// Do nothing.
					}
					break;
			}
		}
		else
		{
			m_state = Finished;
			sendMessage(BYE, "\r\n");
		}
	}
}

void TransferContext::readyWrite()
{
	if(m_direction == Outgoing && m_state == DataTransfer){
		readyToSend();
	}
}

void TransferContext::readyToSend()
{}

#include "p2p.moc"
