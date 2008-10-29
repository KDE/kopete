/*
    kirchandler_p.h - IRC handler private.

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

#ifndef KIRCHANDLER_P_H
#define KIRCHANDLER_P_H

#include "kirchandler.h"

#include <QtCore/QByteArray>
#include <QtCore/QMultiHash>

class KIrc::HandlerPrivate
{
public:
	HandlerPrivate()
		: enabled(true)
	{
	}

	bool enabled;
	QList<KIrc::Handler*> eventHandlers;
	QMultiHash<QByteArray, QByteArray> commandAliases;
	QMultiHash<QByteArray, QByteArray> messageAliases;
};

#endif // KIRCHANDLER_P_H
