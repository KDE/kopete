/*
   connector.cpp - Papillon Socket connector abstract class.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Based on code Copyright (c) 2004 Matt Rogers <mattr@kde.org>
   Based on code copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
   Based on Iris, Copyright (C) 2003  Justin Karneges

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "Papillon/Base/Connector"

namespace Papillon
{

class Connector::Private
{
public:
	bool haveaddr;
	QHostAddress addr;
	quint16 port;
};

Connector::Connector(QObject *parent)
:QObject(parent), d(new Private())
{
	setPeerAddressNone();
}

Connector::~Connector()
{
	delete d;
}

bool Connector::havePeerAddress() const
{
	return d->haveaddr;
}

QHostAddress Connector::peerAddress() const
{
	return d->addr;
}

quint16 Connector::peerPort() const
{
	return d->port;
}

void Connector::setPeerAddressNone()
{
	d->haveaddr = false;
	d->addr = QHostAddress();
	d->port = 0;
}

void Connector::setPeerAddress(const QHostAddress &_addr, quint16 _port)
{
	d->haveaddr = true;
	d->addr = _addr;
	d->port = _port;
}

}

#include "connector.moc"
