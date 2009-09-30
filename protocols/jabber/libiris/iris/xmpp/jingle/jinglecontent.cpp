/*
 * jinglecontent.cpp - Jingle content
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

#include "jinglecontent.h"
#include "jinglesession.h"
#include "jingleapplication.h"
#include "jingletransport.h"

#include <QDomElement>
#include <QUdpSocket>
#include <QDebug>

//----------------------
// JingleContent
//----------------------

using namespace XMPP;


class JingleContent::Private
{
public:
	Private() : senders(Both),
		    established(false),
		    parent(0),
		    transport(0),
		    application(0),
		    started(false),
		    reason(NoReason)
	{}
	
	Task *rootTask;
	QString creator;
	Senders senders;
	QString name;
	bool established;
	//Mode mode;
	JingleSession *parent;

	JingleTransport *transport;
	QList<JingleApplication*> supportedApplications;
	JingleApplication *application;

	QList<QDomElement> pendingTransportInfo;
	QList<QDomElement> pendingSessionInfo;

	bool started;

	ReasonType reason;
};

JingleContent::JingleContent(JingleSession *parent)
: d(new Private())
{
	setParent(parent);
}

JingleContent::~JingleContent()
{
	delete d->transport;
	delete d->application;
	
	delete d;
}

void JingleContent::setParent(JingleSession *parent)
{
	if (parent == 0)
		return;

	d->parent = parent;
	d->rootTask = d->parent->rootTask();
}

void JingleContent::start()
{
	if (d->started || !d->transport || !d->application || !d->parent)
		return;
	
	JingleApplication *app = d->application;
	
	d->transport->setComponentCount(app->componentCountNeeded());
	d->transport->start();
	
	if (!d->pendingTransportInfo.isEmpty())
	{
		foreach (QDomElement e, d->pendingTransportInfo)
			d->transport->addTransportInfo(e);

		d->pendingTransportInfo.clear();
	}

	if (!d->pendingSessionInfo.isEmpty())
	{
		foreach (QDomElement e, d->pendingSessionInfo)
			d->application->sessionInfo(e);
		
		d->pendingSessionInfo.clear();
	}

	d->started = true;
}

void JingleContent::stopNegotiation()
{
	if (d->transport && !d->transport->isConnected())
	{
		delete d->transport;
		d->transport = 0;
	}
}

void JingleContent::setTransport(JingleTransport* t)
{
	//FIXME:Should d->transport be deleted first ?
	d->transport = t;
	t->setParent(this);
	connect(t, SIGNAL(success()), SIGNAL(established()));
	//connect(t, SIGNAL(failure()), SLOT(transportFailure()));

	start();
}

JingleSession *JingleContent::parent() const
{
	return d->parent;
}

void JingleContent::setCreator(const QString& c)
{
	d->creator = c;
}

void JingleContent::setSenders(const Senders s)
{
	if (d->senders == s)
		return;

	d->senders = s;

	emit sendersChanged();
}

void JingleContent::setName(const QString& n)
{
	d->name = n;
}

/*
 * TODO:
 * this method could return an error value which will be used by the session
 * to remove the content or terminate the session with the right reason.
 */
void JingleContent::fromElement(const QDomElement& e)
{
	if (e.tagName() != "content")
		return;

	d->creator = e.attribute("creator");
	d->name = e.attribute("name");
	
	QDomElement elem = e.firstChildElement();

	while (!elem.isNull())
	{
		if (elem.tagName() == "description")
		{
			JingleApplication *a = JingleApplication::createFromXml(elem, this);
			if (!a)
			{
				qDebug() << "Application type not supported (or you don't have free memory on your computer)";
				parent()->removeContent(this);
				return;
			}

			foreach (JingleApplication *app, d->supportedApplications)
			{
				if (a->isCompatibleWith(app))
				{
					setApplication(a->mergeWith(app));
					break;
				}
			}

		}
		else if (elem.tagName() == "transport")
		{
			JingleTransport *t = JingleTransport::createFromXml(elem, JingleTransport::Responder, this);
			setTransport(t);
		}

		elem = elem.nextSiblingElement();
	}

	if (!application())
	{
		// No compatible payload in the application (or at least, not compatible/supported)
		qDebug() << "Content" << name() << "is not compatible.";
		d->reason = UnsupportedApplication;
		parent()->removeContent(this);
		return;
		// No need to go further.
	}
	
	if (!transport())
	{
		qDebug() << "Transport method not supported (or you don't have free memory on your computer)";
		d->reason = UnsupportedTransport;
		parent()->removeContent(this);
		return;
	}

	start();
}

