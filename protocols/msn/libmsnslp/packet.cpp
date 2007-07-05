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
#include <qbuffer.h>
#include <kdebug.h>

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

			payload = new QBuffer();
			payload->open(IO_ReadWrite);
			ownsPayload = true;
		}

		Packet::Header header;
		bool ownsPayload;
		QIODevice* payload;
};

Packet::Packet() : d(new PacketPrivate())
{
}

Packet::~Packet()
{
	if (d->ownsPayload) delete d->payload;
	delete d;
}

Packet::Header & Packet::header() const
{
	return d->header;
}

QIODevice* Packet::payload() const
{
	return d->payload;
}

void Packet::setPayload(QIODevice* payload)
{
	if (payload == 0l)
	{
		kdDebug() << k_funcinfo << "Parameter \'payload\' cannot be null."  << endl;
		return;
	}

	if (d->ownsPayload)
	{
		delete d->payload;
		d->ownsPayload = false;
	}

	d->payload = payload;
}

const Q_UINT32 Packet::size() const
{
	return (sizeof(d->header) + d->payload->size());
}

const QString Packet::toString() const
{
	QString s = "\n";
	Q_UINT32 i = 0;
	QString hex;

	const Packet::Header & h = d->header;
	unsigned char *bytes = (unsigned char*)(&h);
	for (Q_INT32 j=0; j < 48; ++j)
	{
		++i;
		unsigned char c = bytes[j];
		if (c < 0x10) hex.append("0");
		hex.append(QString("%1 ").arg(c, 0, 16));

		if (i == 16)
		{
			s += hex;

			if (j == 15)
			{
				s += QString("    Destination: %1, Seq: %2, Offset: %3\n").arg(h.destination)
						.arg(h.identifier).arg((Q_UINT32)h.offset);
			}

			if (j == 31)
			{
				s += QString("    Chunk: %1..%2 of %3,").arg((Q_UINT32)h.offset)
						.arg((Q_UINT32)h.offset+h.payloadSize).arg((Q_UINT32)h.window);
				s += QString(" Flag: 0x%1 (%2)\n").arg(QString::number(h.type, 16).rightJustify(8, '0')).arg(h.type);
			}

			if (j == 47)
			{
				s += QString("    Last Rcvd: %1, Last Sent: %2, Last Len: %3\n").arg(h.lprcvd)
						.arg(h.lpsent).arg((Q_UINT32)h.lpsize);
			}

			hex = QString::null;
			i = 0;
		}
	}

	return s;
}

}
