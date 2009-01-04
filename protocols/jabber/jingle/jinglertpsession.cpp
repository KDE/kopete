/*
 * jinglertpsession.cpp -- provides an rtp packager for incoming and outgoing audio packet.
 *
 * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
 *
 * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
 *
 * *************************************************************************
 * *                                                                       *
 * * This program is free software; you can redistribute it and/or modify  *
 * * it under the terms of the GNU General Public License as published by  *
 * * the Free Software Foundation; either version 2 of the License, or     *
 * * (at your option) any later version.                                   *
 * *                                                                       *
 * *************************************************************************
 */

#include "jinglertpsession.h"
#include "mediasession.h"

#include <ortp/payloadtype.h>

#include <QUdpSocket>
#include <QImage>
#include <QByteArray>
#include <KDebug>
#include <QDomElement>

JingleRtpSession::JingleRtpSession(Direction d)
{
	m_direction = d;
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Creating" << (d == In ? "IN" : "OUT") << "JingleRtpSession";
	/*
	 * The RtpSession objects represent a RTP session:
	 * once it is configured with local and remote network addresses and a payload type is given,
	 * it let you send and recv a media stream.
	 */
	
/*	rtpSocket = new QUdpSocket(this); //Should it really be created and connected here ???
	if (d == In)
		connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
	else if (d == Out)
		connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
	
	rtcpSocket = new QUdpSocket(this);
	//connect(rtcpSocket, SIGNAL(readyRead()), this, SLOT(rtcpDataReady())); // FIXME:Not sure I must do this, oRTP will manage that, I don't care about this signal.
*/	
	m_rtpSession = rtp_session_new(m_direction == In ? RTP_SESSION_RECVONLY : RTP_SESSION_SENDONLY);
	//rtp_session_set_connected_mode(m_rtpSession, true);

	payloadID = -1;
	payloadName = "";
	bufSize = 0;
	receivingTS = 0;
	sendingTS = 0;
	rtpSocket = 0;
	rtcpSocket = 0;

	rtp_session_set_scheduling_mode(m_rtpSession, 0);
	rtp_session_set_blocking_mode(m_rtpSession, 0);
	kDebug() << "Created";
}

JingleRtpSession::~JingleRtpSession()
{
	kDebug() << "destroyed";
	rtp_session_bye(m_rtpSession, "Ended");
	rtp_session_destroy(m_rtpSession);
	
	if (rtpSocket)
		delete rtpSocket;

	if (rtcpSocket)
		delete rtcpSocket;
}

/*void JingleRtpSession::connectToHost(const QString& address, int rtpPort, int rtcpPort)
{
	kDebug() << "redo connect to port" << rtpPort << "and host" << address;
	rtpSocket->connectToHost(address, rtpPort, m_direction == In ? QIODevice::ReadOnly : QIODevice::WriteOnly);
	
	rtcpSocket->connectToHost(address, rtcpPort == 0 ? rtpPort + 1 : rtcpPort, QIODevice::ReadWrite);

	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}*/

void JingleRtpSession::setRtpSocket(QAbstractSocket* socket, int rtcpPort)
{
	kDebug() << (socket->isValid() ? "Socket ready" : "Socket not ready");
	
	//rtpSocket = (QUdpSocket*) socket;
	
	// WARNING, this is a workaround, that's not clean code.
	// What I do here is that I create a new socket for RTP with information which are in the old one.
	// that means we have to re-bind or re-connect it.
	// As long as we are here, the connection is possible with that socket and
	// UDP policy does not prevent me to do that.
	
	rtpSocket = new QUdpSocket(this); //Part of the workaround
	rtcpSocket = new QUdpSocket(this);
	
	if (m_direction == In)
	{
		int localPort = socket->localPort();
		delete socket;
		rtpSocket->bind(localPort); // ^ Part of the workaround

		kDebug() << "Given socket is bound to :" << rtpSocket->localPort();
		kDebug() << "RTCP socket will be bound to :" << (rtcpPort == 0 ? rtpSocket->localPort() + 1 : rtcpPort);
		connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
		connect(rtcpSocket, SIGNAL(readyRead()), this, SLOT(rtcpDataReady()));
		rtcpSocket->bind(/*rtpSocket->localAddress(), */rtcpPort == 0 ? rtpSocket->localPort() + 1 : rtcpPort);
	}
	else if (m_direction == Out)
	{
		int peerPort = socket->peerPort();
		QHostAddress peerAddress = socket->peerAddress();
		delete socket;
		rtpSocket->connectToHost(peerAddress, peerPort); //Part of the workaround

		kDebug() << "Given socket is connected to" << rtpSocket->peerAddress() << ":" << rtpSocket->peerPort();
		kDebug() << "RTCP socket will be connected to" << rtpSocket->peerAddress() << ":" << (rtcpPort == 0 ? rtpSocket->peerPort() + 1 : rtcpPort);
		connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
		rtcpSocket->connectToHost(rtpSocket->peerAddress(), rtcpPort == 0 ? rtpSocket->peerPort() + 1 : rtcpPort, QIODevice::ReadWrite);
	}

	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}

