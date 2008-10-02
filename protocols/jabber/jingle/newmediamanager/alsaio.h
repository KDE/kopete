#ifndef ALSA_IO
#define ALSA_IO

#include <alsa/asoundlib.h>

#include <QObject>
#include <QSocketNotifier>
#include <QFile>
#include <QTimer>

class AlsaIO : public QObject
{
	Q_OBJECT
public:
	/** PCM sample format */
	enum Format {
		/** Unknown */
		Unknown = -1,
		/** Signed 8 bit */
		Signed8 = 0,
		/** Unsigned 8 bit */
		Unsigned8 = SND_PCM_FORMAT_U8,
		/** Signed 16 bit Little Endian */
		Signed16Le = SND_PCM_FORMAT_S16_LE,
		/** Signed 16 bit Big Endian */
		Signed16Be = SND_PCM_FORMAT_S16_BE,
		/** Unsigned 16 bit Little Endian */
		Unsigned16Le = SND_PCM_FORMAT_U16_LE,
		/** Unsigned 16 bit Big Endian */
		Unsigned16Be = SND_PCM_FORMAT_U16_BE,
		/** Mu-Law */
		MuLaw = SND_PCM_FORMAT_MU_LAW,
		/** A-Law */
		ALaw = SND_PCM_FORMAT_A_LAW
	};

	enum StreamType {
		Capture = 0,
		Playback
	};
	
	AlsaIO(StreamType t, Format f);
	~AlsaIO();

	StreamType type() const;

	bool start();
	
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

	/**
	 * @return used format in snd_pcm_format_t (this should change)
	 */
	Format format() const {return m_format;}
	void setFormat(Format f);

	QByteArray data();
	
	unsigned int timeStamp();
	void writeData();
	bool prepare();
	int frameSizeBytes();

public slots:
	void slotActivated(int socket);
	void checkAlsaPoll(int);

signals:
	void readyRead();
	void bytesWritten();

private:
	StreamType m_type;
	snd_pcm_t *handle;
	QSocketNotifier *notifier;
	bool ready;
	QByteArray buf;
	unsigned int pTime;
	snd_pcm_uframes_t pSize;
	unsigned int samplingRate;
	int fdCount;
	struct pollfd *ufds;
	unsigned int written;
	void stop();
	QFile *testFile;
	Format m_format;
	snd_pcm_hw_params_t *hwParams;
	int pSizeBytes;
	QByteArray tmpBuf;
};

#endif //ALSA_IO
