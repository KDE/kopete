/*
 * bytestream.cpp - base class for bytestreams
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"xmpp_bytestream.h"


//----------------------------------------------------------------------------
// ByteStream
//----------------------------------------------------------------------------
class ByteStream::Private
{
public:
	Private() {}
};

ByteStream::ByteStream(QObject *parent)
:QObject(parent)
{
	d = new Private;
}

ByteStream::~ByteStream()
{
	delete d;
}

bool ByteStream::isConnected() const
{
	return false;
}

void ByteStream::close()
{
}

void ByteStream::write(const QByteArray &)
{
}

QByteArray ByteStream::read()
{
	return QByteArray();
}

bool ByteStream::canRead() const
{
	return false;
}

int ByteStream::bytesToWrite() const
{
	return 0;
}

#include "xmpp_bytestream.moc"