/*void JingleRtpSession::bind(int rtpPort, int rtcpPort)
{
	kDebug() << "redo bind to port" << rtpPort;
	rtpSocket->bind(rtpPort);

	rtcpSocket->bind(rtcpPort == 0 ? rtpPort + 1 : rtcpPort);
}*/

void JingleRtpSession::send(const QByteArray& outData, int t) //TODO:There should be overloaded methods to support other data type (QString, const *char).
{
	Q_UNUSED(t)
	//kDebug() << "Send data";
	//kDebug() << data.size() << "bytes";
	
	//for (int i = 0; i < data.count(); i++)
	//	printf("'%c' ", data[i]);
	//printf("\n");
	//fflush(stdout);
	
	//if (payloadID == -1)
	//	return;
	
	//kDebug() << "Prepare a packet with" << data.size() << "bytes.";
	mblk_t *packet = rtp_session_create_packet_with_data(m_rtpSession, (uint8_t*)outData.data(), outData.size(), /*freefn*/ NULL); //the free function is managed by the bytesWritten signal
	int ts = m_mediaSession->timeStamp();
	
	int size = rtp_session_sendm_with_ts(m_rtpSession, packet, ts);
	if (size == -1)
	{
		kDebug() << "Error sending packet";
		return;
	}
	
//	kDebug() << "Bytes sent :" << size;

	sendingTS += payloadTS;
}

void JingleRtpSession::rtpDataReady()
{
	/*
	 * Timestamp :
	 * The timestamp must not be increased each time we receive data but it
	 * must be taken from the media manager which should increase it
	 * automatically.
	 */
	//kDebug() << "Incoming data ready to be read !";
	void *buf = new uint8_t[bufSize];
	int more;
	//int times = 0;
	
	/*while (rtp_session_recv_with_ts(m_rtpSession, static_cast<uint8_t*>(buf), bufSize, receivingTS, &more) == 0)
	{
	//	kDebug() << "No packet received.";
		receivingTS += payloadTS; //Must be increased for unknown reason.
		//return;
		if (++times == 50)
			return;
		//FIXME:no!
	}*/

	int ts = m_mediaSession->timeStamp();

	int ret = rtp_session_recv_with_ts(m_rtpSession, static_cast<uint8_t*>(buf), bufSize, ts, &more);
	if (ret == 0)
	{
		kDebug() << "Error receiving Rtp packet.";
		if (more != 0)
			kDebug() << "Still some data to read";

		kDebug() << "Purging socket...";
		QByteArray b;
		b.resize(rtpSocket->pendingDatagramSize());
		rtpSocket->readDatagram(b.data(), rtpSocket->pendingDatagramSize());
		return;
	}

	
	inData.resize(bufSize);
	inData = static_cast<char*>(buf);
	//QByteArray data(static_cast<char*>(buf), bufSize);
	
	// Seems we should empty the socket...
	QByteArray b;
	b.resize(rtpSocket->pendingDatagramSize());
	rtpSocket->readDatagram(b.data(), rtpSocket->pendingDatagramSize());

	//kDebug() << "Data :" << data.toBase64() << "(" << data.size() << "bytes)";
	
	emit readyRead(inData);
}

void JingleRtpSession::rtcpDataReady()
{
	//kDebug() << "Received :" << rtcpSocket->readAll();
}

void JingleRtpSession::setPayload(const QDomElement& payload)
{
	Q_UNUSED(payload)
	// Parse QDomElement here and store data.
	//payloadTS must be set here.
	payloadName = "speex";
	bufSize = 38;
	payloadID = 96;
	payloadTS = 160;
	RtpProfile *profile = rtp_profile_new(payloadName.toAscii());
	rtp_profile_set_payload(profile, 96, &payload_type_speex_nb);
	rtp_session_set_profile(m_rtpSession, profile);
	rtp_session_set_payload_type(m_rtpSession, 96);
}

void JingleRtpSession::slotBytesWritten(qint64 size)
{
	Q_UNUSED(size)
	//kDebug() << size << "bytes written";
	//if (state != SendingData)
	//	return;
	//emit dataSent();
}

void JingleRtpSession::setMediaSession(MediaSession *sess)
{
	m_mediaSession = sess;
}
