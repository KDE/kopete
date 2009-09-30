/*
 * jinglertpapplication.cpp - Jingle RTP Application
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

//TODO: For all XML rendering methods (this doesn't only apply to this class),
//	an argument should be added to tell the method why it has to generate XML.
//	This argument has to be of the form of an enum containing Jingle actions.
//	This way, methods can generate different XML for different actions
//	(add the usable crypto element for session-accept, ...)

#include <QDomElement>
#include <QDebug>

#include "jinglertpapplication.h"
#include "jinglesession.h"

using namespace XMPP;

class JingleRtpApplication::Private
{
public:
	Private() : state(Active) {}
	QList<QDomElement> payloads;
	State state;
	QDomElement xml;
};

JingleRtpApplication::JingleRtpApplication(JingleContent *parent)
 : JingleApplication(parent), d(new Private)
{
	setDescriptionNS(NS_JINGLE_APPS_RTP);
	
	if (parent != 0)
		init();
}

JingleRtpApplication::JingleRtpApplication(const QDomElement& xml, JingleContent *parent)
 : JingleApplication(parent), d(new Private)
{
	setDescriptionNS(NS_JINGLE_APPS_RTP);
	
	d->xml = xml;
	
	if (parent != 0)
		init();
}

JingleRtpApplication::~JingleRtpApplication()
{
	delete d;
}

void JingleRtpApplication::init()
{
	if (d->xml.isNull())
		return;

	// We are not the initiator, saying that it is ringing.
	/*JingleSessionManager *manager = JingleSessionManager::manager();
	if (!manager)
		return;

	manager->*/
	JT_JingleAction *rja = new JT_JingleAction(parent()->rootTask());
	rja->setSession(parent()->parent()); //FIXME:is parent()->parent() always ready at this point ?
	QDomDocument doc("");
	QDomElement ringing = doc.createElement("ringing");
	ringing.setAttribute("xmlns", NS_JINGLE_APPS_RTP_INFO);
	rja->sessionInfo(ringing);
	rja->go(true);

	fromXml(d->xml);
}

QDomElement JingleRtpApplication::bestPayload(const QList<QDomElement>& payloads1, const QList<QDomElement>& payloads2)
{
	/*
	 * FIXME : this is not the best algorithm to determine which one is the best.
	 * |-------|
	 * | a | c |
	 * +---+---+
	 * | b | b |
	 * +---+---+
	 * | d | e |
	 * +---+---+
	 * | c | a |
	 * |-------|
	 *  --> In that case, payload a will be chosen but payload b would be the best choice.
	 */
	
	foreach (QDomElement payload1, payloads1)
	{
		foreach (QDomElement payload2, payloads2)
		{
			if (samePayload(payload1, payload2))
				return payload1;
		}
	}

	return QDomElement();
}

void JingleRtpApplication::fromXml(const QDomElement& e)
{
	setDescriptionNS(e.attribute("xmlns"));
	setMediaType(stringToMediaType(e.attribute("media")));
	
	QDomElement elem = e.firstChildElement();

	while (!elem.isNull())
	{
		if (elem.tagName() == "payload-type")
			addPayload(elem);

		if (elem.tagName() == "encryption")
		{
			setEncryption(true);
			if (elem.hasAttribute("required") && (elem.attribute("required") == "1" || elem.attribute("required") == "true"))
				setEncryptionRequired(true);
			
			//d->srtp = new SRtp(elem.firstChildElement()); // Using only the first crypto element
			//if (!d->srtp->isValid())
			//{
				if (encryptionRequired())
				{
					// FIXME:should only remove the content which wants to use this application.
					//error, terminate the session because we do not support encryption yet.
					JingleContent *c = parent();
					if (!c)
						return;
					JingleSession *s = c->parent();
					if (!s)
						return;
					QDomDocument doc("");

					QDomElement reason = doc.createElement("reason");

					QDomElement se = doc.createElement("security-error");

					QDomElement ic = doc.createElement("invalid-crypto");
					ic.setAttribute("xmlns", "urn:xmpp:jingle:apps:rtp:errors:1");

					reason.appendChild(se);
					reason.appendChild(ic);

					s->sessionTerminate(reason);
					return;
				}
				else
					setEncryption(false); //continue without encryption.
			//}
		}

		elem = elem.nextSiblingElement();
	}
}

void JingleRtpApplication::addPayload(const QDomElement& pl)
{
	d->payloads << pl;
}

void JingleRtpApplication::addPayloads(const QList<QDomElement>& pl)
{
	d->payloads << pl;
}

void JingleRtpApplication::setPayloads(const QList<QDomElement>& pl)
{
	d->payloads.clear();
	d->payloads << pl;
}

