/*
    outgoingtransfer.cpp - msn p2p protocol

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

#include "outgoingtransfer.h"
//Added by qt3to4:
#include <QByteArray>
using P2P::TransferContext;
using P2P::Dispatcher;
using P2P::OutgoingTransfer;
using P2P::Message;

#include <stdlib.h>

// Kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kcodecs.h>

// Qt includes
#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtcpsocket.h>

// Kopete includes
#include <kopetetransfermanager.h>

#include <netinet/in.h> // For htonl
using P2P::TransferContext;
using P2P::Dispatcher;
using P2P::OutgoingTransfer;
using P2P::Message;

OutgoingTransfer::OutgoingTransfer(const QString& to, P2P::Dispatcher *dispatcher, quint32 sessionId)
: TransferContext(to,dispatcher,sessionId)
{
	m_direction = Outgoing;
	m_handshake = 0x01;
}

OutgoingTransfer::~OutgoingTransfer()
{
	kDebug(14140) ;
}

void OutgoingTransfer::sendImage(const QByteArray& image)
{
	Q_UNUSED(image);

// 	TODO QByteArray base64 = KCodecs::base64Encode(image);
//
// 	QCString body = "MIME-Version: 1.0\r\n"
// 		"Content-Type: image/gif\r\n"
// 		"\r\n"
// 		"base64:" +
//
// 	Message outbound;
// 	outbound.header.sessionId  = m_sessionId;
// 	outbound.header.identifier = m_baseIdentifier;
// 	outbound.header.dataOffset = 0;
// 	outbound.header.totalDataSize = 4;
// 	outbound.header.dataSize = 4;
// 	outbound.header.flag = 0;
// 	outbound.header.ackSessionIdentifier = rand()%0x8FFFFFF0 + 4;
// 	outbound.header.ackUniqueIdentifier  = 0;
// 	outbound.header.ackDataSize   = 0l;
// 	QByteArray bytes(4);
// 	bytes.fill('\0');
// 	outbound.body = bytes;
// 	outbound.applicationIdentifier = 0;
// 	outbound.attachApplicationId = false;
// 	outbound.destination = m_recipient;
//
// 	sendMessage(outbound, body);
}

void OutgoingTransfer::slotSendData()
{
	qint32 bytesRead = 0;
	QByteArray buffer;
	buffer.resize(1202);

	if(!m_file)
		return;

	// Read a chunk from the source file.
	bytesRead = m_file->read(buffer.data(), buffer.size());
	
	if (bytesRead < 0) {
		m_file->close();
                // ### error handling
        }
	else {

		if(bytesRead < 1202){
			buffer.resize(bytesRead);
		}

		kDebug(14140) << QString("Sending, %1 bytes").arg(bytesRead);

		if((m_offset + bytesRead) < m_file->size())
		{
			sendData(buffer);
			m_offset += bytesRead;
		}
		else
		{
			m_isComplete = true;
			// Send the last chunk of the file.
			sendData(buffer);
			m_offset += buffer.size();
			// Close the file.
			m_file->close();
		}
	}

	if(m_transfer){
		m_transfer->slotProcessed(m_offset);
		if(m_isComplete){
			// The transfer is complete.
			m_transfer->slotComplete();
		}
	}
}

void OutgoingTransfer::acknowledged()
{
	kDebug(14140) ;

	switch(m_state)
	{
		case Invitation:
		{
			if(m_type == UserDisplayIcon)
			{
				m_state = Negotiation;
				// Send data preparation message.
				sendDataPreparation();
			}
			break;
		}

		case Negotiation:
		{
			if(m_type == UserDisplayIcon)
			{
				// <<< Data preparation acknowledge message.
				m_state = DataTransfer;
				m_identifier++;
				// Start sending data.
				slotSendData();
			}
			break;
		}

		case DataTransfer:
			// NOTE <<< Data acknowledged message.
			// <<< Bye message should follow.
			if(m_type == File)
			{
				if(m_handshake == 0x01)
				{
					// Data handshake acknowledge message.
					// Start sending data.
					slotSendData();
				}
				else if(m_handshake == 0x02)
				{
					// Data acknowledge message.
					// Send the recipient a BYE message.
					m_state = Finished;
					sendMessage(BYE, "\r\n");
				}
			}

			break;

		case Finished:
			if(m_type == File)
			{
				// BYE acknowledge message.
				m_dispatcher->detach(this);
			}

			break;
	}
}

void OutgoingTransfer::processMessage(const Message& message)
{
	QString body =
		QByteArray(message.body.data(), message.header.dataSize);
	kDebug(14140) << "received, " << body;

	if(body.startsWith("BYE"))
	{
		m_state = Finished;
		// Send the recipient an acknowledge message.
		acknowledge(message);
		if(!m_isComplete)
		{
			// The peer canceled the transfer.
			if(m_transfer)
			{
				// Inform the user of the file transfer cancellation.
				m_transfer->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
			}
		}
		// Dispose of this transfer context.
		m_dispatcher->detach(this);
	}
	else if(body.startsWith("MSNSLP/1.0 200 OK"))
	{
		// Retrieve the message content type.
		QRegExp regex("Content-Type: ([A-Za-z0-9$!*/\\-]*)");
		regex.indexIn(body);
		QString contentType = regex.cap(1);

		if(contentType == "application/x-msnmsgr-sessionreqbody")
		{
			// Recipient has accepted the file transfer.
			// Acknowledge the recipient.
			acknowledge(message);

			// Try to open the file for reading.
			// If an error occurs, send an internal
			// error message to the recipient.
			if(!m_file->open(QIODevice::ReadOnly)){
				error();
				return;
			}

			// Retrieve the receiving client's contact.
			Kopete::Contact *contact = m_dispatcher->getContactByAccountId(m_recipient);
			if(contact == 0l)
			{
				error();
				return;
			}

			m_transfer =
				Kopete::TransferManager::transferManager()->addTransfer(contact, m_file->fileName(), m_file->size(), m_recipient, Kopete::FileTransferInfo::Outgoing);

			QObject::connect(m_transfer , SIGNAL(transferCanceled()), this, SLOT(abort()));

			m_state = Negotiation;

			m_branch = P2P::Uid::createUid();

			// Send the direct connection invitation message.
			QString content = "Bridges: TRUDPv1 TCPv1\r\n" +
				QString("NetID: %1\r\n").arg("-123657987") +
				QString("Conn-Type: %1\r\n").arg("Restrict-NAT") +
				"UPnPNat: false\r\n"
				"ICF: false\r\n" +
				QString("Hashed-Nonce: {%1}\r\n").arg(P2P::Uid::createUid()) +
				"\r\n";
			sendMessage(INVITE, content);
		}
		else if(contentType == "application/x-msnmsgr-transrespbody")
		{
			// Determine whether the recipient created
			// a listening endpoint.
			regex = QRegExp("Listening: ([^\r\n]+)\r\n");
			regex.indexIn(body);
			bool isListening = (regex.cap(1) == "true");

			// Send the recipient an acknowledge message.
			acknowledge(message);

			m_state = DataTransfer;

#if 1
			isListening = false; // TODO complete direct connection.
#endif
			if(isListening)
			{
				// Retrieve the hashed nonce for this direct connection instance.
				regex = QRegExp("Hashed-Nonce: \\{([0-9A-F\\-]*)\\}\r\n");
				regex.indexIn(body);
				m_nonce = regex.cap(1);
				// Retrieve the listening endpoints of the receiving client.
				regex = QRegExp("IPv4Internal-Addrs: ([^\r\n]+)\r\n");
				regex.indexIn(body);
				m_peerEndpoints = regex.cap(1).split( ' ', QString::SkipEmptyParts );
				m_endpointIterator = m_peerEndpoints.begin();
				// Retrieve the listening port of the receiving client.
				regex = QRegExp("IPv4Internal-Port: ([^\r\n]+)\r\n");
				regex.indexIn(body);
				m_remotePort = regex.cap(1).toUInt();

				// Try to connect to the receiving client's
				// listening endpoint.
				connectToEndpoint(*m_endpointIterator);
			}
			else
			{
				m_handshake = 0x02;
				// Otherwise, send data through the already
				// existing session.
				slotSendData();
			}
		}
	}
	else if(body.startsWith("MSNSLP/1.0 603 Decline"))
	{
		// File transfer has been canceled remotely.
		// Send an acknowledge message
		acknowledge(message);
		if(m_transfer)
		{
			// Inform the user of the file transfer cancellation.
			m_transfer->slotError(KIO::ERR_ABORTED, i18n("File transfer cancelled."));
		}

		if(m_file && m_file->isOpen()){
			// Close the file.
			m_file->close();
		}
		m_dispatcher->detach(this);
	}
}

