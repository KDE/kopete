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

#include <ortp/ortp.h>

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

char* RtpPacket::data()
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

void JingleRtpManager::setMediaData(const QByteArray& data)
{
	m_mediaDataIn = data;
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

	return 0;
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

	//FIXME:Do we have to empty the packet ?
	
	JingleRtpManager *manager = JingleRtpManager::manager();
	if (!manager)
		return EXIT_FAILURE;
	
	manager->appendRtpPacketOut(new RtpPacket(msg), t);
	
	return EXIT_SUCCESS;
}

int JingleRtpSession::recvRtpFrom(RtpTransport *t, mblk_t *msg, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	Q_UNUSED(t)
	Q_UNUSED(msg)
	Q_UNUSED(flags)
	Q_UNUSED(from)
	Q_UNUSED(fromlen)

	/* oRTP wants me to provide it with Rtp data to unpack and use it.
	 * Arguments are the same as sendRtpTo().
	 */
	JingleRtpManager *m = JingleRtpManager::manager();
	if (!m)
		return EXIT_FAILURE;
	
	JingleRtpSession *sess = m->sessionWithTransport(t);
	if (!sess)
		return EXIT_FAILURE;

	m->sessionWithTransport(t)->fillRtpPacket(msg);
	
	return EXIT_SUCCESS;
}

int JingleRtpSession::sendRtcpTo(RtpTransport *t, mblk_t *msg , int flags, const struct sockaddr *to, socklen_t tolen)
{
	Q_UNUSED(t)
	Q_UNUSED(msg)
	Q_UNUSED(flags)
	Q_UNUSED(to)
	Q_UNUSED(tolen)

	return EXIT_SUCCESS;
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

void JingleRtpSession::fillRtpPacket(mblk_t *p)
{
	RtpPacket *firstPacket = 0;
	if (!rtpPacketsIn.isEmpty())
		firstPacket = rtpPacketsIn.takeFirst();
	else
		return; //FIXME:what do we do if there isn't any data incoming ?

	mblk_t *temp = rtp_session_create_packet_with_data(rtpSession(),
							   (uint8_t*) firstPacket->data(),
							   firstPacket->size(),
							   /*void (*freefn)(void*)*/ NULL);

	p->b_rptr = temp->b_rptr;
	p->b_wptr = temp->b_wptr;
	//FIXME:what about using memcpy here ?
}

/*QByteArray JingleRtpSession::getMediaDataReady(int ts)
{
	Q_UNUSED(ts)
	//FIXME : when should that code be run ?
	void *buf = new uint8_t[bufSize];
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
	
	emit readyRead(inData);
	return QByteArray();
}*/

/*void JingleRtpSession::rtcpDataReady()
{
	//kDebug() << "Received :" << rtcpSocket->readAll();
}*/

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
	emit packedReady(p);
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
