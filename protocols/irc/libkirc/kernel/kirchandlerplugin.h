/*
    kirchandlerplugin.h - IRC Handler Plugin.

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

#ifndef KIRCHANDLERPLUGIN_H
#define KIRCHANDLERPLUGIN_H

#include "kirc_export.h"

#include <QtCore/QList>
#include <QtCore/QtPlugin>

namespace KIrc
{

class Handler;

class KIRC_EXPORT HandlerPlugin
	: public QObject
{
	Q_OBJECT

public:
	HandlerPlugin();
	virtual ~HandlerPlugin();

public:
	virtual QStringList keys() = 0;
	virtual KIrc::Handler *create(const QString &key, QObject *parent) = 0;

private:
	Q_DISABLE_COPY(HandlerPlugin)
};

}

#endif

