/*
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

#include "jinglertpmanager.h"
//#include "mediasession.h"

//#include <ortp/payloadtype.h>
#include <ortp/ortp.h>
//#include <ortp/rtpsession.h>

#include <QUdpSocket>
#include <QImage>
#include <QByteArray>
#include <QList>
#include <KDebug>
#include <QDomElement>

static JingleRtpManager* man = 0;

/*
 * RtpPacket
 */

RtpPacket::RtpPacket()
{
	m_data = NULL;
	m_size = 0;
}

RtpPacket::RtpPacket(mblk_t *msg)
{
	m_data = (char*)msg->b_rptr;
	m_size = (uint8_t)(msg->b_wptr - msg->b_rptr);
}

RtpPacket::RtpPacket(const QByteArray& data)
{
	m_data = (char*)data.data();
	m_size = data.size();
}

/*
 * Destructor does not delete the data
 */
RtpPacket::~RtpPacket()
{
	
}

char* RtpPacket::getData()
{
	return m_data;
}

size_t RtpPacket::size()
{
	return m_size;
}

void RtpPacket::setType(RtpType type)
{
	m_type = type;
}

RtpPacket::RtpType RtpPacket::type() const
{
	return m_type;
}

/*
 * JingleRtpManager
 */
JingleRtpManager::JingleRtpManager()
{
	man = this;
}

JingleRtpManager::~JingleRtpManager()
{

}

JingleRtpManager *JingleRtpManager::manager()
{
	if (!man)
		man = new JingleRtpManager();

	return man;
}

void JingleRtpManager::appendRtpPacketOut(RtpPacket* packet, RtpTransport* transport)
{
	foreach(JingleRtpSession *s, m_sessions)
	{
		if (transport == s->rtpTransport())
		{
			packet->setType(RtpPacket::Rtp);
			s->appendRtpPacketOut(packet);
			break;
		}
		
		if (transport == s->rtcpTransport())
		{
			packet->setType(RtpPacket::Rtcp);
			s->appendRtpPacketOut(packet);
			break;
		}
	}
}

void JingleRtpManager::appendSession(JingleRtpSession *sess)
{
	m_sessions << sess;
}

void JingleRtpManager::fillPacket(mblk_t *p, RtpTransport *t)
{
	JingleRtpSession *sess = sessionWithTransport(t);
	if (!sess)
		return;

	/*FIXME:How to get payload ?*/
	mblk_t *temp = rtp_session_create_packet_with_data(sess->rtpSession(), uint8_t *payload, int payload_size, /*void (*freefn)(void*)*/ NULL);
	
	/*
	 * m_data = (char*)msg->b_rptr;
	 * m_size = (uint8_t)(msg->b_wptr - msg->b_rptr);
	 */
	
	p->b_rptr = temp->b_rtpr;
	p->b_wptr = temp->b_wtpr;
}

JingleRtpSession* JingleRtpManager::sessionWithTransport(RtpTransport *t)
{
	foreach(JingleRtpSession *s, m_sessions)
	{
		if (s->rtpTransport() == t || s->rtcpTransport() == t)
		{
			return s;
		}
	}
}

/*
 * JingleRtpSession.cpp -- provides an rtp packager for incoming and outgoing media packet.
 */
JingleRtpSession::JingleRtpSession() 
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Creating JingleRtpSession";

	JingleRtpManager *manager = JingleRtpManager::manager();
	manager->appendSession(this);

	payloadID = -1;
	payloadName = "";
	bufSize = 0;

	m_rtpSession = rtp_session_new(RTP_SESSION_SENDRECV);

	//We are using RtpTransport's to pack and unpack data.
	m_rtpTransport.t_sendto = sendRtpTo;
	m_rtpTransport.t_recvfrom = JingleRtpSession::recvRtpFrom;
	
	m_rtcpTransport.t_sendto = JingleRtpSession::sendRtcpTo;
	m_rtcpTransport.t_recvfrom = JingleRtpSession::recvRtcpFrom;
	
	rtp_session_set_transports(m_rtpSession, &m_rtpTransport, &m_rtcpTransport);

	rtp_session_set_scheduling_mode(m_rtpSession, 0);
	rtp_session_set_blocking_mode(m_rtpSession, 0);

	kDebug() << "Created";
}