QDomElement JingleContent::contentElement(JingleTransport::TransportType tType, JingleApplication::ApplicationType aType)
{
	// Create the QDomElement which has to be returned.
	QDomDocument doc("");

	QDomElement content = doc.createElement("content");
	content.setAttribute("creator", d->creator);
	content.setAttribute("name", d->name);
	content.setAttribute("senders", sendersToStr(d->senders));

	/*switch (aType)
	{
	case JingleApplication::Application :*/
	if (d->application)
		content.appendChild(d->application->toXml(aType));
	/*	break;

	default :
		break;
	}*/
	
	if (d->transport)
		content.appendChild(d->transport->toXml(tType));

	return content;
}

QString JingleContent::name() const
{
	return d->name;
}

QString JingleContent::creator() const
{
	return d->creator;
}

JingleContent::Senders JingleContent::senders() const
{
	return d->senders;
}

JingleContent& JingleContent::operator=(const JingleContent &other)
{
	d->transport = other.transport();
	d->creator = other.creator();
	d->senders = other.senders();
	d->name = other.name();
	d->established = other.d->established;

	d->supportedApplications = other.d->supportedApplications;
	d->application = other.application();

	d->pendingTransportInfo = other.d->pendingTransportInfo;

	return *this;
}

bool JingleContent::isReady() const
{
	return d->transport->isConnected();
}

void JingleContent::setRootTask(Task *rt)
{
	d->rootTask = rt;
}

Task* JingleContent::rootTask() const
{
	return d->rootTask;
}

void JingleContent::activated()
{
	
}

void JingleContent::muted()
{

}

void JingleContent::addTransportInfo(const QDomElement& e)
{
	if (d->transport)
		d->transport->addTransportInfo(e);
	else
		d->pendingTransportInfo << e;
}

void JingleContent::writeDatagram(const QByteArray& data, JingleTransport::Channel c)
{
	d->transport->writeDatagram(data, c);
}

QByteArray JingleContent::readAll(JingleTransport::Channel c)
{
	return d->transport->readAll(c);
}

void JingleContent::setSupportedApplications(QList<JingleApplication*> apps)
{
	d->supportedApplications = apps;
}

void JingleContent::sessionInfo(const QDomElement& info)
{
	if (info.hasAttribute("name") && info.attribute("name") != name())
	{
		//This does not concerns us.
		return;
	}
	
	// SessionInfo's are for the application. Transports use TransportInfo's.
	if (application())
		d->application->sessionInfo(info);
	else
		d->pendingSessionInfo << info;
}

JingleTransport* JingleContent::transport() const
{
	return d->transport;
}

QString JingleContent::sendersToStr(const Senders s)
{
	switch (s)
	{
	case Both :
		return "both";
	case Initiator :
		return "initiator";
	case Responder :
		return "responder";
	}

	return "both";
}

JingleContent::Senders JingleContent::strToSenders(const QString& s)
{
	if (s == "responder")
		return Responder;
	else if (s == "initiator")
		return Initiator;
	else
		return Both;
}

void JingleContent::setApplication(JingleApplication* a)
{
	d->application = a;
	d->application->setParent(this);
	start();
}

JingleApplication* JingleContent::application() const
{
	return d->application;
}

JingleContent::ReasonType JingleContent::reason() const
{
	return d->reason;
}
