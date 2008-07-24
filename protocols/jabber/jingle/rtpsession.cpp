/*WARNING : needs oRTP 0.13.1*/
#include "rtpsession.h"

#include <ortp/payloadtype.h>

#include <QUdpSocket>
#include <QImage>
#include <QByteArray>
#include <KDebug>
//#include <>

JingleRtpSession::JingleRtpSession()
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Creating JingleRtpSession";
	/*
	 * The RtpSession objects represent a RTP session:
	 * once it is configured with local and remote network addresses and a payload type is given,
	 * it let you send and recv a media stream.
	 */
	rtpSocket = new QUdpSocket(); //Should it really be created and connected here ???
	connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
	connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
	rtcpSocket = new QUdpSocket();
	connect(rtcpSocket, SIGNAL(readyRead()), this, SLOT(rtcpDataReady())); // FIXME:Not sure I must do this, oRTP will manage that, I don't care about this signal.
	m_rtpSession = rtp_session_new(RTP_SESSION_SENDRECV);
	payloadID = -1;
	payloadName = "";
	receivingTS = 0;
	sendingTS = 0;
	rtp_session_set_recv_buf_size(m_rtpSession, 80); // ?????????
}

JingleRtpSession::~JingleRtpSession()
{
	rtp_session_destroy(m_rtpSession);
}

void JingleRtpSession::connectToHost(const QString& address, int rtpPort, int rtcpPort)
{
	rtpSocket->connectToHost(address, rtpPort, QIODevice::ReadWrite);
	
	rtcpSocket->connectToHost(address, rtcpPort == 0 ? rtpPort + 1 : rtcpPort, QIODevice::ReadWrite);

	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}

void JingleRtpSession::setRtpSocket(QAbstractSocket* socket, int rtcpPort)
{
	kDebug(KDE_DEFAULT_DEBUG_AREA) << (socket->isValid() ? "Socket ready" : "Socket not ready");
	delete rtpSocket;
	rtpSocket = (QUdpSocket*) socket;
	connect(rtpSocket, SIGNAL(readyRead()), this, SLOT(rtpDataReady()));
	connect(rtpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(slotBytesWritten(qint64)));
	
	rtcpSocket->connectToHost(rtpSocket->peerAddress(), rtcpPort == 0 ? rtpSocket->localPort() + 1 : rtcpPort, QIODevice::ReadWrite);
	rtp_session_set_sockets(m_rtpSession, rtpSocket->socketDescriptor(), rtcpSocket->socketDescriptor());
}

void JingleRtpSession::bind(int rtpPort, int rtcpPort)
{
	rtpSocket->bind(rtpPort);

	rtcpSocket->bind(rtcpPort == 0 ? rtpPort + 1 : rtcpPort);
}

void JingleRtpSession::send(const QByteArray& data) //TODO:There should be overloaded methods to support other data type (QString, const *char).
{
	qDebug() << "JingleRtpSession::send (" << data << ")";
	if (payloadID == -1)
		return;
	state = SendingData;
	mblk_t *packet = rtp_session_create_packet_with_data(m_rtpSession, (uint8_t*)data.data(), data.size(), /*freefn*/ NULL); //the free function is managed by the bytesWritten signal
	int size = rtp_session_sendm_with_ts(m_rtpSession, packet, sendingTS);
	qDebug() << "Bytes sent :" << size;

	sendingTS += payloadTS;
}

void JingleRtpSession::rtpDataReady()
{
	qDebug() << "JingleRtpSession::rtpDataReady";
	if (payloadID == -1)
		return;
	/*mblk_t *packet = rtp_session_recvm_with_ts(m_rtpSession, receivingTS);
	//data is : packet->b_cont->b_rptr
	//of length : len=mp->b_cont->b_wptr - mp->b_cont->b_rptr;
	data.data() = (char*) packet->b_cont->b_rptr;*/
	QByteArray data((const char*)rtp_session_get_data(m_rtpSession), 80);
	//char* = unsigned char*
	kDebug(KDE_DEFAULT_DEBUG_AREA) << "Received (80 bytes) : " << data;
	receivingTS += payloadTS; //FIXME:What is the increment ? It depends on the payload.
}

void JingleRtpSession::rtcpDataReady()
{

}

void JingleRtpSession::setPayload(const QString&)
{
	// Parse SDP string here and store data.
	//payloadTS must be set here.
	payloadName = "theora";
	payloadID = 96;
	payloadTS = 2000; // For testing, data sent each 2000 ms TODO:Change that !!!
	RtpProfile *profile = rtp_profile_new(payloadName.toAscii());
	rtp_profile_set_payload(profile, 96, &payload_type_theora);
}

void JingleRtpSession::slotBytesWritten(qint64 size)
{
	if (state != SendingData)
		return;
	emit dataSent();
}

void JingleRtpSession::wrapJpegData(char *data, int size)
{
	QImage img;
	img.loadFromData((const uchar*)data, size, "JPEG");
	unsigned char header[8];
	header[0] = 0;
	header[1] = 0;
	header[2] = 0;
	header[3] = 0;
	header[4] = 0;
	header[5] = 0;
	header[6] = img.width() / 8;
	header[7] = img.height() / 8;
	
}
