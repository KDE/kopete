/*
 * This class is a media manager.
 * It manages audio and video input using Phonon in order to _send_ media data.
 * This class will also encode data in the asked format.
 * 
 * Currently, I think of having methods like getVideo(payloadType, size).
 * The problem will be to use different multimedia devices (The user would like to use
 * the first webcam (/dev/video0) for his father and the second one (/dev/video1) for
 * his brother.) That could be resolved by a session system...
 *
 * JingleMediaSession *createNewSession(payload-type,...);
 * A pointer to this session will be kept in the JingleMediaManager which will write on
 * sessions when data from the system is ready for them.
 *
 * Also, this session could be used (as in a Jingle content, both streams
 * IN and OUT are the same payload) to encode incoming video.
 * (That would delete the JingleDecoder)
 *
 * I had an idea here but I don't remember it ^^
 *
 * FIXME : Phonon does not support neither audio nor video input yet.
 * This class will manage multimedia devices and give data to RtpSession's objects.
 */
#ifndef JINGLE_MEDIA_MANAGER
#define JINGLE_MEDIA_MANAGER

#include <QObject>
#include <QDomElement>
#include <QList>
#include <QTimer>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/audiointerface.h>
#include <solid/video.h>

#include <errno.h>

/*
 * TODO:Move me in my own source file.
 */
class AlsaIO;
class JingleMediaManager;
class JingleMediaSession : public QObject
{
	Q_OBJECT
public:
	JingleMediaSession(JingleMediaManager *parent);
	~JingleMediaSession();

	enum Type {
		Audio = 0,
		Video,
		NotValid
	} m_type;

	/**
	 * @brief sets the payload m_type for this media session
	 * @param payload the XML payload-m_type tag with it's children so we have all informations on the stream
	 */
	void setPayloadType(const QDomElement& payload);

	/**
	 * @brief sets the output device to use to show remote data
	 * @param the device to used as one of the list returned by JingleMediaManager::[video|audio]Devices()
	 */
	void setOutputDevice(Solid::Device& device);
	
	/**
	 * @brief sets the input device to use to take data from
	 * @param the device to used as one of the list returned by JingleMediaManager::[video|audio]Devices()
	 */
	void setInputDevice(Solid::Device& device);

	void setCaptureMediaPlugin(AlsaIO *plugin);

	void setPlaybackMediaPlugin(AlsaIO *plugin);

	void start();

	void playData(const QByteArray& data);

	QByteArray data();

public slots:
	void slotInReadyRead();

signals:
	/**
	 * @brief emitted when new data is coming from outside on this session
	 */
	void incomingData();
	void readyRead(int);
private:
	QDomElement m_payload;
	JingleMediaManager *m_mediaManager;
	Solid::AudioInterface *audioInputDevice;
	Solid::AudioInterface *audioOutputDevice;
	AlsaIO *playbackPlugin;
	AlsaIO *capturePlugin;
	unsigned int ts; // Current timestamp.
	unsigned int tsValue; // Increment time stamp value
};

class JingleMediaManager : public QObject
{
	Q_OBJECT
public:
	JingleMediaManager();
	~JingleMediaManager();
	
	void findDevices();

	/**
	 * @return a list of the supported payloads by this media manager
	 */
	QList<QDomElement> payloads();

	/**
	 * @brief starts streaming for the @payloadType payload m_type if not already started
	 * @param payload-m_type payload m_type to start the stream with
	 */
	void startStreaming(const QDomElement& payloadType);
	QByteArray data(); //--> FIXME:No, This should be in JingleMediaSession.
	
	/**
	 * @brief returns the list of available video devices
	 * @return a list of available devices.
	 */
	QList<Solid::Device> videoDevices();

	/**
	 * @brief creates a new media session
	 * @param payload the XML payload-m_type tag with it's children so we have all informations on the stream
	 * @param inputDevice the device from which the media data will be taken (which webcam, which audio card) to send to the other peer
	 * @param outputDevice the device which wil be used to display video or play audio received from the othe peer
	 * @return returns a new JingleMediaSession
	 */
	JingleMediaSession* createNewSession(const QDomElement& payload, Solid::Device inputDevice = Solid::Device(), Solid::Device outputDevice = Solid::Device());
	
	/**
	 * @brief switch on the multimedia device (webcam)
	 * @description starts the necessary devices for the existing sessions
	 * this method is deprecated, use startStreaming()
	 * @see startStreaming()
	 */
	void startVideoStreaming();

signals:
	void audioReadyRead(); //--> FIXME:No, This should be in JingleMediaSession.
	void videoReadyRead(); //--> FIXME:No, This should be in JingleMediaSession.

public slots:
	void slotSessionTerminated();
	void slotIncomingData();

private:
	QList<Solid::Device> m_microphones; //FIXME:switch to Phonon audio devices
	QList<Solid::Device> m_audioOutputs; // "
	QList<Solid::Device> m_videoInputs; // "
	QTimer *timer;
	QList<JingleMediaSession*> m_sessions;
	AlsaIO *alawCapture;
	AlsaIO *alawPlayback;

};

#endif //JINGLE_MEDIA_MANAGER
