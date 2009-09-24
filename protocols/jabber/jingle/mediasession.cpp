 /*
  * mediasession.cpp - A helper class to easily manage media data.
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

#include "mediasession.h"
#include "mediamanager.h"
#include "abstractio.h"
#include "alsaio.h"
#include "speexio.h"

#include <KDebug>
#include <QTime>
#include <QTimer>

class MediaSession::Private
{
public :
	AbstractIO *plugin;
	MediaManager *mediaManager;
	QString codecName;
	//QByteArray encodedData;
	QTime startTime;
	int tsValue;
	int ts;
	
	QTimer *coderTimer;

	QByteArray rawData;
	//QByteArray encodedData;
	QByteArray encodedData;
	QByteArray rawDataForAlsa;

	int pluginFSize;
	int alsaFSize;
	int pluginPTime;
};

MediaSession::MediaSession(MediaManager *mm, const QString& codecName)
 : d(new Private())
{
	d->mediaManager = mm;
	d->codecName = codecName;

	if (d->codecName == "speex")
		d->plugin = new SpeexIO();

	d->ts = 0;

	d->pluginPTime = d->plugin->periodTime()/1000; //periodTime is in ms, is there a way to have a microseconds QTimer ?

	qDebug() << "MediaSession : framesize =" << d->plugin->frameSizeBytes() << "bytes";
	
	d->coderTimer = 0;
	
	qDebug() << "Created Media Session for codec" << codecName;
}

MediaSession::~MediaSession()
{
	d->mediaManager->removeSession(this);
	delete d->plugin;
        delete d;
	qDebug() << "Deleted Media Session";
}

void MediaSession::setSamplingRate(int sr)
{
	static_cast<SpeexIO*>(d->plugin)->setSamplingRate(sr);
	d->tsValue = d->plugin->tsValue();
}

void MediaSession::setQuality(int q)
{
	static_cast<SpeexIO*>(d->plugin)->setQuality(q);
}

bool MediaSession::start()
{
	d->startTime = QTime::currentTime();
	bool managerOk = d->mediaManager->addSession(this); //Tell the media manager the session is being started.
	bool pluginOk = d->plugin->start();

	connect((QObject*) d->mediaManager->alsaIn(), SIGNAL(readyRead()), (QObject*) this, SLOT(slotReadyRead()));
	connect((QObject*) d->plugin, SIGNAL(encoded()), (QObject*) this, SLOT(slotEncoded()));
	connect((QObject*) d->plugin, SIGNAL(decoded()), (QObject*) this, SLOT(slotDecoded()));
		
	d->alsaFSize = d->mediaManager->alsaIn()->frameSizeBytes();
	d->pluginFSize = d->plugin->frameSizeBytes();

	return managerOk && pluginOk;
}

void MediaSession::slotReadyRead()
{
	//qDebug() << "Ready read";
	//d->plugin->encode(d->mediaManager->read());
	
	d->rawData.append(d->mediaManager->read());
	
	// RawData should never overpass 3 times the size of alsaFSize.
	if (d->rawData.count() - (3 * d->alsaFSize) > 0)
		d->rawData = resample(d->rawData, 3 * d->alsaFSize);

	if (!d->coderTimer)
	{
		d->coderTimer = new QTimer();
		d->coderTimer->setInterval(d->pluginPTime);
		connect(d->coderTimer, SIGNAL(timeout()), SLOT(pluginCompress()));
		d->coderTimer->start();
	}
}

void MediaSession::write(const QByteArray& sData)
{
	qDebug() << "Received" << sData.count() << "bytes.";
	
	d->encodedData.append(sData);
	
	while (d->encodedData.count() >= 70) //Where is that 70 from ? get it from the plugin.
	{
		d->plugin->decode(d->encodedData.left(70));
		d->encodedData.remove(0, 70);
		d->rawDataForAlsa.append(d->plugin->decodedData());

		if (d->rawDataForAlsa.count() >= d->alsaFSize)
		{
			d->mediaManager->write(d->rawDataForAlsa.left(d->alsaFSize));
			d->rawDataForAlsa.remove(0, d->alsaFSize);
		}
	}
}

void MediaSession::pluginCompress()
{
	if (d->rawData.length() < d->pluginFSize)
	{
		if (d->pluginFSize - d->rawData.length() <= d->pluginFSize/6)
		{
			d->rawData = resample(d->rawData, d->pluginFSize);
		}
		else
		{
			qDebug() << "BAD RETURN !";
			return;
		}
	}
	
	d->plugin->encode(d->rawData.left(d->pluginFSize));
	d->rawData.remove(0, d->pluginFSize);
	d->encodedData = d->plugin->encodedData();
	
	emit readyRead(); // Encoded data is ready to be read and sent over the network.
	
	//outSocket->write(coder->encodedData());
}

/*
 * Resamples data to a packet of size.
 * This method is only for 16 bits samples !
 */
QByteArray MediaSession::resample(QByteArray data, int endSize)
{
	/*
	 * 'X' is the mean of the 2 'x' next to it.
	 * The first X is at position : (startSize/(endSize - startSize + 1)
	 *
	 *'      20 bytes      '            '           28 bytes         '
	 * xxxxxxxxxxxxxxxxxxxx +2 byte  --> xxxxxxxxxxXXxxxxxxxxxx
	 * xxxxxxxxxxxxxxxxxxxx +4 bytes --> xxxxxxXXxxxxxxXXxxxxxxxx
	 * xxxxxxxxxxxxxxxxxxxx +6 bytes --> xxxxxXXxxxxxXXxxxxxXXxxxxx
	 * xxxxxxxxxxxxxxxxxxxx +8 bytes --> xxxxXXxxxxXXxxxxXXxxxxXXxxxx
	 *
	 */

	int startSize = data.count();
	QByteArray ret = data;

	int pos = 0;
	if (endSize - startSize > 0)
	{
		for (int i = 0; i < endSize - startSize; i += 2)
		{
			pos = i * (startSize/(endSize - startSize)) + i;
			if (pos % 2 == 1)
				pos += 1; // Don't insert a value between two bytes that form a 16 bits sample.

			if (pos > 1)
			{
				ret.insert(pos, ret.at(pos - 1));
				ret.insert(pos, ret.at(pos - 2));
			}
			else
			{
				ret.insert(pos, ret.at(pos + 1));
				ret.insert(pos, ret.at(pos));
			}
		}
	}
	else
	{
		for (int i = 0; i < startSize - endSize; i += 2)
		{
			pos = i * (startSize/(startSize - endSize)) + i;
			if (pos % 2 == 1)
				pos += 1; // Don't insert a value between two bytes that form a 16 bits sample.

			ret.remove(pos, 2);
		}
	}

	//qDebug() << "Size after resampling :" << ret.count() << "bytes.";
	return ret;
}

QByteArray MediaSession::read() const
{
	return d->encodedData;
}

/*void MediaSession::slotDecoded()
{
	//kDebug() << "Decoded !";

	QByteArray rawData = d->plugin->decodedData(); //FIXME:what about this QByteArray lifetime ?
	if (rawData.isNull())
	{
		qDebug() << "rawData is NULL !";
		return;
	}
	
	//qDebug() << "rawData =" << rawData.toBase64() << "(" << rawData.size() << "bytes)";
	
	//FIXME: write should become writeAudio, a write Video should be created.
	d->mediaManager->write(rawData);
}*/

int MediaSession::timeStamp()
{
	int ret = d->startTime.msecsTo(QTime::currentTime());
	//kDebug() << "Return value :" << ret;
	return ret;
}
