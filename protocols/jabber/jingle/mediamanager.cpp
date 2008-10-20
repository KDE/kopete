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
#include <KDebug>

#include "mediamanager.h"
#include "alsaio.h"

class MediaManager::Private
{
public:
	AlsaIO *alsaIn;
	AlsaIO *alsaOut;

	bool started;
};

MediaManager::MediaManager(QString inputDev, QString outputDev)
 : d(new Private)
{
	d->alsaIn = new AlsaIO(AlsaIO::Capture, inputDev, AlsaIO::Signed16Le);
	d->alsaOut = new AlsaIO(AlsaIO::Playback, "default", AlsaIO::Signed16Le);

	d->started = false;

	qDebug() << "Created Media Manager.";
}

MediaManager::~MediaManager()
{
	delete d->alsaIn;
	delete d->alsaOut;
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
	return (d->started = d->alsaIn->start() && d->alsaOut->start());
}

QByteArray MediaManager::read()
{
	return alsaIn()->data();
}

void MediaManager::write(const QByteArray& data)
{
	//kDebug() << "Writin on alsa device !";
	alsaOut()->write(data);
}

