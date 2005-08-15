/*
    outgoingtransfer.h - msn p2p protocol

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

#include "outgoingtransfer.h"
using P2P::TransferContext;
using P2P::Dispatcher;
using P2P::OutgoingTransfer;
using P2P::Message;

#include <stdlib.h>

// Kde includes
#include <kbufferedsocket.h>
#include <kdebug.h>
#include <klocale.h>
using namespace KNetwork;

// Qt includes
#include <qfile.h>
#include <qregexp.h>
#include <qtimer.h>

// Kopete includes
#include <kopetetransfermanager.h>

OutgoingTransfer::OutgoingTransfer(const QString& to, P2P::Dispatcher *dispatcher, Q_UINT32 sessionId)
: TransferContext(dispatcher)
{
	m_direction = Outgoing;
	m_handshake = 0x01;
	m_sessionId  = sessionId;
	m_recipient  = to;
	m_offset = 0l;
}

OutgoingTransfer::~OutgoingTransfer()
{
	kdDebug(14140) << k_funcinfo << endl;
}

void OutgoingTransfer::slotSendData()
{		
	Q_INT32 bytesRead = 0;
	QByteArray buffer(1202);
	if(m_file){
		// Read a chunk from the source file.
		bytesRead = m_file->readBlock(buffer.data(), buffer.size());
	}

	if(bytesRead < 1202){
		buffer.resize(bytesRead);
	}

	kdDebug(14140) << k_funcinfo << QString("Sending, %1 bytes").arg(bytesRead) << endl;
	
	if((m_offset + bytesRead) < m_file->size())
	{
		sendData(buffer);
		m_offset += bytesRead;
		if(m_transfer){
			m_transfer->slotProcessed(m_offset);
		}
// 		QTimer::singleShot(10, this, SLOT(slotSendData()));
	}
	else
	{
		// Send the last chunk of the file.
		sendData(buffer);
		if(m_transfer){
			m_transfer->slotProcessed(m_file->size());
		}
		// Close the file.
		m_file->close();

		if(m_transfer) m_transfer->slotComplete();
	}
}

void OutgoingTransfer::acknowledged()
{
	kdDebug(14140) << k_funcinfo << endl;
	
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
		QCString(message.body.data(), message.header.dataSize);
	kdDebug(14140) << k_funcinfo << "received, " << body << endl;

	if(body.startsWith("BYE"))
	{
		m_state = Finished;
		// Send the recipient an acknowledge message.
		acknowledge(message);
		// Dispose of this transfer context.
		m_dispatcher->detach(this);
	}
	else if(body.startsWith("MSNSLP/1.0 200 OK"))
	{
		// Retrieve the message content type.
		QRegExp regex("Content-Type: ([A-Za-z0-9$!*/\\-]*)");
		regex.search(body);
		QString contentType = regex.cap(1);

		if(contentType == "application/x-msnmsgr-sessionreqbody")
		{
			// Recipient has accepted the file transfer.
			// Acknowledge the recipient.
			acknowledge(message);

			// Try to open the file for reading.
			// If an error occurs, send an internal
			// error message to the recipient.
			if(!m_file->open(IO_ReadOnly)){
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
				Kopete::TransferManager::transferManager()->addTransfer(contact, m_file->name(), m_file->size(), m_recipient, Kopete::FileTransferInfo::Outgoing);

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
			regex.search(body);
			bool isListening = (regex.cap(1) == "true");

			// Send the recipient an acknowledge message.
			acknowledge(message);

			m_state = DataTransfer;

			isListening = false; // TODO complete direct connection.
				
			if(isListening)
			{
				// Retrieve the hashed nonce for this direct connection instance.
				regex = QRegExp("Hashed-Nonce: \\{([0-9A-F\\-]*)\\}\r\n");
				regex.search(body);
				m_nonce = regex.cap(1);
				// Retrieve the listening endpoints of the receiving client.
				regex = QRegExp("IPv4Internal-Addrs: ([^\r\n]+)\r\n");
				regex.search(body);
				m_peerEndpoints = QStringList::split(" ", regex.cap(1));
				m_endpointIterator = m_peerEndpoints.begin();
				// Retrieve the listening port of the receiving client.
				regex = QRegExp("IPv4Internal-Port: ([^\r\n]+)\r\n");
				regex.search(body);
				m_remotePort = regex.cap(1);

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
		// File transfer has been cancelled remotely.
		// Send an acknowledge message
		acknowledge(message);
		if(m_transfer)
		{
			// Inform the user of the file transfer cancelation.
			m_transfer->slotError(KIO::ERR_USER_CANCELED, i18n("File transfer cancelled."));
			m_transfer = 0l; // TODO handle error
		}

		if(m_file){
			// Close the file.
			m_file->close();
		}
		m_dispatcher->detach(this);
	}
}

void OutgoingTransfer::readyToSend()
{
	if(m_file){
		// If the file is not open, do nothing.
		if(!m_file->isOpen()) return;
	}
		
	slotSendData();
}

void OutgoingTransfer::connectToEndpoint(const QString& hostName)
{
	m_socket = new KBufferedSocket(hostName, m_remotePort);
	m_socket->setBlocking(false);
	m_socket->enableRead(true);
	// Disable write signal for now.  Only enable
	// when we are ready to sent data.
	// NOTE readyWrite consumes too much cpu usage.
	m_socket->enableWrite(false);
	QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(slotRead()));
	QObject::connect(m_socket, SIGNAL(connected(const KResolverEntry&)), this, SLOT(slotConnected()));
	QObject::connect(m_socket, SIGNAL(gotError(int)), this, SLOT(slotSocketError(int)));
	QObject::connect(m_socket, SIGNAL(closed()), this, SLOT(slotSocketClosed()));
	// Try to connect to the endpoint.
	m_socket->connect();
}

void OutgoingTransfer::slotConnected()
{
	kdDebug(14140) << k_funcinfo << endl;
	// Check if connection is ok.
	Q_UINT32 bytesWritten = m_socket->writeBlock(QCString("foo").data(), 4);
	if(bytesWritten != 4)
	{
		// Not all data was written, close the socket.
		m_socket->closeNow();
		// Send the data through the session.
		slotSendData();
		return;
	}

	// TODO Send data handshake message.
}

void OutgoingTransfer::slotRead()
{}

void OutgoingTransfer::slotSocketError(int)
{
	kdDebug(14140) << k_funcinfo << endl;
	// If an error has occurred, try to connect
	// to another available peer endpoint.
	// If there are no more available endpoints,
	// send the data through the session.
	delete m_socket;
	m_socket = 0l;

	// Move to the next available endpoint.
	m_endpointIterator++;
	if(m_endpointIterator != m_peerEndpoints.end()){
		// Try to connect to the endpoint.
		connectToEndpoint(*m_endpointIterator);
	}
	else
	{
		// Otherwise, send the data through the session.
		slotSendData();
	}
}

void OutgoingTransfer::slotSocketClosed()
{
	kdDebug(14140) << k_funcinfo << endl;
	m_socket->deleteLater();
	m_socket = 0l;
}

#include "outgoingtransfer.moc"
