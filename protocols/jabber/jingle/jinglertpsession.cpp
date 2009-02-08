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

JingleRtpSession::JingleRtpSession()
{
	inPort = 60000 + (rand() % 999);
	outPort = 61000 + (rand() % 999);
	kDebug() << "Creating JingleRtpSession";
	/*
	 * The RtpSession objects represent a RTP session:
	 * once it is configured with local and remote network addresses and a payload type is given,
	 * it let you send and recv a media stream.
	 */
	
	m_rtpInSession = rtp_session_new(RTP_SESSION_RECVONLY);
	m_rtpOutSession = rtp_session_new(RTP_SESSION_SENDONLY);

	payloadID = -1;
	payloadName = "";
	bufSize = 0;
	
	rtpInSocket = new QUdpSocket();
	rtpInSocket->bind(QHostAddress::LocalHost, inPort);
	connect(rtpInSocket, SIGNAL(readyRead()), this, SIGNAL(mediaDataReady()));
	rtcpInSocket = new QUdpSocket();
	
	writeSocket = new QUdpSocket();
	writeSocket->connectToHost(QHostAddress::LocalHost, inPort);
	
	readSocket = new QUdpSocket();
	connect(readSocket, SIGNAL(readyRead()), this, SIGNAL(rtpDataReady()));
	readSocket->bind(QHostAddress::LocalHost, outPort);

	rtpOutSocket = new QUdpSocket();
	rtpOutSocket->connectToHost(QHostAddress::LocalHost, outPort);
	rtcpOutSocket = new QUdpSocket();

	setRtpSockets();

	rtp_session_set_scheduling_mode(m_rtpOutSession, 0);
	rtp_session_set_blocking_mode(m_rtpOutSession, 0);
	rtp_session_set_scheduling_mode(m_rtpInSession, 0);
	rtp_session_set_blocking_mode(m_rtpInSession, 0);
	kDebug() << "Created";
}

void JingleRtpSession::slotA()
{
	QUdpSocket *socket = dynamic_cast<QUdpSocket*>(sender());
	kDebug() << socket->hasPendingDatagrams();
	kDebug() << socket->pendingDatagramSize();
	kDebug() << "called";
}

void JingleRtpSession::slotB()
{
	QUdpSocket *socket = dynamic_cast<QUdpSocket*>(sender());
	kDebug() << socket->hasPendingDatagrams();
	kDebug() << socket->pendingDatagramSize();
	QByteArray b;
	b.resize(socket->pendingDatagramSize());
	socket->readDatagram(b.data(), socket->pendingDatagramSize());
	kDebug() << "called";
}

JingleRtpSession::~JingleRtpSession()
{
	rtp_session_bye(m_rtpOutSession, "Ended");
	rtp_session_destroy(m_rtpInSession);
	rtp_session_destroy(m_rtpOutSession);
	
	delete rtpInSocket;
	delete rtpOutSocket;
	delete rtcpInSocket;
	delete rtcpOutSocket;
	delete readSocket;
	delete writeSocket;

	kDebug() << "destroyed";
}

void JingleRtpSession::setRtpSockets()
{
	// Data to unpack
	rtp_session_set_sockets(m_rtpInSession, rtpInSocket->socketDescriptor(), rtcpInSocket->socketDescriptor());
		
	// Data to pack
	rtp_session_set_sockets(m_rtpOutSession, rtpOutSocket->socketDescriptor(), rtcpOutSocket->socketDescriptor());
}

void JingleRtpSession::setPayload(const QDomElement& payload)
{
	payloadName = payload.attribute("name");
	bufSize = 38; //FIXME:How do I know that ?
	payloadID = payload.attribute("id").toInt();
	RtpProfile *profile = rtp_profile_new(payloadName.toAscii());
	rtp_profile_set_payload(profile, payloadID, &payload_type_speex_nb);
	
	rtp_session_set_profile(m_rtpOutSession, profile);
	rtp_session_set_payload_type(m_rtpOutSession, payloadID);
	rtp_session_set_profile(m_rtpInSession, profile);
	rtp_session_set_payload_type(m_rtpInSession, payloadID);
}

void JingleRtpSession::setMediaSession(MediaSession *sess)
{
	m_mediaSession = sess;
}

QByteArray JingleRtpSession::takeMediaData()
{
	//kDebug() << "called";
	// Get media data from oRTP.
	void *buf = new uint8_t[bufSize];
	int more;
	
	int ts = m_mediaSession->timeStamp();

	int err = rtp_session_recv_with_ts(m_rtpInSession, static_cast<uint8_t*>(buf), bufSize, ts, &more);
	if (err == 0)
	{
		kDebug() << "Error receiving Rtp packet. (Most likely this timestamp has expired)";
		if (more != 0)
			kDebug() << "Still some data to read";

		kDebug() << "Purging the socket.";
		QByteArray b;
		b.resize(rtpInSocket->pendingDatagramSize());
		rtpInSocket->readDatagram(b.data(), rtpInSocket->pendingDatagramSize());
		return QByteArray();
	}

	inData.resize(bufSize);
	inData = static_cast<char*>(buf);
	
	// Seems we should empty the socket...
	QByteArray b;
	b.resize(rtpInSocket->pendingDatagramSize());
	rtpInSocket->readDatagram(b.data(), rtpInSocket->pendingDatagramSize());
	
	QByteArray ret = inData;
	inData.clear();
	return ret;
}

QByteArray JingleRtpSession::takeRtpData()
{
	QByteArray ret;
	ret.resize(readSocket->pendingDatagramSize());
	readSocket->readDatagram(ret.data(), readSocket->pendingDatagramSize());
	return ret;
}

void JingleRtpSession::unpackRtpData(const QByteArray& rtpData)
{
	//kDebug() << "called";
	writeSocket->write(rtpData);
}

void JingleRtpSession::packMediaData(const QByteArray& mediaData)
{
	//kDebug() << "called";
	// Write Media data to oRTP.
	mblk_t *packet = rtp_session_create_packet_with_data(m_rtpOutSession, (uint8_t*)mediaData.data(), mediaData.size(), /*freefn*/ NULL); //the free function is managed by the bytesWritten signal
	
	int ts = m_mediaSession->timeStamp();
	int size = rtp_session_sendm_with_ts(m_rtpOutSession, packet, ts);
	if (size == -1)
	{
		kDebug() << "Error sending packet";
		return;
	}
}

