/*
    packet.cpp - Peer to Peer Transport Layer Packet class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "packet.h"
#include <qtextstream.h>

namespace PeerToPeer
{

class Packet::PacketPrivate
{
	public:
		PacketPrivate()
		{
			header.destination = 0;
			header.identifier = 0;
			header.offset = 0l;
			header.window = 0l;
			header.payloadSize = 0;
			header.type = 0;
			header.lprcvd = 0;
			header.lpsent = 0;
			header.lpsize = 0l;

			payload.open(IO_ReadWrite);
			priority = 1;
		}

		Packet::Header header;
		QBuffer payload;
		Q_UINT32 priority;
};

Packet::Packet() : d(new PacketPrivate())
{
}

Packet::Packet(const Packet& other) : d(new PacketPrivate())
{
	*this = other;
}

Packet& Packet::operator=(const Packet& other)
{
	d->header = other.header();
	if (other.header().window > 0)
	{
		d->payload.writeBlock(other.payload().buffer());
	}
	d->priority = other.priority();
	return *this;
}

Packet::~Packet()
{
	delete d;
	d = 0l;
}

Packet::Header & Packet::header() const
{
	return d->header;
}

const QBuffer & Packet::payload() const
{
	return d->payload;
}

QBuffer & Packet::payload()
{
	return d->payload;
}

const Q_UINT32 Packet::priority() const
{
	return d->priority;
}

void Packet::setPriority(const Q_UINT32 priority) const
{
	d->priority = priority;
}

const Q_UINT32 Packet::size() const
{
	return (sizeof(d->header) + d->payload.size());
}

bool Packet::operator==(const Packet& other)
{
	return (d->header.destination == other.header().destination &&
			d->header.identifier == other.header().identifier &&
			d->header.offset == other.header().offset &&
			d->header.window == other.header().window &&
			d->header.payloadSize == other.header().payloadSize &&
			d->header.type == other.header().type &&
			d->header.lprcvd == other.header().lprcvd &&
			d->header.lpsent == other.header().lpsent &&
			d->header.lpsize == other.header().lpsize &&
			d->priority == other.priority());
}

const QString Packet::toString() const
{
	QString string;
	QTextStream(string, IO_WriteOnly)
	<< "[destination]   " << QString::number(d->header.destination) << endl
	<< "[identifier]    " << QString::number(d->header.identifier) << endl
	<< "[offset]        " << QString::number(d->header.offset) << endl
	<< "[window]        " << QString::number(d->header.window) << endl
	<< "[payload size]  " << QString::number(d->header.payloadSize) << endl
	<< "[flag]          " << QString::number(d->header.type) << endl
	<< "[lprcvd]        " << QString::number(d->header.lprcvd) << endl
	<< "[lpsent]        " << QString::number(d->header.lpsent) << endl
	<< "[lpsize]        " << QString::number(d->header.lpsize) << endl;

	return string;
}

}
