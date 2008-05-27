/*
    kirchandlerfactory.cpp - IRC Handler Factory.

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

#include "kirchandlerfactory.h"

using namespace KIrc;

QStringList HandlerFactory::keys()
{
	return QStringList();
}

KIrc::Handler *HandlerFactory::create(const QString &key, QObject *parent)
{
	return 0;
}