QList<QDomElement> JingleRtpApplication::payloads() const
{
	return d->payloads;
}

JingleApplication* JingleRtpApplication::mergeWith(JingleApplication *o)
{
	JingleRtpApplication *other = static_cast<JingleRtpApplication*>(o);

	if (!isCompatibleWith(other))
		return 0;

	JingleRtpApplication *ret = new JingleRtpApplication(parent());
	ret->setMediaType(mediaType());

	foreach (QDomElement payload, d->payloads)
	{
		foreach (QDomElement otherPayload, other->payloads())
		{
			if (samePayload(payload, otherPayload))
				ret->addPayload(otherPayload);
		}
	}

	return ret;
}

bool JingleRtpApplication::isCompatibleWith(JingleApplication* o)
{
	JingleRtpApplication *other = dynamic_cast<JingleRtpApplication*>(o);
	if (!other)
		return false;

	if (other->mediaType() != mediaType())
		return false;
	
	bool ok = false;
	foreach (QDomElement payload, payloads())
	{
		foreach (QDomElement otherPayload, other->payloads())
		{
			if (samePayload(payload, otherPayload))
			{
				ok = true;
				break;
			}
		}
	}

	if (!ok)
	{
		qDebug() << "No compatible payload found !";
		return false;
	}

	qDebug() << "Applications are compatible.";
	return true;
}

bool JingleRtpApplication::samePayload(const QDomElement& p1, const QDomElement& p2)
{
	// Checking payload-type attributes.
	if (!p1.hasAttribute("id") || !p2.hasAttribute("id"))
		return false;

	int id = p1.attribute("id").toInt();
	if ((id >= 96) && (id <= 127)) //dynamic payloads, "name" attribute must be there.
	{
		if (!p1.hasAttribute("name") || !p2.hasAttribute("name"))
			return false;
		if (p1.attribute("name").toLower() != p2.attribute("name").toLower())
			return false;
	}
	else
	{
		// not dynamic payloads, id's must be the same.
		if (p1.attribute("id") != p2.attribute("id"))
			return false;
	}
	
	if (p1.hasAttribute("channels") && p2.hasAttribute("channels"))
		if (p1.attribute("channels") != p2.attribute("channels"))
			return false;
	
	if (p1.hasAttribute("clockrate") && p2.hasAttribute("clockrate"))
		if (p1.attribute("clockrate") != p2.attribute("clockrate"))
			return false;
	
	// Parameters (if there's any) must be the same
	if (p1.hasChildNodes() && p2.hasChildNodes())
	{
		QDomElement pa1 = p1.firstChildElement();
		QDomElement pa2 = p2.firstChildElement();
		
		if (pa1.childNodes().count() != pa2.childNodes().count())
			return false;

		while (!pa1.isNull())
		{
			if (pa1.tagName() != "parameter")
				return false;
			
			bool found = false;

			while (!pa2.isNull())
			{
				if (pa2.tagName() != "parameter")
					return false;

				if (pa1 == pa2)
					found = true;
			
				pa2 = pa2.nextSiblingElement();
			}

			if (!found)
				return false;

			pa1 = pa1.nextSiblingElement();
		}
	}
	else if ((p1.hasChildNodes() && !p2.hasChildNodes()) || (!p1.hasChildNodes() && p2.hasChildNodes()))
	{
		return false;
	}
	
	return true;
}

QDomElement JingleRtpApplication::toXml(const ApplicationType t)
{
	Q_UNUSED(t)

	QDomDocument doc("");
	
	QDomElement desc = doc.createElement("description");
	desc.setAttribute("media", mediaTypeToString(mediaType()));
	desc.setAttribute("xmlns", descriptionNS());
	
	foreach (QDomElement payload, payloads())
	{
		desc.appendChild(payload);
	}

	return desc;
}
	
void JingleRtpApplication::sessionInfo(const QDomElement& info)
{
	if (info.attribute("xmlns") != "urn:xmpp:jingle:apps:rtp:info:1")
		return; //This does not concern us.

	if (info.tagName() == "mute" && d->state == Active)
	{
		d->state = Muted;
		emit mute();
	}
	else if (info.tagName() == "hold" && d->state == Active)
	{
		d->state = Held;
		emit hold();
	}
	else if ((info.tagName() == "unmute" && d->state == Muted) ||
		 (info.tagName() == "unhold" && d->state == Held) ||
		 (info.tagName() == "active" && d->state != Active))
	{
		d->state = Active;
		emit active();
	}
	else if (info.tagName() == "ringing")
	{
		emit ringing();
	}
}

int JingleRtpApplication::componentCountNeeded()
{
	return 2;
}