JingleRtpSession::~JingleRtpSession()
{
	kDebug() << "destroyed";
	rtp_session_bye(m_rtpSession, "Ended");
	rtp_session_destroy(m_rtpSession);
}

int JingleRtpSession::sendRtpTo(RtpTransport *t, mblk_t *msg , int flags, const struct sockaddr *to, socklen_t tolen)
{
	Q_UNUSED(t)
	Q_UNUSED(flags)
	Q_UNUSED(to)
	Q_UNUSED(tolen)

	/* oRTP wants me to send an RTP packet to the peer. Let's do that !
	 * 	t 	is the transport.
	 * 	msg 	is the data to send.
	 * 	flags 	unused.
	 * 	to	to address, unused. (iris doesn't give the ip address)
	 * 	tolen	to address length, unused.
	 */
	
	JingleRtpManager *manager = JingleRtpManager::manager();
	if (manager)
		manager->appendRtpPacketOut(new RtpPacket(msg), t);
	
	return 0;
}

int JingleRtpSession::recvRtpFrom(RtpTransport *t, mblk_t *msg, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	Q_UNUSED(t)
	Q_UNUSED(msg)
	Q_UNUSED(flags)
	Q_UNUSED(from)
	Q_UNUSED(fromlen)

	/* oRTP wants me to provide it with RTP data to unpack and use it.
	 * Arguments are the same as sendRtpTo().
	 */
	JingleRtpManager *m = manager();
	if (!m)
		return 0;
	
	m->fillPacket(msg, t);
	
	//Fill the mblk_t structure.
	
	//msg()
	return 0;
}

int JingleRtpSession::sendRtcpTo(RtpTransport *t, mblk_t *msg , int flags, const struct sockaddr *to, socklen_t tolen)
{
	Q_UNUSED(t)
	Q_UNUSED(msg)
	Q_UNUSED(flags)
	Q_UNUSED(to)
	Q_UNUSED(tolen)

	return 0;
}

int JingleRtpSession::recvRtcpFrom(RtpTransport *t, mblk_t *msg, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	Q_UNUSED(t)
	Q_UNUSED(msg)
	Q_UNUSED(flags)
	Q_UNUSED(from)
	Q_UNUSED(fromlen)

	return 0;
}

/*void JingleRtpSession::setRtpSocket(QAbstractSocket* socket, int rtcpPort)
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
		rtcpSocket->bind(rtpSocket->localAddress(), rtcpPort == 0 ? rtpSocket->localPort() + 1 : rtcpPort);
	}
	else if (m_direction == Out)
	{
		int peerPort = socket->peerPort();
		QHostAddress peerAddress = socket->peerAddress();
		delete socket;
		rtpSocket->connectToHost(peerAddress, peerPort); //Part of the workaround

		kDebug() << "Given socket is connected to" << rtpSocket->peerAddress() << ":" << rtpSocket->peerPort();
		kDebug() << "RTCP socket will be connected to" << rtpSocket->peerAddress() << ":" << (rtcpPort == 0 ? rtpSocket->peerPort() + 1 : rtcpPort);
		rtcpSocket->connectToHost(rtpSocket->peerAddress(), rtcpPort == 0 ? rtpSocket->peerPort() + 1 : rtcpPort, QIODevice::ReadWrite);
	}

	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}*/

