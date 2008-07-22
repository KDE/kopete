/*
 * This class is a media manager.
 * It manages audio inputs and outputs and video input using Phonon
 * FIXME : Phonon does not support audio input yet.
 * This class will manage multimedia devices and give data to RtpSession's objects.
 */
#ifndef JABBER_MEDIA_MANAGER
#define JABBER_MEDIA_MANAGER

#include <QObject>
#include <QDomElement>
#include <QList>
#include <QTimer>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/audiointerface.h>

class JingleMediaManager : public QObject
{
	Q_OBJECT
public:
	JingleMediaManager();
	~JingleMediaManager();
	int findDevice();
	QList<QDomElement> payloads();

	/*
	 * Starts Audio streaming if not already started.
	 */
	void startAudioStreaming();
	QByteArray data();

signals:
	void audioReadyRead();
	void videoReadyRead();

private:
	Solid::AudioInterface *m_inputDevice;
	Solid::AudioInterface *m_outputDevice;
	QTimer *timer;

};

#endif //JABBER_MEDIA_MANAGER
