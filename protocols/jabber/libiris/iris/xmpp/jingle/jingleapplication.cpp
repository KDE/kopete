/*
 * jingleapplication.cpp - Jingle Application
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
#include <QList>
#include <QString>
#include <QDebug>

#include "jinglesession.h"
#include "jinglecontent.h"
#include "jingleapplication.h"
#include "jinglertpapplication.h"

using namespace XMPP;

class JingleApplication::Private
{
public:
	Private() : encryption(false),
		    encryptionRequired(false),
		    parent(0)
	{}
	QList<QDomElement> payloads; // My payloads.
	
//	Task *rootTask;
	QString descriptionNS;
	MediaType mediaType;

	bool encryption;
	bool encryptionRequired;

	JingleContent *parent;
};


JingleApplication::JingleApplication(JingleContent *parent)
 : d(new Private)
{
	if (parent != 0)
		d->parent = parent;
}

JingleApplication::~JingleApplication()
{
	delete d;
}

JingleApplication* JingleApplication::createFromXml(const QDomElement& elem, JingleContent *parent)
{
	if (elem.attribute("xmlns") == NS_JINGLE_APPS_RTP)
		return new JingleRtpApplication(elem, parent);
	/*else if (elem.attribute("xmlns") == NS_JINGLE_APPS_FT)
		return new JingleFTApplication(parent, elem);*/
	else
		return NULL;
}

void JingleApplication::setParent(JingleContent *c)
{
	if (c == 0)
		return;
	d->parent = c;

	init(); //No, the subclass has not been constructed yet !
}

void JingleApplication::setMediaType(JingleApplication::MediaType t)
{
	d->mediaType = t;
}

JingleApplication::MediaType JingleApplication::mediaType() const
{
	return d->mediaType;
}

QString JingleApplication::mediaTypeToString(JingleApplication::MediaType t)
{
	switch(t)
	{
		case Video :
			return "video";
		case Audio :
			return "audio";
		case FileTransfer :
			return "file transfer";
		default:
			return "unknown";
	}
}

JingleApplication::MediaType JingleApplication::stringToMediaType(const QString& s)
{
	if (s == "video")
		return Video;
	else if (s == "audio")
		return Audio;
	else if (s == "file transfer")
		return FileTransfer;
	else
		return NoType;
}

void JingleApplication::setDescriptionNS(const QString& desc)
{
	d->descriptionNS = desc;
}

QString JingleApplication::descriptionNS() const
{
	return d->descriptionNS;
}

void JingleApplication::setEncryption(bool b)
{
	d->encryption = b;
}

bool JingleApplication::encryption() const
{
	return d->encryption;
}

void JingleApplication::setEncryptionRequired(bool b)
{
	d->encryptionRequired = b;
}

bool JingleApplication::encryptionRequired() const
{
	return d->encryptionRequired;
}

JingleContent* JingleApplication::parent() const
{
	return d->parent;
}
