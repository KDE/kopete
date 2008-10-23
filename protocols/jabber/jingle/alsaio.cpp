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
#include <QStringList>

#include <KDebug>

#include "alsaio.h"

// taken from netinterface_unix (changed the split to KeepEmptyParts)
static QStringList read_proc_as_lines(const char *procfile)
{
	QStringList out;

	FILE *f = fopen(procfile, "r");
	if(!f)
		return out;

	QByteArray buf;
	while(!feof(f))
	{
		// max read on a proc is 4K
		QByteArray block(4096, 0);
		int ret = fread(block.data(), 1, block.size(), f);
		if(ret <= 0)
			break;
		block.resize(ret);
		buf += block;
	}
	fclose(f);

	QString str = QString::fromLocal8Bit(buf);
	out = str.split('\n', QString::KeepEmptyParts);
	return out;
}

QList<Item> getAlsaItems()
{
#ifdef Q_OS_LINUX
	QList<Item> out;

	QList<AlsaItem> items;
	QStringList devices_lines = read_proc_as_lines("/proc/asound/devices");
	foreach(QString line, devices_lines)
	{
		// get the fields we care about
		QString devbracket, devtype;
		int x = line.indexOf(": ");
		if(x == -1)
			continue;
		QString sub = line.mid(x + 2);
		x = sub.indexOf(": ");
		if(x == -1)
			continue;
		devbracket = sub.mid(0, x);
		devtype = sub.mid(x + 2);

		// skip all but playback and capture
		bool input;
		if(devtype == "digital audio playback")
			input = false;
		else if(devtype == "digital audio capture")
			input = true;
		else
			continue;

		// skip what isn't asked for
		/*if(!(type & DIR_INPUT) && input)
			continue;
		if(!(type & DIR_OUTPUT) && !input)
			continue;
*/
		// hack off brackets
		if(devbracket[0] != '[' || devbracket[devbracket.length()-1] != ']')
			continue;
		devbracket = devbracket.mid(1, devbracket.length() - 2);

		QString cardstr, devstr;
		x = devbracket.indexOf('-');
		if(x == -1)
			continue;
		cardstr = devbracket.mid(0, x);
		devstr = devbracket.mid(x + 1);

		AlsaItem ai;
		bool ok;
		ai.card = cardstr.toInt(&ok);
		if(!ok)
			continue;
		ai.dev = devstr.toInt(&ok);
		if(!ok)
			continue;
		ai.input = input;
		ai.name.sprintf("ALSA Card %d, Device %d", ai.card, ai.dev);
		items += ai;
	}

	// try to get the friendly names
	QStringList pcm_lines = read_proc_as_lines("/proc/asound/pcm");
	foreach(QString line, pcm_lines)
	{
		QString devnumbers, devname;
		int x = line.indexOf(": ");
		if(x == -1)
			continue;
		devnumbers = line.mid(0, x);
		devname = line.mid(x + 2);
		x = devname.indexOf(" :");
		if(x != -1)
			devname = devname.mid(0, x);
		else
			devname = devname.trimmed();

		QString cardstr, devstr;
		x = devnumbers.indexOf('-');
		if(x == -1)
			continue;
		cardstr = devnumbers.mid(0, x);
		devstr = devnumbers.mid(x + 1);

		bool ok;
		int cardnum = cardstr.toInt(&ok);
		if(!ok)
			continue;
		int devnum = devstr.toInt(&ok);
		if(!ok)
			continue;

		for(int n = 0; n < items.count(); ++n)
		{
			AlsaItem &ai = items[n];
			if(ai.card == cardnum && ai.dev == devnum)
				ai.name = devname;
		}
	}

	for(int n = 0; n < items.count(); ++n)
	{
		AlsaItem &ai = items[n];

		// make an item for both hw and plughw
		Item i;
		i.type = Item::Audio;
		if(ai.input)
			i.dir = Item::Input;
		else
			i.dir = Item::Output;
		i.name = ai.name;
		i.driver = "alsa";
		i.id = QString().sprintf("plughw:%d,%d", ai.card, ai.dev);
		out += i;

		i.name = ai.name + " (Direct)";
		i.id = QString().sprintf("hw:%d,%d", ai.card, ai.dev);
		out += i;
	}

	Item outDef, inDef;
	outDef.type = Item::Audio;
	outDef.dir = Item::Output;
	outDef.name = "Default device";
	outDef.id = "default";
	outDef.driver = "alsa";
	
	inDef.type = Item::Audio;
	inDef.dir = Item::Input;
	inDef.name = "Default device";
	inDef.id = "default";
	inDef.driver = "alsa";

	out << outDef << inDef;

	return out;
#else
	// return empty list if non-linux
	return QList<Item>();
#endif
}

