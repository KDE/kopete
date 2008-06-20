/*
 * This class is an audio manager.
 * It manages sound inputs and outputs using Phonon
 * FIXME : Phonon does not support audio input yet.
 */

#ifndef JABBER_AUDIO_MANAGER
#define JABBER_AUDIO_MANAGER

#include <QObject>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/audiointerface.h>

class JingleAudioManager : public QObject
{
	Q_OBJECT
public:
	JingleAudioManager();
	~JingleAudioManager();
	int findDevice();
private:
	Solid::AudioInterface *m_inputDevice;
	Solid::AudioInterface *m_outputDevice;

};

#endif //JABBER_AUDIO_MANAGER
