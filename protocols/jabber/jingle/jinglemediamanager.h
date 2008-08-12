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

/*
 * TODO:Move me in my own source file.
 */
class AlsaALaw;
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
	 * @brief sets the payload type for this media session
	 * @param payload the XML payload-type tag with it's children so we have all informations on the stream
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

	void setMediaPlugin(AlsaALaw *plugin);

	void start();

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
	AlsaALaw *plugin;
	unsigned int ts;
	unsigned int tsValue;
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
	 * @brief starts streaming for the @payloadType payload type if not already started
	 * @param payload-type payload type to start the stream with
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
	 * @param payload the XML payload-type tag with it's children so we have all informations on the stream
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
	AlsaALaw *alaw;

};

#include <alsa/asoundlib.h>

#include <QObject>
#include <QSocketNotifier>
#include <QDebug>
#include <KDebug>

class AlsaALaw : public QObject
{
	Q_OBJECT
public:
	AlsaALaw()
	{
		ready = false;
		int err;
		snd_pcm_hw_params_t *hwParams;

		if ((err = snd_pcm_open(&captureHandle, "default", SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0)
		{
			kDebug() << "cannot open audio device default";
			return;
		}

		if ((err = snd_pcm_hw_params_malloc(&hwParams)) < 0)
		{
			kDebug() << "cannot allocate hardware parameter structure" ;
			return;
		}

		if ((err = snd_pcm_hw_params_any(captureHandle, hwParams)) < 0)
		{
			kDebug() << "cannot initialize hardware parameter structure" ;
			return;
		}

		if ((err = snd_pcm_hw_params_set_access(captureHandle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
		{
			kDebug() << "cannot set access type" ;
			return;
		}

		if ((err = snd_pcm_hw_params_set_format(captureHandle, hwParams, SND_PCM_FORMAT_A_LAW)) < 0)	//A-Law format can be sent directly with oRTP (format supported)
		{
			kDebug() << "cannot set sample format" ;
			return;
		}

		samplingRate = 64000;
		if ((err = snd_pcm_hw_params_set_rate_near(captureHandle, hwParams, &samplingRate, 0)) < 0)
		{
			kDebug() << "cannot set sample rate" ;
			return;
		}
		kDebug() << "Set rate" << samplingRate;

		if ((err = snd_pcm_hw_params_set_channels(captureHandle, hwParams, 1)) < 0) //Only 1 channel for ALaw RTP
		{
			kDebug() << "cannot set channel count" ;
			return;
		}
		
		pTime = 20000;
		if ((err = snd_pcm_hw_params_set_period_time_near(captureHandle, hwParams, &pTime, 0)) < 0) // Set 20000 µs (20ms) before letting us know that there is data.
		{
			kDebug() << "Unable to set period time! Error" << err;
			return;
		}
		kDebug() << "Period time =" << pTime;

		if ((err = snd_pcm_hw_params(captureHandle, hwParams)) < 0)
		{
			kDebug() << "cannot set parameters" ;
			return;
		}

		snd_pcm_hw_params_free(hwParams);

		if ((err = snd_pcm_prepare(captureHandle)) < 0)
		{
			kDebug() << "cannot prepare audio interface for use" ;
			return;
		}
		ready = true;
	}
	~AlsaALaw()
	{
		//TODO:Close alsa stuff !!!
		snd_pcm_close (captureHandle);
		close(notifier->socket());
		
		delete notifier;
		delete timer;
	}
	
	void start()
	{
		kDebug() << "start()";
		if (!ready)
		{
			kDebug() << "Not Ready, sending 44 bytes of zeros every 168 second.";
			kDebug() << "This could probably be caused by an innacessible audio device or simply because there is no audio device.";
			
			timer = new QTimer(this);
			timer->setInterval(168);
			connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));
			timer->start();
			return;
		}
		int count = snd_pcm_poll_descriptors_count(captureHandle);
		if (count <= 0)
		{
			kDebug() << "No poll fd... WEIRD!";
			return;
		}

		pollfd *ufds;
		ufds = new pollfd[count];
		int err = snd_pcm_poll_descriptors(captureHandle, ufds, count);
		if (err < 0)
		{
			kDebug() << "Error retrieving fd.";
			return;
		}
		
		kDebug() << "Retreived" << count << "file descriptors.";

		int fd = ufds[0].fd;

		notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
		if (!notifier->isEnabled())
			notifier->setEnabled(true);
		
		snd_pcm_start(captureHandle);
		
		connect(notifier, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
			
	}
	/**
	 * @return period time in milisecond
	 */
	unsigned int periodTime() const
	{
		return pTime / 1000;
	}
	/**
	 * @return sampling rate.
	 */
	unsigned int sRate() const
	{
		return samplingRate;
	}

	QByteArray data()
	{
		QByteArray data = buf;
		buf.clear();
		return data;
	}
	
	unsigned int timeStamp()
	{
		unsigned int wps = sRate()/8;	// Bytes per second
		kDebug() << "Bytes per second =" << wps;
		unsigned int wpms = wps/1000;		// Bytes per milisecond
		kDebug() << "Bytes per millisecond =" << wpms;
		unsigned int ts = wpms * periodTime();		// Time stamp
		kDebug() << "Time stamp =" << ts;
		return ts;
	}

public slots:
	void slotActivated(int socket)
	{
		//kDebug() << "Data arrived. (Alsa told me !)";
		size_t size;
		//while (EAGAIN == size)
		{
			QByteArray tmpBuf;
			tmpBuf.resize(1024);
			size = read(socket, tmpBuf.data(), 1024);
			tmpBuf.resize(size);
			buf.append(tmpBuf);
		}
		emit readyRead();
		/*kDebug() << "Read" << buf.count() << "bytes";
		for(int i = 0; i < buf.count(); i++)
		{
			printf("0x%02x ", (int) buf.at(i));
		}*/
	}
	void timerTimeOut()
	{
		// With a period time of 21333 µs and the A-Law format, we always have 44 bytes.
		// Here, this is 44 bytes which are all zeros.
		buf.fill('\0', 44);
		emit readyRead();
	}
private:
	snd_pcm_t *captureHandle;
	QSocketNotifier *notifier;
	bool ready;
	QByteArray buf;
	unsigned int pTime;
	unsigned int samplingRate;
	QTimer *timer;
signals:
	void readyRead();
};

#endif //JABBER_MEDIA_MANAGER
