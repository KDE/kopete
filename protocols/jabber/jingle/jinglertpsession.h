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
	 * Creates a new RTP session with direction dir.
	 */
	JingleRtpSession();

	/*
	 * Destroys the RTP sessions and frees all allocated memory.
	 */
	~JingleRtpSession();

	/*
	 * Prepares and set socket in oRTP.
	 */
	void setRtpSockets();
	
	/*
	 * Sets the payload type used for this session.
	 * The argument is the payload type in a payload-type XML tag.
	 */
	void setPayload(const QDomElement& payload);

	void setMediaSession(MediaSession *mSession);

	/* 
	 * Returns size bytes of media data from the internal buffer.
	 * If size = -1, the whole buffer is returned.
	 */
	QByteArray takeMediaData();
	
	/* 
	 * Returns size bytes of RTP data from the internal buffer.
	 * If size = -1, the whole buffer is returned.
	 */
	QByteArray takeRtpData();

	/*
	 * Unpack RTP data.
	 * mediaDataReady() is emitted when it's finished.
	 */
	void unpackRtpData(const QByteArray& data);

	/*
	 * Pack media data.
	 * rtpDataReady() is emitted when it's finished.
	 */
	void packMediaData(const QByteArray& data);

private slots:
	/*
	 * Called when rtp data is ready to be read from the socket, we then wait
	 * for the media data to be extracted from the RTP packet by oRTP.
	 */
	//void rtpDataReady();
	//void rtcpDataReady(); // Maybe not used.
	void slotA();
	void slotB();

signals:
	/*
	 * Emitted when rtp data has been unpacked and is ready to be read.
	 * This is always emitted after calling unpackRtpData().
	 */
	void mediaDataReady();

	/*
	 * Emitted when media data has been packed and is ready to be read.
	 * This is alway emitted after calling packMediaData().
	 */
	void rtpDataReady();

	void dataSent();
	void readyRead(const QByteArray&);

private:
	QUdpSocket *rtpInSocket;
	QUdpSocket *rtpOutSocket;

	QUdpSocket *readSocket;
	QUdpSocket *writeSocket;
	
	QUdpSocket *rtcpInSocket;
	QUdpSocket *rtcpOutSocket;

	RtpSession *m_rtpInSession;
	RtpSession *m_rtpOutSession;

	int payloadID;
	QString payloadName;
	enum State {SendingData = 0} state;
	int bufSize;
	QByteArray inData;
	QByteArray mediaBuffer;
	MediaSession *m_mediaSession;
	int inPort, outPort;
};

#endif
