/*
 * jingles5btransport.cpp - Jingle SOCKS5 Transport
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
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

#include "jingles5btransport.h"

using namespace XMPP;

class JingleS5BTransport::Private
{
public:
	Private() : componentCount(0)
	{}
	
	int componentCount;
	QDomElement elem;
	
};

JingleS5BTransport::JingleS5BTransport(Mode mode, JingleContent *parent, const QDomElement& elem)
 : JingleTransport(mode, parent)
{
	d->elem = elem;

	if (parent)
		init();
}

JingleS5BTransport::~JingleS5BTransport()
{
	delete d;
}

void JingleS5BTransport::init()
{

}

QString JingleS5BTransport::transportNS() const
{
	return "urn:xmpp:jingle:transports:s5b:1";
}

void JingleS5BTransport::start()
{

}

void JingleS5BTransport::addTransportInfo(const QDomElement& elem)
{

}

void JingleS5BTransport::setComponentCount(int n)
{
	d->componentCount = n;
}

QDomElement JingleS5BTransport::toXml(TransportType t)
{

}

void JingleS5BTransport::writeDatagram(const QByteArray& data, Channel c)
{

}

QByteArray JingleS5BTransport::readAll(Channel c)
{

}
