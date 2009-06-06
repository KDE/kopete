 /*
  * alsaio.h - An alsa I/O manager (works in Capture or in Playback mode but not both at a time.)
  *
  * Copyright (c) 2008 by Detlev Casanova <detlev.casanova@gmail.com>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */
#ifndef ALSA_IO
#define ALSA_IO

#include <alsa/asoundlib.h>

#include <QObject>
#include <QSocketNotifier>
#include <QFile>
#include <QTimer>

class Item
{
public:
	enum Type
	{
		Audio,
		Video
	};

	enum Direction
	{
		Input,
		Output
	};

	Type type;      // Audio or Video
	Direction dir;  // Input (mic) or Output (speaker)
	QString name;   // friendly name
	QString driver; // e.g. "oss", "alsa"
	QString id;     // e.g. "/dev/dsp", "hw:0,0"
};

QList<Item> getAlsaItems();

class AlsaItem
{
public:
	int card, dev;
	bool input;
	QString name;
};

class AlsaIO : public QObject
{
	Q_OBJECT
	Q_ENUMS(Format)
	Q_ENUMS(StreamType)
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

	// Stream direction
	enum StreamType {
		Capture = 0,
		Playback
	};
	
	/*
	 * create and configure the alsa handle for the stream type t, the device
	 * dev and the sample format f
	 */
	AlsaIO(StreamType t, QString dev, Format f);
	~AlsaIO();

	/*
	 * returns the stream type currently used.
	 */
	StreamType type() const;

	/*
	 * start streaming, this must be called before starting playback too.
	 */
	bool start();
	
	/*
	 * writes raw audio data on the device.
	 * Actually, it fills a temporary buffer and write it on the device
	 * when the device is ready
	 */
	void write(const QByteArray& data);

	/*
	 * check if the device is ready. that means if it is correcly configured
	 * and ready to start streaming.
	 */
	bool isReady();
	
	/**
	 * @return period time in milisecond
	 */
	unsigned int periodTime() const;

	/**
	 * @return sampling rate
	 */
	unsigned int sRate() const;

	/**
	 * @return used format
	 */
	Format format() const {return m_format;}
	void setFormat(Format f);
	
	/*
	 * returns the data currently available.
	 * each time readyRead() is emitted, those data are
	 * dropped (if you have a reference on the data, it won't be dropped)
	 * and the next sample is available.
	 */
	QByteArray data();
	
	unsigned int timeStamp();
	void writeData();
	int frameSizeBytes();

public slots:
	void slotReadyRead(int socket);
	void slotReadyWrite(int socket);

signals:
	void readyRead();
	void bytesWritten();

private:
	StreamType m_type;
	Format m_format;
	
	QSocketNotifier *notifier;
	QByteArray buf;
	QByteArray tmpBuf;

	unsigned int pTime;
	unsigned int samplingRate;
	unsigned int written;
	int fdCount;
	int pSizeBytes;
	int times;
	bool ready;
	bool bufferizing;
	
	snd_pcm_uframes_t pSize;
	snd_pcm_hw_params_t *hwParams;
	snd_pcm_t *handle;
	struct pollfd *ufds;
	
	bool prepare();
	void stop();
};

#endif //ALSA_IO
