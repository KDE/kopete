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

#include <QDebug>

#include "alsaio.h"

AlsaIO::AlsaIO(StreamType t, Format f)
: m_type(t)
{
	ready = false;
	written = 0;
	notifier = 0;
	int err;

	if ((err = snd_pcm_open(&handle, "default", m_type == Capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0)
	{
		qDebug() << "cannot open audio device default";
		return;
	}

	if ((err = snd_pcm_hw_params_malloc(&hwParams)) < 0)
	{
		qDebug() << "cannot allocate hardware parameter structure" ;
		return;
	}

	if ((err = snd_pcm_hw_params_any(handle, hwParams)) < 0)
	{
		qDebug() << "cannot initialize hardware parameter structure" ;
		return;
	}

	if ((err = snd_pcm_hw_params_set_access(handle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		qDebug() << "cannot set access type" ;
		return;
	}

	snd_pcm_format_t fmt = static_cast<snd_pcm_format_t>(f);
	if ((err = snd_pcm_hw_params_set_format(handle, hwParams, fmt)) < 0)
	{
		qDebug() << "cannot set sample format";
		qDebug() << "Setting first format...";
		if ((err = snd_pcm_hw_params_set_format_first(handle, hwParams, &fmt)) < 0)
		{
			qDebug() << "cannot set first sample format !";
			return;
		}
	}
	
	snd_pcm_hw_params_get_format(hwParams, &fmt);
	
	m_format = static_cast<Format>(fmt);

	switch(m_format)
	{
	case Signed8:
		qDebug() << "Signed 8";
		break;
	case Unsigned8:
		qDebug() << "Unsigned 8";
		break;
	case Signed16Le:
		qDebug() << "Signed 16 LE";
		break;
	case Signed16Be:
		qDebug() << "Signed 16 BE";
		break;
	case Unsigned16Le:
		qDebug() << "Unsigned 16 LE";
		break;
	case Unsigned16Be:
		qDebug() << "Unsigned 16 BE";
		break;
	}

	unsigned int p = 20000;
	if ((err = snd_pcm_hw_params_set_period_time_near(handle, hwParams, &p, 0)) < 0)
	{
		qDebug() << "cannot set period time near to 20 ms";
		//return;
	}

	samplingRate = 8000;
	if ((err = snd_pcm_hw_params_set_rate_near(handle, hwParams, &samplingRate, 0)) < 0)
	{
		qDebug() << "cannot set sample rate";
		return;
	}
	
	if ((err = snd_pcm_hw_params_set_channels(handle, hwParams, 1)) < 0) //Only 1 channel for ALaw RTP (see RFC specification)
	{
		qDebug() << "cannot set channel 1";
		return;
	}

	if ((err = snd_pcm_hw_params(handle, hwParams)) < 0)
	{
		qDebug() << "cannot set parameters";
		return;
	}
	
	snd_pcm_hw_params_get_period_size(hwParams, &pSize, 0);
	qDebug() << "Period size =" << pSize;
	snd_pcm_hw_params_get_period_time(hwParams, &pTime, 0);
	qDebug() << "Period time =" << pTime;
	qDebug() << "Sampling rate =" << samplingRate;

	pSizeBytes = snd_pcm_frames_to_bytes(handle, pSize);
	
	ready = true;
}

AlsaIO::~AlsaIO()
{
	if (notifier)
	{
		close(notifier->socket());
		delete notifier;
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	
	qDebug() << "DESTROYED";
}

AlsaIO::StreamType AlsaIO::type() const
{
	return m_type;
}

bool AlsaIO::start()
{
	qDebug() << "start()";
	if (ready)
	{
		snd_pcm_hw_params_free(hwParams);

		if (snd_pcm_prepare(handle) < 0)
		{
			qDebug() << "cannot prepare audio interface for use" ;
			ready = false;
		}
	}

	if (!ready)
	{
		if (m_type == Capture)
		{
			return false;
		}
		else if (m_type == Playback)
		{
			qDebug() << "Device is not ready, we will simply drop packets. --> NO PLAYBACK";
			return false;
		}
	}
	
	fdCount = snd_pcm_poll_descriptors_count(handle);
	
	if (fdCount <= 0)
	{
		qDebug() << "No poll fd... WEIRD!";
		return false;
	}

	ufds = new pollfd[fdCount];
	int err = snd_pcm_poll_descriptors(handle, ufds, fdCount);
	if (err < 0)
	{
		qDebug() << "Error retrieving fd.";
		return false;
	}
	
	qDebug() << "Retreived" << fdCount << "file descriptors.";

	if (m_type == Capture)
	{
		qDebug() << "Setting up Capture";
		//Always use the first pollfd
		notifier = new QSocketNotifier(ufds[0].fd, QSocketNotifier::Read, this);
		notifier->setEnabled(true);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
		snd_pcm_start(handle);
	}
	else if (m_type == Playback)
	{
		qDebug() << "Setting up Playback";
		//Always use the first pollfd
		QSocketNotifier::Type type;
		switch (ufds[0].events & (POLLIN | POLLPRI | POLLOUT))
		{
		case POLLIN:
			qDebug() << "QSocketNotifier::Read";
			type = QSocketNotifier::Read;
			break;
		case POLLOUT:
			qDebug() << "QSocketNotifier::Write";
			type = QSocketNotifier::Write;
			break;
		default:
			qDebug() << "Unsupported poll events";
			return false;
		}

		notifier = new QSocketNotifier(ufds[0].fd, type); //FIXME:Taking 100% of CPU time...
		notifier->setEnabled(true);
		connect(notifier, SIGNAL(activated(int)), this, SLOT(checkAlsaPoll(int)));
		qDebug() << "Time stamp =" << timeStamp();
	}

	return true;
}

void AlsaIO::write(const QByteArray& data)
{
	if (!ready || m_type != Playback)
	{
		qDebug() << "Packet dropped";
		return; // Must delete the data before ?
	}
	buf = data;
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
	//QByteArray data = buf;
	//qDebug() << "data.size() =" << data.size();
	//buf.clear();
	return buf;
}

unsigned int AlsaIO::timeStamp()
{
	unsigned int wps = sRate()/8;	// Bytes per second
	qDebug() << "Bytes per second =" << wps;
	unsigned int wpms = wps/1000;		// Bytes per milisecond
	qDebug() << "Bytes per millisecond =" << wpms;
	unsigned int ts = wpms * periodTime();		// Time stamp
	qDebug() << "Time stamp =" << ts;
	return ts;
}

void AlsaIO::slotActivated(int) //Rename this slot
{
	//qDebug() << "Data arrived. (Alsa told me !)";
	size_t size;
	//buf.clear();
	
	buf.resize(pSizeBytes);
	size = snd_pcm_readi(handle, buf.data(), pSize);
	buf.resize(snd_pcm_frames_to_bytes(handle, size));
	
	emit readyRead();
}

void AlsaIO::checkAlsaPoll(int)
{
	//qDebug() << "started since" << (times * periodTime()) / 1000 << "sec.";
	//times++;
	unsigned short revents;
	
	poll(ufds, fdCount, -1);
	snd_pcm_poll_descriptors_revents(handle, ufds, fdCount, &revents);

	if (revents & POLLOUT)
		writeData();
	else
		qDebug() << "poll returned no event (" << revents << ", " << ufds[0].revents << ") ?";	
}

void AlsaIO::writeData()
{
	if (buf.size() < pSizeBytes)
	{
		//qDebug() << "No enough Data in the buffer.";
		//We don't write data now as it's an empty buffer and it would make weird noises only.
		return;
	}

	int size = snd_pcm_writei(handle, buf.data(), buf.size());

	emit bytesWritten();
	
	buf.clear();

	if (size < 0)
	{
		if (size == -EPIPE)
		{
			qDebug() << "buffer underrun";
			prepare();
			return;
		}
		qDebug() << "An error occured while writing data on the device. (" << snd_strerror(size) << ")";
	}
}

bool AlsaIO::prepare()
{
	int err;

	qDebug() << "prepare()";
	if ((err = snd_pcm_prepare(handle)) < 0)
	{
		qDebug() << "cannot prepare audio interface for use" ;
		return false;
	}

	return true;
}

void AlsaIO::setFormat(Format f)
{
	snd_pcm_format_t format = static_cast<snd_pcm_format_t>(f);

	if (snd_pcm_hw_params_set_format(handle, hwParams, format) < 0)
	{
		qDebug() << "cannot set sample format";
		return;
	}

	m_format = f;
}

int AlsaIO::frameSizeBytes()
{
	return pSizeBytes;
}