AlsaIO::AlsaIO(StreamType t, QString device, Format f)
: m_type(t)
{
	times = 0;
	ready = false;
	written = 0;
	notifier = 0;
	int err;
	//const char *device = (m_type == Capture ? "hw:0,0" : "default");
	printf("device = %s\n", device.toUtf8().data());

	if ((err = snd_pcm_open(&handle, device.toUtf8().data(), m_type == Capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0)
	{
		qDebug() << "cannot open audio device" << device;
		qDebug() << "trying default";
		if ((err = snd_pcm_open(&handle, "default", m_type == Capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0)
		{
			kDebug() << "cannot open audio device default";
			return;
		}
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

	unsigned int p = 20000;
	if ((err = snd_pcm_hw_params_set_period_time_near(handle, hwParams, &p, 0)) < 0)
	{
		qDebug() << "cannot set period time near to 20 ms";
		return;
	}

	samplingRate = 8000;
	if ((err = snd_pcm_hw_params_set_rate_near(handle, hwParams, &samplingRate, 0)) < 0)
	{
		qDebug() << "cannot set sample rate";
		//Don't return now, could work without that.
		//return;
	}
	
	if ((err = snd_pcm_hw_params_set_channels(handle, hwParams, 1)) < 0)
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
	snd_pcm_hw_params_get_rate (hwParams, &samplingRate, 0);
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

	if (ready)
	{
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	}
	
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
		//This is done here so we can modify parameters before starting.
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
			qDebug() << "Device is not ready, no packet will be sent.";
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
		connect(notifier, SIGNAL(activated(int)), this, SLOT(slotReadyRead(int)));
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

		notifier = new QSocketNotifier(ufds[0].fd, type);
		notifier->setEnabled(false); //Will be activated as soon as data comes in
		connect(notifier, SIGNAL(activated(int)), this, SLOT(slotReadyWrite(int)));
		qDebug() << "Time stamp =" << timeStamp();
	}
	kDebug() << "started.";

	return true;
}

void AlsaIO::write(const QByteArray& data)
{
	//kDebug() << "Buffered ! (" << data.size() << "bytes )";
	if (!ready || m_type != Playback)
	{
		qDebug() << "Packet dropped";
		return; // Must delete the data before ?
	}

	buf.append(data);
	if (notifier && !notifier->isEnabled())
	{
		//kDebug() << "Reactivating notifier.";
		notifier->setEnabled(true);
	}
	//kDebug() << "Buffer size is now" << buf.size() << "bytes )";
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

void AlsaIO::slotReadyRead(int)
{
	//qDebug() << "Data arrived. (Alsa told me !)";
	size_t size;
	
	buf.resize(pSizeBytes);
	size = snd_pcm_readi(handle, buf.data(), pSize);
	buf.resize(snd_pcm_frames_to_bytes(handle, size));
	
	emit readyRead();
}

void AlsaIO::slotReadyWrite(int)
{
	//qDebug() << "started since" << (times * periodTime()) / 1000 << "sec.";
	//times++;
	unsigned short revents;
	
	poll(ufds, fdCount, -1);
	snd_pcm_poll_descriptors_revents(handle, ufds, fdCount, &revents);

	if (revents & POLLOUT)
		writeData();
	else
	{
		notifier->setEnabled(false);
		qDebug() << "poll returned no event (" << revents << ", " << ufds[0].revents << ") ?";	
	}

}

void AlsaIO::writeData()
{
	//kDebug() << "Preparing writing ! " << times++;
	if (buf.size() < pSizeBytes)
	{
		//kDebug() << "No enough Data in the buffer. Waiting for more...";
		notifier->setEnabled(false);
		//We don't write data now as it's an empty buffer and it would make weird noises only.
		return;
	}

	int size = snd_pcm_writei(handle, buf.data(), buf.size());

	emit bytesWritten();

	//kDebug() << "Written on alsa device !";
	
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
