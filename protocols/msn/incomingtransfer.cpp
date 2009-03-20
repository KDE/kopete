/*
    incomingtransfer.cpp - msn p2p protocol

    Copyright (c) 2003-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "incomingtransfer.h"
//Added by qt3to4:
#include <QByteArray>
using P2P::TransferContext;
using P2P::IncomingTransfer;
using P2P::Message;

// Kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

// Qt includes
#include <qfile.h>
#include <qregexp.h>
#include <qtcpserver.h>
#include <qtcpsocket.h>

// Kopete includes
#include <kopetetransfermanager.h>

IncomingTransfer::IncomingTransfer(const QString& from, P2P::Dispatcher *dispatcher, quint32 sessionId)
: TransferContext(from,dispatcher,sessionId)
{
	m_direction = P2P::Incoming;
	m_listener  = 0l;
}

IncomingTransfer::~IncomingTransfer()
{
	kDebug(14140) ;
	if(m_listener)
	{
		delete m_listener;
		m_listener = 0l;
	}

	if(m_socket)
	{
		delete m_socket;
		m_socket = 0l;
	}
}


void IncomingTransfer::slotTransferAccepted(Kopete::Transfer* transfer, const QString& /*fileName*/)
{
	quint32 sessionId = transfer->info().internalId().toUInt();
	if(sessionId!=m_sessionId)
		return;
	
	QObject::connect(transfer , SIGNAL(transferCanceled()), this, SLOT(abort()));
	m_transfer = transfer;
		
	QString content = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	sendMessage(OK, content);
	
	QObject::disconnect(Kopete::TransferManager::transferManager(), 0l, this, 0l);
}

void IncomingTransfer::slotTransferRefused(const Kopete::FileTransferInfo& info)
{
	quint32 sessionId = info.internalId().toUInt();
	if(sessionId!=m_sessionId)
		return;
	
	QString content = QString("SessionID: %1\r\n\r\n").arg(sessionId);
	// Send the sending client a cancellation message.
	sendMessage(DECLINE, content);
	m_state=Finished;
	
	QObject::disconnect(Kopete::TransferManager::transferManager(), 0l, this, 0l);
}



void IncomingTransfer::acknowledged()
{
	kDebug(14140) ;
	
	switch(m_state)
	{
		case Invitation:
				// NOTE UDI: base identifier acknowledge message, ignore.
				//      UDI: 200 OK message should follow.
				if(m_type == File)
				{
					// FT: 200 OK acknowledged message.
					// If this is the first connection between the two clients, a direct connection invitation
					// should follow. Otherwise, the file transfer may start right away.
					if(m_transfer)
					{
						QFile *destination = new QFile(m_transfer->destinationURL().toLocalFile());
						if(!destination->open(QIODevice::WriteOnly))
						{
							m_transfer->slotError(KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("Cannot open file for writing"));
							m_transfer = 0l;
							
							error();
							return;
						}
						m_file = destination;
					}
					m_state = Negotiation;
				}
			break;

		case Negotiation:
				// 200 OK acknowledge message.
			break;

		case DataTransfer:
			break;
			
		case Finished:
			// UDI: Bye acknowledge message.
			m_dispatcher->detach(this);
			break;
	}
}

