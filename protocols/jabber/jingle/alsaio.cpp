 /*
  * alsaio.cpp - An alsa I/O manager (works in Capture or in Playback mode but not both at a time.)
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
#include <alsa/asoundlib.h>

#include <QObject>
#include <QSocketNotifier>

#include <KDebug>

#include "alsaio.h"

AlsaIO::AlsaIO(StreamType t)
: m_type(t)
{
	//fd = 0;
	ready = false;
	written = 0;
	int err;
	snd_pcm_hw_params_t *hwParams;
	timer = 0;
	notifier = 0;

	if ((err = snd_pcm_open(&handle, "default", m_type == Capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0)
	{
		kDebug() << "cannot open audio device default";
		return;
	}

	if ((err = snd_pcm_hw_params_malloc(&hwParams)) < 0)
	{
		kDebug() << "cannot allocate hardware parameter structure" ;
		return;
	}

	if ((err = snd_pcm_hw_params_any(handle, hwParams)) < 0)
	{
		kDebug() << "cannot initialize hardware parameter structure" ;
		return;
	}

	if ((err = snd_pcm_hw_params_set_access(handle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		kDebug() << "cannot set access type" ;
		return;
	}

	if ((err = snd_pcm_hw_params_set_format(handle, hwParams, SND_PCM_FORMAT_A_LAW)) < 0)	//A-Law format can be sent directly with oRTP (format supported)
	{
		kDebug() << "cannot set sample format" ;
		return;
	}

	samplingRate = 64000;
	if ((err = snd_pcm_hw_params_set_rate_near(handle, hwParams, &samplingRate, 0)) < 0)
	{
		kDebug() << "cannot set sample rate" ;
		return;
	}

	if ((err = snd_pcm_hw_params_set_channels(handle, hwParams, 1)) < 0) //Only 1 channel for ALaw RTP (see RFC specification)
	{
		kDebug() << "cannot set channel 1" ;
		return;
	}

	if ((err = snd_pcm_hw_params(handle, hwParams)) < 0)
	{
		kDebug() << "cannot set parameters" ;
		return;
	}
	
	snd_pcm_hw_params_get_period_size(hwParams, &pSize, 0);
	snd_pcm_hw_params_get_period_time(hwParams, &pTime, 0);
	
	snd_pcm_hw_params_free(hwParams);

	if ((err = snd_pcm_prepare(handle)) < 0)
	{
		kDebug() << "cannot prepare audio interface for use" ;
		return;
	}

	ready = true;
}

AlsaIO::~AlsaIO()
{
	if (notifier)
	{
		close(notifier->socket());
		delete notifier;
	}

	if (timer)
		delete timer;
	
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	
	kDebug() << "DESTROYED";
}

AlsaIO::StreamType AlsaIO::type() const
{
	return m_type;
}

void AlsaIO::start()
{
	kDebug() << "start()";
	if (!ready)
	{
		if (m_type == Capture)
		{
			//FIXME : should I send nothing in this case ????
			//The time stamp is 168 but the period time is 21 millisecond
			kDebug() << "Not Ready, sending 32 bytes of zeros every 21 millisecond.";
			kDebug() << "This could probably be caused by an innacessible audio device or simply because there is no audio device.";
			kDebug() << "No, we do nothing. --> NO CAPTURE";

			//For Testing purpose.
			timer = new QTimer(this);
			timer->setInterval(21);
			connect(timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));
			timer->start();

			return;
		}
		else if (m_type == Playback)
		{
			kDebug() << "Device is not ready, we will simply drop packets. --> NO PLAYBACK";
			
			testFile = new QFile("kopete-test.alaw");
			testFile->open(QIODevice::WriteOnly);

			return;
		}
	}
	fdCount = snd_pcm_poll_descriptors_count(handle);
	
	if (fdCount <= 0)
	{
		kDebug() << "No poll fd... WEIRD!";
		return;
	}

	ufds = new pollfd[fdCount];
	int err = snd_pcm_poll_descriptors(handle, ufds, fdCount);
	if (err < 0)
	{
		kDebug() << "Error retrieving fd.";
		return;
	}
	
	kDebug() << "Retreived" << fdCount << "file descriptors.";

	if (m_type == Capture)
	{
		kDebug() << "Setting up Capture";
		//Always use the first pollfd
		notifier = new QSocketNotifier(ufds[0].fd, QSocketNotifier::Read, this);
		notifier->setEnabled(true);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
		snd_pcm_start(handle);
	}
	else if (m_type == Playback)
	{
		kDebug() << "Setting up Playback";
		//Always use the first pollfd
		QSocketNotifier::Type type;
		switch (ufds[0].events & (POLLIN | POLLPRI | POLLOUT))
		{
		case POLLIN:
			kDebug() << "QSocketNotifier::Read";
			type = QSocketNotifier::Read;
			break;
		case POLLOUT:
			kDebug() << "QSocketNotifier::Write";
			type = QSocketNotifier::Write;
			break;
		default:
			kDebug() << "Unsupported poll events";
			return;
		}

		notifier = new QSocketNotifier(ufds[0].fd, type);
		notifier->setEnabled(true);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(checkAlsaPoll(int)));

		testFile = new QFile("kopete-test.alaw");
		testFile->open(QIODevice::WriteOnly);

	}
	kDebug() << "After start, fdCount =" << fdCount;
}

void AlsaIO::write(const QByteArray& data)
{
	testFile->write(data);

	if (!ready || m_type != Playback)
	{
		kDebug() << "Packet dropped";
		return; // Must delete the data before ?
	}
	
	kDebug() << "Appending received data (" << data.size() << "bytes)";
	buf.append(data);
	kDebug() << "Buffer size is now" << buf.size() << "bytes";
}

bool AlsaIO::isReady()
{
	return ready;
}
/**
 * @return period time in milisecond
 */
