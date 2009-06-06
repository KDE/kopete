#ifndef JINGLE_RTP_SESSION_H
#define JINGLE_RTP_SESSION_H

#include <QObject>

#include <ortp/ortp.h>

class QAbstractSocket;
class QUdpSocket;
class QDomElement;
class MediaSession;

class JingleRtpSession : public QObject
{
	Q_OBJECT
public:
	/*
	 * Directions :
	 * 	In = for data coming in.
	 * 	Out = for data going out.
	 */
	enum Direction {In = 0, Out};
	
	/*
	 * Creates a new RTP session with direction dir.
	 */
	JingleRtpSession(Direction dir);

	/*
	 * Destroys the RTP sessions and frees all allocated memory.
	 */
	~JingleRtpSession();
	
	/*
	 * Sets the socket as RTP socket. This socket must be connected to a host.
	 * It will create the RTCP socket on port rtcpPort if set, RTP socket port + 1 if not.
	 */
	void setRtpSocket(QAbstractSocket*, int rtcpPort = 0);
	
	/*
	 * Create UDP sockets with the given address and ports respectively for RTP and RTCP.
	 * If rtcpPort is not set, rtpPort + 1 will be used.
	 */
	void connectToHost(const QString& address, int rtpPort, int rtcpPort = 0);
	
	/*
	 * Binds sockets to any address on ports rtpPort and rtcpPort for respectively RTP and RTCP.
	 * If rtcpPort is not set, rtpPort + 1 will be used.
	 */
	void bind(int rtpPort, int rtcpPort = 0);

	/*
	 * Sends data to the remote host after wrapping it in a RTP packet.
	 * TODO:There should be overloaded methods to support other data type (QString, const *char).
	 */
	void send(const QByteArray& data);

	/*
	 * Sets the payload type used for this session.
	 * The argument is the payload type in a payload-type XML tag.
	 */
	void setPayload(const QDomElement& payload);

	void setMediaSession(MediaSession *mSession);

private slots:
	/*
	 * Called when rtp data is ready to be read from the socket, we then wait
	 * for the media data to be extracted from the RTP packet by oRTP.
	 */
	void rtpDataReady();
	void rtcpDataReady(); // Maybe not used.

signals:
	void dataSent();
	void readyRead(const QByteArray&);

private:
	QUdpSocket *rtpSocket;
	QUdpSocket *rtcpSocket;
	RtpSession *m_rtpSession;
	int payloadID;
	QString payloadName;
	enum State {SendingData = 0} state;
	Direction m_direction;
	int bufSize;
	QByteArray inData;
	MediaSession *m_mediaSession;
};

#endif