void IncomingTransfer::processMessage(const Message& message)
{
	if(m_file && (message.header.flag == 0x20 || message.header.flag == 0x01000030))
	{
		if(m_state == Finished) {
			return;
		}
		// UserDisplayIcon data or File data is in this message.
		// Write the received data to the file.
		kDebug(14140) << QString("Received, %1 bytes").arg(message.header.dataSize);
		
		m_file->write(message.body.data(), message.header.dataSize);
		if(m_transfer){
			m_transfer->slotProcessed(message.header.dataOffset + message.header.dataSize);
		}
		
		if((message.header.dataOffset + message.header.dataSize) == message.header.totalDataSize)
		{
			// Transfer is complete.
			if(m_type == UserDisplayIcon){
				m_tempFile->close();
				m_dispatcher->displayIconReceived(m_tempFile, m_object);
				m_tempFile = 0l;
				m_file = 0l;
			}
			else
			{
				m_file->close();
			}

			m_isComplete = true;
			// Send data acknowledge message.
			acknowledge(message);

			if(m_type == UserDisplayIcon)
			{
				m_state = Finished;
				// Send BYE message.
				sendMessage(BYE, "\r\n");
			}
		}
	}
	//pidgin is probably broken since it sends 0 as appid but we don't need this check anyway
	else if(message.header.dataSize == 4 /*&& message.applicationIdentifier == 1*/)
	{
		// Data preparation message.
		m_tempFile = new KTemporaryFile();
		m_tempFile->setPrefix("msnpicture--");
// 		m_tempFile->setSuffix(".png");
		m_tempFile->open();
		m_file = m_tempFile;
		m_state = DataTransfer;
		// Send data preparation acknowledge message.
		acknowledge(message);
	}
	else
	{
		QString body =
			QByteArray(message.body.data(), message.header.dataSize);
//		kDebug(14140) << "received, " << body;

		if(body.startsWith("INVITE"))
		{
			// Retrieve some MSNSLP headers used when
			// replying to this INVITE message.
			QRegExp regex(";branch=\\{([0-9A-F\\-]*)\\}\r\n");
			regex.indexIn(body);
			m_branch = regex.cap(1);
			// NOTE Call-ID never changes.
			regex = QRegExp("Call-ID: \\{([0-9A-F\\-]*)\\}\r\n");
			regex.indexIn(body);
			m_callId = regex.cap(1);
			regex = QRegExp("Bridges: ([^\r\n]*)\r\n");
			regex.indexIn(body);
			QString bridges = regex.cap(1);
			// The NetID field is 0 if the Conn-Type is
			// Direct-Connect or Firewall, otherwise, it is
			// a randomly generated number.
			regex = QRegExp("NetID: (\\-?\\d+)\r\n");
			regex.indexIn(body);
			QString netId = regex.cap(1);
			kDebug(14140) << "net id, " << netId;
			// Connection Types
			// - Direct-Connect
			// - Port-Restrict-NAT
			// - IP-Restrict-NAT
			// - Symmetric-NAT
			// - Firewall
			regex = QRegExp("Conn-Type: ([^\r\n]+)\r\n");
			regex.indexIn(body);
			QString connType = regex.cap(1);

			bool wouldListen = false;
			if(netId.toUInt() == 0 && connType == "Direct-Connect"){
				wouldListen = true;

			}
			else if(connType == "IP-Restrict-NAT"){
				wouldListen = true;
			}
#if 1
			wouldListen = false; // TODO Direct connection support
#endif			
			QString content;
			
			if(wouldListen)
			{
				// Create a listening socket for direct file transfer.
				m_listener = new QTcpServer();
				// Create the callback that will try to accept incoming connections.
				QObject::connect(m_listener, SIGNAL(newConnection()), SLOT(slotAccept()));

				// Listen for incoming connections.
				bool isListening = m_listener->listen();
				kDebug(14140) << (isListening ? "listening" : "not listening");
				kDebug(14140) << "local endpoint, " << m_listener->serverAddress().toString();
				
				content = "Bridge: TCPv1\r\n"
					"Listening: true\r\n" +
					QString("Hashed-Nonce: {%1}\r\n").arg(P2P::Uid::createUid()) +
					QString("IPv4Internal-Addrs: %1\r\n").arg(m_listener->serverAddress().toString())   +
					QString("IPv4Internal-Port: %1\r\n").arg(m_listener->serverPort()) +
					"\r\n";
			}
			else
			{
				content =
					"Bridge: TCPv1\r\n"
					"Listening: false\r\n"
					"Hashed-Nonce: {00000000-0000-0000-0000-000000000000}\r\n"
					"\r\n";
			}
			
			m_state = DataTransfer;
			
			if (m_type != File)
			{
				// NOTE For file transfers, the connection invite *must not* be acknowledged in any way
				//      as this trips MSN 7.5
				
				acknowledge(message);
				// Send 200 OK message to the sending client.
				sendMessage(OK, content);
			}
		}
		else if(body.startsWith("BYE"))
		{
			m_state = Finished;
			// Send the sending client an acknowledge message.
			acknowledge(message);

			if(m_file && m_transfer)
			{
				if(m_isComplete){
					// The transfer is complete.
					m_transfer->slotComplete();
				}
				else
				{
					// The transfer has been canceled remotely.
					if(m_transfer){
						// Inform the user of the file transfer cancellation.
						m_transfer->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
					}
					// Remove the partially received file.
					m_file->remove();
				}
			}

			// Dispose of this transfer context.
			m_dispatcher->detach(this);
		}
		else if(body.startsWith("MSNSLP/1.0 200 OK"))
		{
			if(m_type == UserDisplayIcon){
				m_state = Negotiation;
				// Acknowledge the 200 OK message.
				acknowledge(message);
			}
		}
	}
}

void IncomingTransfer::slotAccept()
{
	// Try to accept an incoming connection from the sending client.
	m_socket = m_listener->nextPendingConnection();
	if(!m_socket)
	{
		// NOTE If direct connection fails, the sending
		// client wil transfer the file data through the
		// existing session.
		kDebug(14140) << "Direct connection failed.";
		// Close the listening endpoint.
		m_listener->close();
		return;
	}

	kDebug(14140) << "Direct connection established.";

	// Create the callback that will try to read bytes from the accepted socket.
	QObject::connect(m_socket, SIGNAL(readyRead()),   this, SLOT(slotSocketRead()));
	// Create the callback that will try to handle the socket close event.
	QObject::connect(m_socket, SIGNAL(disconnected()),      this, SLOT(slotSocketClosed()));
	// Create the callback that will try to handle the socket error event.
	QObject::connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
}

void IncomingTransfer::slotSocketRead()
{
	int available = m_socket->bytesAvailable();
	kDebug(14140) << available << ", bytes available.";
	if(available > 0)
	{
		QByteArray buffer = m_socket->read(available);
		if(QString(buffer) == "foo"){
			kDebug(14140) << "Connection Check.";
		}
	}
}

void IncomingTransfer::slotSocketClosed()
{
	kDebug(14140) ;
}

void IncomingTransfer::slotSocketError(QAbstractSocket::SocketError errorCode)
{
	kDebug(14140) << errorCode;
}

#include "incomingtransfer.moc"
