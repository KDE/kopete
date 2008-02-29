/*
    kirceventhandlerplugin.h - IRC Event Handler Plugin.

    Copyright (c) 2008      by Michel Hermier <michel.hermier@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCEVENTHANDLERPLUGIN_H
#define KIRCEVENTHANDLERPLUGIN_H

#include "kircglobal.h"

#include <QtCore/QList>
#include <QtCore/QtPlugin>

namespace KIrc
{

class EventHandler;

class KIRC_EXPORT EventHandlerPlugin
	: public QObject
{
	Q_OBJECT

public:
	EventHandlerPlugin();
	virtual ~EventHandlerPlugin();

public:
	virtual QStringList keys() = 0;
	virtual KIrc::EventHandler *create(const QString &key, QObject *parent) = 0;

private:
	Q_DISABLE_COPY(EventHandlerPlugin)
};

}

#endif

