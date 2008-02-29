/*
    kirceventhandlerfactory.h - IRC event handler factory.

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

#ifndef KIRCEVENTHANDLERFACTORY_H
#define KIRCEVENTHANDLERFACTORY_H

#include "kircglobal.h"

#include <QtCore/QStringList>

namespace KIrc
{

//class Context;

class KIRC_EXPORT EventHandlerFactory
{
public:
	static QStringList keys();
//	static KIrc::EventHandler *create(const QString &key, QObject *parent);
};

}

#endif

