/*
    kanimatedsystemtrayicon.cpp  -  System Tray Icon that can play movies
				    Designed for Kopete but usable anywhere

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kanimatedsystemtrayicon.h"

#include <QMovie>

class KAnimatedSystemTrayIcon::Private 
{
	public:
		Private (const QString& m)
		{
			movie = new QMovie (m);
		}
		
		Private (QMovie * m = 0)
		{
			movie = m;
		}

		~Private ()
		{
			delete movie;
		}
		
		QMovie * movie;	
};

KAnimatedSystemTrayIcon::KAnimatedSystemTrayIcon(QWidget *parent)
	: KSystemTrayIcon(parent)
{
	d = new Private(0);
}

KAnimatedSystemTrayIcon::KAnimatedSystemTrayIcon(const QString &movie, QWidget *parent)
	: KSystemTrayIcon(parent),
	d ( new Private (movie))
{

}

KAnimatedSystemTrayIcon::KAnimatedSystemTrayIcon(QMovie* movie, QWidget *parent)
	: KSystemTrayIcon(parent),
	d ( new Private (movie))
{
	
}

KAnimatedSystemTrayIcon::~KAnimatedSystemTrayIcon()
{
	delete d;
}

void KAnimatedSystemTrayIcon::setMovie (QMovie* m)
{
	delete d->movie;
	d->movie = m;
}

const QMovie * KAnimatedSystemTrayIcon::movie () const
{
	return d->movie;
}

void KAnimatedSystemTrayIcon::startMovie()
{
	if (d->movie){
		connect (d->movie, SIGNAL(frameChanged(int)), this, SLOT (slotNewFrame()));
		d->movie->setCacheMode (QMovie::CacheAll);
		d->movie->start();		
	}
}

void KAnimatedSystemTrayIcon::stopMovie()
{
	if (d->movie){
		d->movie->stop();
	}
}

bool KAnimatedSystemTrayIcon::isPlaying() const
{
	if (d->movie->state() == QMovie::Running)
		return true;
	else
		return false;
}

void KAnimatedSystemTrayIcon::slotNewFrame()
{
	setIcon (QIcon (d->movie->currentPixmap()));
}