void OutgoingTransfer::readyToSend()
{
	if(m_isComplete){
		// Ignore, do nothing.
		return;
	}

	slotSendData();
}

void OutgoingTransfer::connectToEndpoint(const QString& hostName)
{
	m_socket = new QTcpSocket();

	QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotRead()));
	QObject::connect(m_socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	QObject::connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
	QObject::connect(m_socket, SIGNAL(disconnected()), this, SLOT(slotSocketClosed()));
	// Try to connect to the endpoint.
	m_socket->connectToHost(hostName, m_remotePort);
}

void OutgoingTransfer::slotConnected()
{
	kDebug(14140) ;
	// Check if connection is ok.
	quint32 bytesWritten = m_socket->write(QByteArray("foo"));
	if(bytesWritten != 4)
	{
		// Not all data was written, close the socket.
		m_socket->close();
		// Schedule the data to be sent through the existing session.
		QTimer::singleShot(2000, this, SLOT(slotSendData()));
		return;
	}

	// Send data handshake message.
	P2P::Message handshake;
	handshake.header.sessionId  = 0;
	handshake.header.identifier = ++m_identifier;
	handshake.header.dataOffset = 0l;
	handshake.header.totalDataSize = 0l;
	handshake.header.dataSize = 0;
	// Set the flag to indicate that this is
	// a direct connection handshake message.
	handshake.header.flag = 0x100;
	QString nonce = m_nonce.remove('-');
	handshake.header.ackSessionIdentifier = nonce.mid(0, 8).toUInt(0, 16);
	handshake.header.ackUniqueIdentifier  =
		nonce.mid(8, 4).toUInt(0, 16) | (nonce.mid(12, 4).toUInt(0, 16) << 16);
	const quint32 lo = nonce.mid(16, 8).toUInt(0, 16);
	const quint32 hi = nonce.mid(24, 8).toUInt(0, 16);
	handshake.header.ackDataSize =
		((qint64)htonl(lo)) | (((qint64)htonl(hi)) << 32);

	QByteArray stream;
	// Write the message to the memory stream.
	m_messageFormatter.writeMessage(handshake, stream, true);
	// Send the byte stream over the wire.
	m_socket->write(stream);
}

void OutgoingTransfer::slotRead()
{
	qint32 bytesAvailable = m_socket->bytesAvailable();
	kDebug(14140) << bytesAvailable << ", bytes available.";
}

void OutgoingTransfer::slotSocketError(QAbstractSocket::SocketError)
{
	kDebug(14140) << m_socket->errorString();
	// If an error has occurred, try to connect
	// to another available peer endpoint.
	// If there are no more available endpoints,
	// send the data through the session.
	m_socket->close();

	// Move to the next available endpoint.
	m_endpointIterator++;
	if(m_endpointIterator != m_peerEndpoints.end()){
		// Try to connect to the endpoint.
		connectToEndpoint(*m_endpointIterator);
	}
	else
	{
		// Otherwise, send the data through the session.
		m_identifier -= 1;
		QTimer::singleShot(2000, this, SLOT(slotSendData()));
	}
}

void OutgoingTransfer::slotSocketClosed()
{
	kDebug(14140) ;
	m_socket->deleteLater();
	m_socket = 0l;
}

#include "outgoingtransfer.moc"
