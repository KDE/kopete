/*
    kirchandlerfactory.h - IRC Handler Factory.

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

#ifndef KIRCHANDLERFACTORY_H
#define KIRCHANDLERFACTORY_H

#include "kirc_export.h"

#include <QtCore/QStringList>

namespace KIrc
{

class Handler;

class KIRC_EXPORT HandlerFactory
{
public:
	static QStringList keys();
	static KIrc::Handler *create(const QString &key, QObject *parent);
};

}

#endif

