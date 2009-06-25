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
#include "speexio.h"

#include <KDebug>
#include <QTime>

class MediaSession::Private
{
public :
	AbstractIO *plugin;
	MediaManager *mediaManager;
	QString codecName;
	QByteArray encodedData;
	QTime startTime;
	int tsValue;
	int ts;
};

MediaSession::MediaSession(MediaManager *mm, const QString& codecName)
 : d(new Private())
{
	d->mediaManager = mm;
	d->codecName = codecName;

	if (d->codecName == "speex")
		d->plugin = new SpeexIO();

	d->ts = 0;

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

	return managerOk && pluginOk;
}

void MediaSession::write(const QByteArray& sData)
{
	//decoding speex data.
	//kDebug() << "Receiving ! (" << sData.size() << "bytes)";
	d->plugin->decode(sData);
}

void MediaSession::slotReadyRead()
{
	//qDebug() << "Ready read";
	d->plugin->encode(d->mediaManager->read());
}

void MediaSession::slotEncoded()
{
	//d->encodedData.clear();
	d->encodedData = d->plugin->encodedData();
	
	emit readyRead(); // Encoded data is ready to be read and sent over the network.
}

QByteArray MediaSession::read() const
{
	return d->encodedData;
}

void MediaSession::slotDecoded()
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
}

int MediaSession::timeStamp()
{
	int ret = d->startTime.msecsTo(QTime::currentTime());
	//kDebug() << "Return value :" << ret;
	return ret;
}