/*void JingleRtpSession::send(const QByteArray& outData)
{
	mblk_t *packet = rtp_session_create_packet_with_data(m_rtpSession, (uint8_t*)outData.data(), outData.size(), NULL); //the free function is managed by the bytesWritten signal
	
	int ts = m_mediaSession->timeStamp(); //FIXME:Timestamp as argument !
	int size = rtp_session_sendm_with_ts(m_rtpSession, packet, ts);
	if (size == -1)
	{
		kDebug() << "Error sending packet";
		return;
	}
}*/

QByteArray JingleRtpSession::getMediaDataReady(int ts)
{
	Q_UNUSED(ts)
	//FIXME : when should that code be run ?
/*	void *buf = new uint8_t[bufSize];
	int more;
	
	int ret = rtp_session_recv_with_ts(m_rtpSession, static_cast<uint8_t*>(buf), bufSize, ts, &more);
	if (ret == 0)
	{
		kDebug() << "Error receiving Rtp packet. (Most likely this timestamp has expired)";
		if (more != 0)
			kDebug() << "Still some data to read";

		kDebug() << "Purging the socket.";
		QByteArray b;
		b.resize(rtpSocket->pendingDatagramSize());
		rtpSocket->readDatagram(b.data(), rtpSocket->pendingDatagramSize());
		return;
	}

	inData.resize(bufSize);
	inData = static_cast<char*>(buf);
	
	// Seems we should empty the socket...
	QByteArray b;
	b.resize(rtpSocket->pendingDatagramSize());
	rtpSocket->readDatagram(b.data(), rtpSocket->pendingDatagramSize());
	
	emit readyRead(inData);*/
	return QByteArray();
}

void JingleRtpSession::rtcpDataReady()
{
	//kDebug() << "Received :" << rtcpSocket->readAll();
}

void JingleRtpSession::setPayload(const QDomElement& payload)
{
	Q_UNUSED(payload)
	//TODO:Parse QDomElement here and store data.
	payloadName = "speex";
	bufSize = 38;
	payloadID = 96;
	RtpProfile *profile = rtp_profile_new(payloadName.toAscii());
	rtp_profile_set_payload(profile, 96, &payload_type_speex_nb);
	rtp_session_set_profile(m_rtpSession, profile);
	rtp_session_set_payload_type(m_rtpSession, 96);
}

/*void JingleRtpSession::setMediaSession(MediaSession *sess)
{
	m_mediaSession = sess;
}*/

void JingleRtpSession::pack(const QByteArray& data, int ts)
{
	/* 
	 * This data that will be "packed" will be prepared to be sent.
	 * Once it has been packed, the RTP transport will execute t_sendto()
	 * which is a pointer to JingleRtpSession::sendRtpTo() here.
	 */
	mblk_t *packet =
	       rtp_session_create_packet_with_data(m_rtpSession,
			       			   (uint8_t*)data.data(),
						   data.size(),
						   NULL);
	
	//FIXME: what about rtp_session_get_current_send_ts() ?
	int size = rtp_session_sendm_with_ts(m_rtpSession, packet, ts);

	if (size == -1)
	{
		kDebug() << "Error sending packet";
		return;
	}
}

void JingleRtpSession::unpack(const QByteArray& data)
{
	rtpPacketsIn << new RtpPacket(data);
	
	/* Here, we must create a mblk_t packet and then, put it in a queue
	 * which will be emptied by JingleRtpSession::recvRtpFrom()
	 * (that method is called by oRTP).
	 */
}

QByteArray JingleRtpSession::getPacked()
{
	return QByteArray();
}

QByteArray JingleRtpSession::getUnpacked()
{
	return QByteArray();
}

void JingleRtpSession::appendRtpPacketOut(RtpPacket* p)
{
	emit packetOutReady(p);
}

RtpTransport* JingleRtpSession::rtpTransport() const
{
	return (RtpTransport*) &m_rtpTransport;
}

RtpTransport* JingleRtpSession::rtcpTransport() const
{
	return (RtpTransport*) &m_rtcpTransport;
}

RtpSession* JingleRtpSession::rtpSession() const
{
	return m_rtpSession;
}
