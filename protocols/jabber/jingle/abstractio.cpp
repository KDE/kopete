/*
 * abstractio.cpp -- An abstract class to create encoder/decoder for Kopete Jingle.
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

#include "abstractio.h"

AbstractIO::AbstractIO()
{

}

AbstractIO::~AbstractIO()
{

}


void AbstractIO::encode(const QByteArray& data)
{
	Q_UNUSED(data)
}

void AbstractIO::decode(const QByteArray& data)
{
	Q_UNUSED(data)
}

QByteArray AbstractIO::encodedData() const
{
	return QByteArray();
}

QByteArray AbstractIO::decodedData() const
{
	return QByteArray();
}

int AbstractIO::tsValue()
{
	return 0;
}

int AbstractIO::frameSizeBytes()
{
	return 0;
}
