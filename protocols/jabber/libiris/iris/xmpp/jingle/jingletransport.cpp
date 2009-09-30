/*
 * jingletransport.cpp - Jingle Transport
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

#include <QDomElement>
#include <QDebug>
#include <QtCrypto>

#include "jinglesession.h"
#include "jinglecontent.h"
#include "jingletransport.h"
#include "jingleicetransport.h"
//#include "jingles5btransport.h"

using namespace XMPP;

class JingleTransport::Private
{
public:
	Private() : parent(0)
	{}

	Mode mode;
	JingleContent *parent;
	
	QDomElement transport;

	bool connected;
};

JingleTransport::JingleTransport(Mode mode, JingleContent *parent)
: d(new Private())
{
	d->mode = mode;
	
	if (parent == 0)
		return;
	
	d->parent = parent;
	d->connected = false;
}

JingleTransport::~JingleTransport()
{
	delete d;
}

void JingleTransport::setParent(JingleContent *c)
{
	if (c == 0)
		return;
	
	d->parent = c;
	
	init();
}

JingleSession *JingleTransport::parentSession() const
{
	if (d->parent)
		return d->parent->parent();
	
	return 0;
}

Task* JingleTransport::rootTask()
{
	if (d->parent)
		return d->parent->rootTask();
	
	return 0;
}

JingleTransport::Mode JingleTransport::mode() const
{
	return d->mode;
}

JingleContent *JingleTransport::parent() const
{
	return d->parent;
}

JingleTransport* JingleTransport::createFromXml(const QDomElement& elem, Mode mode, JingleContent *parent)
{
	if (elem.attribute("xmlns") == NS_JINGLE_TRANSPORTS_ICE && QCA::isSupported("hmac(sha1)"))
		return new JingleIceTransport(mode, parent, elem);
	//else if (elem.attribute("xmlns") == "urn:xmpp:jingle:transports:s5b:1")
	//	return new JingleS5BTransport(mode, parent, elem);
	else
		return NULL;
}

QString JingleTransport::transportNS(const QDomElement& c)
{
	QString ret;
	QDomElement content = c;

	if (content.tagName() != "content")
	{
		qDebug() << "This is not a content xml element.";
		return ret;
	}

	QDomElement transport = content.firstChildElement();
	while (!transport.isNull())
	{
		if (transport.tagName() == "transport")
		{
			ret = transport.attribute("xmlns");
		}
		transport = transport.nextSiblingElement();
	}

	return ret;
}

QDomElement JingleTransport::transport() const
{
	return d->transport;
}

void JingleTransport::setTransport(const QDomElement& t)
{
	d->transport = t;
}

bool JingleTransport::isConnected() const
{
	return d->connected;
}

JingleTransport *JingleTransport::fallbackTransport()
{
	return 0;
}

void JingleTransport::setConnected(bool e)
{
	d->connected = e;
}

