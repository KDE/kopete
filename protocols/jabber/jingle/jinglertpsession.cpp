/*WARNING : needs oRTP 0.13.1*/
#include "jinglertpsession.h"

#include <ortp/payloadtype.h>

#include <QUdpSocket>
#include <QImage>
#include <QByteArray>
#include <KDebug>
#include <QDomElement>
//#include <>

JingleRtpSession::JingleRtpSession(Direction d)
{
	m_direction = d;
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Creating JingleRtpSession";
	/*
	 * The RtpSession objects represent a RTP session:
	 * once it is configured with local and remote network addresses and a payload type is given,
	 * it let you send and recv a media stream.
	 */
	
	rtpSocket = new QUdpSocket(this); //Should it really be created and connected here ???
	if (d == In)
		connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
	else if (d == Out)
		connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
	
	rtcpSocket = new QUdpSocket(this);
	connect(rtcpSocket, SIGNAL(readyRead()), this, SLOT(rtcpDataReady())); // FIXME:Not sure I must do this, oRTP will manage that, I don't care about this signal.
	m_rtpSession = rtp_session_new(m_direction == In ? RTP_SESSION_RECVONLY : RTP_SESSION_SENDONLY);
	rtp_session_set_connected_mode(m_rtpSession, true);
	payloadID = -1;
	payloadName = "";
	receivingTS = 0;
	sendingTS = 0;
	rtp_session_set_scheduling_mode(m_rtpSession,1);
	rtp_session_set_blocking_mode(m_rtpSession,1);
	//rtp_session_set_recv_buf_size(m_rtpSession, 80); // ?????????
}

JingleRtpSession::~JingleRtpSession()
{
	rtp_session_bye(m_rtpSession, "Ended");
	rtp_session_destroy(m_rtpSession);
}

void JingleRtpSession::connectToHost(const QString& address, int rtpPort, int rtcpPort)
{
	rtpSocket->connectToHost(address, rtpPort, m_direction == In ? QIODevice::ReadOnly : QIODevice::WriteOnly);
	
	rtcpSocket->connectToHost(address, rtcpPort == 0 ? rtpPort + 1 : rtcpPort, QIODevice::ReadWrite);

	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}

void JingleRtpSession::setRtpSocket(QAbstractSocket* socket, int rtcpPort)
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << (socket->isValid() ? "Socket ready" : "Socket not ready");
	delete rtpSocket;
	rtpSocket = (QUdpSocket*) socket;
	
	if (m_direction == In)
	{
		connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
		rtcpSocket->bind(rtcpPort == 0 ? rtpSocket->localPort() + 1 : rtcpPort);
		kDebug() << "RTCP socket bound to" << rtcpSocket->localPort();
		//bind --> socket already bound.
	}
	else if (m_direction == Out)
	{
		connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
		rtcpSocket->connectToHost(rtpSocket->peerAddress(), rtcpPort == 0 ? rtpSocket->peerPort() + 1 : rtcpPort, QIODevice::ReadWrite);
	}
	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}

void JingleRtpSession::bind(int rtpPort, int rtcpPort)
{
	rtpSocket->bind(rtpPort);

	rtcpSocket->bind(rtcpPort == 0 ? rtpPort + 1 : rtcpPort);
}

void JingleRtpSession::send(const QByteArray& data, int ts) //TODO:There should be overloaded methods to support other data type (QString, const *char).
{
	//qDebug() << "JingleRtpSession::send ts =" << ts;
	
	//for (int i = 0; i < data.count(); i++)
	//	printf("'%c' ", data[i]);
	//printf("\n");
	//fflush(stdout);
	
	//if (payloadID == -1)
	//	return;
	
	mblk_t *packet = rtp_session_create_packet_with_data(m_rtpSession, (uint8_t*)data.data(), data.size(), /*freefn*/ NULL); //the free function is managed by the bytesWritten signal
	
	int size = rtp_session_sendm_with_ts(m_rtpSession, packet, ts == -1 ? sendingTS : ts);
	
	//qDebug() << "Bytes sent :" << size;

	sendingTS += payloadTS;
}

void JingleRtpSession::rtpDataReady()
{
	//qDebug() << "JingleRtpSession::rtpDataReady";
	
	//kDebug() << "receivingTS =" << receivingTS;

	mblk_t *packet;
	while ((packet = rtp_session_recvm_with_ts(m_rtpSession, receivingTS)) == NULL)
	{
		//kDebug() << "Packet is Null, retrying.";
		receivingTS += payloadTS; //FIXME:What is the increment ? It depends on the payload.
	}
	//data is : packet->b_cont->b_rptr
	//of length : len=packet->b_cont->b_wptr - packet->b_cont->b_rptr;
	QByteArray data((char*) packet->b_cont->b_rptr, packet->b_cont->b_wptr - packet->b_cont->b_rptr);
	//kDebug(KDE_DEFAULT_DEBUG_AREA) << "Received (" << packet->b_cont->b_wptr - packet->b_cont->b_rptr << "bytes) : ";
	//for (int i = 0; i < data.count(); i++)
	//	printf("'%1x' ", data.at(i));
	//printf("\n");
	//fflush(stdout);
	
	// Seems we should empty the socket...
	QByteArray buf;
	buf.resize(rtpSocket->pendingDatagramSize());
	rtpSocket->readDatagram(buf.data(), rtpSocket->pendingDatagramSize());
	
	emit readyRead(data);
}

void JingleRtpSession::rtcpDataReady()
{

}

void JingleRtpSession::setPayload(const QDomElement& payload)
{
	Q_UNUSED(payload)
	// Parse SDP string here and store data.
	//payloadTS must be set here.
	payloadName = "PCMA";
	payloadID = 8;
	payloadTS = 168; // For testing, data sent each 168 ms for PCMA TODO:Change that !!!
	RtpProfile *profile = rtp_profile_new(payloadName.toAscii());
	rtp_profile_set_payload(profile, 8, &payload_type_pcma8000);
	rtp_session_set_profile(m_rtpSession, profile);
	rtp_session_set_payload_type(m_rtpSession, 8);
}

void JingleRtpSession::slotBytesWritten(qint64 size)
{
	//kDebug() << size << "bytes written";
	if (state != SendingData)
		return;
	emit dataSent();
}