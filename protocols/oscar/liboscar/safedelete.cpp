/*
    safedelete.cpp - Kopete Groupwise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "safedelete.h"
#include <QList>
#include <qtimer.h>

//----------------------------------------------------------------------------
// SafeDelete
//----------------------------------------------------------------------------
SafeDelete::SafeDelete()
{
	lock = 0;
}

SafeDelete::~SafeDelete()
{
	if(lock)
		lock->dying();
}

void SafeDelete::deleteLater(QObject *o)
{
	if(!lock)
		deleteSingle(o);
	else
		list.append(o);
}

void SafeDelete::unlock()
{
	lock = 0;
	deleteAll();
}

void SafeDelete::deleteAll()
{
	if(list.isEmpty())
		return;

	foreach( QObject* o, list )
		deleteSingle( o );
	list.clear();
}

void SafeDelete::deleteSingle(QObject *o)
{
	o->deleteLater();
}

//----------------------------------------------------------------------------
// SafeDeleteLock
//----------------------------------------------------------------------------
SafeDeleteLock::SafeDeleteLock(SafeDelete *sd)
{
	own = false;
	if(!sd->lock) {
		_sd = sd;
		_sd->lock = this;
	}
	else
		_sd = 0;
}

SafeDeleteLock::~SafeDeleteLock()
{
	if(_sd) {
		_sd->unlock();
		if(own)
			delete _sd;
	}
}

void SafeDeleteLock::dying()
{
	_sd = new SafeDelete(*_sd);
	own = true;
}

//----------------------------------------------------------------------------
// SafeDeleteLater
//----------------------------------------------------------------------------
SafeDeleteLater *SafeDeleteLater::self = 0;

SafeDeleteLater *SafeDeleteLater::ensureExists()
{
	if(!self)
		new SafeDeleteLater();
	return self;
}

SafeDeleteLater::SafeDeleteLater()
{

	self = this;
	QTimer::singleShot(0, this, SLOT(explode()));
}

SafeDeleteLater::~SafeDeleteLater()
{
	list.clear();
	self = 0;
}

void SafeDeleteLater::deleteItLater(QObject *o)
{
	list.append(o);
}

void SafeDeleteLater::explode()
{
	delete this;
}

#include "safedelete.moc"