unsigned int AlsaIO::periodTime() const
{
	return pTime / 1000;
}
/**
 * @return sampling rate.
 */
unsigned int AlsaIO::sRate() const
{
	return samplingRate;
}

QByteArray AlsaIO::data()
{
	QByteArray data = buf;
	//kDebug() << "data.size() =" << data.size();
	buf.clear();
	return data;
}

unsigned int AlsaIO::timeStamp()
{
	unsigned int wps = sRate()/8;	// Bytes per second
	kDebug() << "Bytes per second =" << wps;
	unsigned int wpms = wps/1000;		// Bytes per milisecond
	kDebug() << "Bytes per millisecond =" << wpms;
	unsigned int ts = wpms * periodTime();		// Time stamp
	kDebug() << "Time stamp =" << ts;
	return ts;
}

void AlsaIO::slotActivated(int) //Rename this slot
{
	//kDebug() << "Data arrived. (Alsa told me !)";
	size_t size;
	QByteArray tmpBuf;
	tmpBuf.resize(pSize);
	size = snd_pcm_readi(handle, tmpBuf.data(), pSize); //Maybe use readi which is more adapted...
	tmpBuf.resize(size);
	buf.append(tmpBuf);

	emit readyRead();
}

void AlsaIO::checkAlsaPoll(int)
{
	unsigned short revents;
	
	poll(ufds, fdCount, -1);
	snd_pcm_poll_descriptors_revents(handle, ufds, fdCount, &revents);

	if (revents & POLLOUT)
		writeData();
	else
		kDebug() << "poll returned no event (" << revents << ", " << ufds[0].revents << ") ?";	
}

void AlsaIO::writeData()
{
	/* We should empty buffer each time this function is called
	 * (each time Alsa tells us we can write on the device)
	 * To do so, we need 2 buffers (one for incoming data (A) and one
	 * for writing on the device (B))
	 * we begin by writing A on the device, while B is being filled,
	 * once this method is called, we can empty A and write B on the device, etc...
	 */
	
	//written += buf.size();
	//kDebug() << "Buffer size =" << buf.size();

	if (buf.size() < pSize)
	{
		//kDebug() << "No enough Data in the buffer.";
		return;
	}

	int size = snd_pcm_writei(handle, buf.data(), pSize); // Could we loose data here ?

	//written += size;

	if (size < 0)
	{
		if (size == -EPIPE)
		{
			kDebug() << "buffer underrun";
			prepare();
			return;
		}
		kDebug() << "An error occured while writing data on the device. (" << snd_strerror(size) << ")";
	}

	kDebug() << "Wrote" << size << "frames on the device.";// (" << written << "bytes since the beginning)";

	buf.clear();
}

bool AlsaIO::prepare()
{
	int err;

	kDebug() << "prepare()";
	if ((err = snd_pcm_prepare(handle)) < 0)
	{
		qDebug() << "cannot prepare audio interface for use" ;
		return false;
	}

	return true;
}

void AlsaIO::timerTimeOut()
{
	// With a period time of 21333 µs and the A-Law format, we always have 32 bytes.
	// Here, this is 32 bytes which are all zeros.
	//buf.fill(static_cast<char>(rand()), 32);
	buf.fill('\0', 32);
	emit readyRead();
}

void AlsaIO::incRef()
{
	ref++;
}

void AlsaIO::decRef()
{
	ref--;
	if (ref == 0)
	{
		stop();
	}
}

void AlsaIO::stop()
{
	if (notifier)
	{
		close(notifier->socket());
		delete notifier;
	}

	if (timer)
		delete timer;
}
