 /*
  * mediamanager.cpp - A manager for the media sessions.
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

#include "mediamanager.h"
#include "mediasession.h"
#include "alsaio.h"

#include <KDebug>

class MediaManager::Private
{
public:
	AlsaIO *alsaIn;
	AlsaIO *alsaOut;
	QString inputDev;
	QString outputDev;

	QList<MediaSession*> sessions;

	bool started;
};

MediaManager::MediaManager(QString inputDev, QString outputDev)
 : d(new Private)
{
	d->inputDev = inputDev;
	d->outputDev = outputDev;

	d->alsaIn = 0;
	d->alsaOut = 0;

	d->started = false;

	qDebug() << "Created Media Manager.";
}

MediaManager::~MediaManager()
{
	stop();
        delete d;
	qDebug() << "Deleted Media Manager.";
}

AlsaIO *MediaManager::alsaIn() const
{
	return d->alsaIn;
}

AlsaIO *MediaManager::alsaOut() const
{
	return d->alsaOut;
}

bool MediaManager::start()
{
	if (d->started)
		return true;
	
	bool in = false, out = false;
	
	d->alsaIn = new AlsaIO(AlsaIO::Capture, d->inputDev, AlsaIO::Signed16Le);
	d->alsaOut = new AlsaIO(AlsaIO::Playback, d->outputDev, AlsaIO::Signed16Le);

	if (d->alsaIn->start())
		in = true;
	if (d->alsaOut->start())
		out = true;
		
	return (d->started = (in && out));
}

void MediaManager::stop()
{
	delete d->alsaIn;
	d->alsaIn = 0;
	
	delete d->alsaOut;
	d->alsaOut = 0;

	d->started = false;
}

QByteArray MediaManager::read()
{
	if (alsaIn())
		return alsaIn()->data();
	return 0;
}

void MediaManager::write(const QByteArray& data)
{
	//kDebug() << "Writin on alsa device !";
	if (alsaOut())
		alsaOut()->write(data);
}

bool MediaManager::addSession(MediaSession *sess)
{
	bool ret = true;
	if (d->sessions.count() == 0)
		ret = start();
	d->sessions << sess;

	return ret;
}

void MediaManager::removeSession(MediaSession *sess)
{
	for (int i = 0; i < d->sessions.count(); i++)
	{
		if (d->sessions.at(i) == sess)
		{
			d->sessions.removeAt(i);
			break;
		}
	}

	if (d->sessions.count() == 0)
		stop();
}
