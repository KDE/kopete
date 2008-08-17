#ifndef ALSA_IO

#include <alsa/asoundlib.h>

#include <QObject>
#include <QSocketNotifier>
#include <QDebug>
#include <KDebug>
#include <QTimer>

class AlsaIO : public QObject
{
	Q_OBJECT
public:
	enum StreamType {
		Capture = 0,
		Playback
	};
	
	AlsaIO(StreamType t);
	~AlsaIO();

	StreamType type() const;

	void start();
	
	void write(const QByteArray& data);

	bool isReady();
	
	/**
	 * @return period time in milisecond
	 */
	unsigned int periodTime() const;

	/**
	 * @return sampling rate.
	 */
	unsigned int sRate() const;

	QByteArray data();
	
	unsigned int timeStamp();
	void writeData();
	bool prepare();

public slots:
	void slotActivated(int socket);
	void timerTimeOut();
	void checkAlsaPoll(int);

signals:
	void readyRead();

private:
	StreamType m_type;
	snd_pcm_t *handle;
	QSocketNotifier *notifier;
	bool ready;
	QByteArray buf;
	unsigned int pTime;
	snd_pcm_uframes_t pSize;
	unsigned int samplingRate;
	QTimer *timer;
	int fdCount;
	struct pollfd *ufds;
	unsigned int written;
};

#endif //ALSA_IO
