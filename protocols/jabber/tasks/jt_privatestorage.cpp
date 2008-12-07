 /*
    Copyright (c) 2007      by Olivier Goffart  <ogoffart@kde.org>

    Kopete    (c) 2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "jt_privatestorage.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_client.h"

using namespace XMPP;



//----------------------------------------------------------------------------
// JT_PrivateStorage
//----------------------------------------------------------------------------
class JT_PrivateStorage::Private
{
	public:
		Private() : type(-1) {}

		QDomElement iq;
		QDomElement elem;
		int type;
};

JT_PrivateStorage::JT_PrivateStorage(Task *parent)
	:Task(parent), d(new Private())
{
}

JT_PrivateStorage::~JT_PrivateStorage()
{
	delete d;
}

void JT_PrivateStorage::get(const QString& tag, const QString& xmlns)
{
	d->type = 0;
	d->iq = createIQ(doc(), "get" , QString() , id() );
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "jabber:iq:private");
	d->iq.appendChild(query);
	QDomElement s = doc()->createElement(tag);
	if(!xmlns.isEmpty())
		s.setAttribute("xmlns", xmlns);
	query.appendChild(s);
}

void JT_PrivateStorage::set(const QDomElement& element)
{
	d->type = 1;
	d->elem=element;
	QDomNode n=doc()->importNode(element,true);

	d->iq = createIQ(doc(), "set" , QString() , id() );
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "jabber:iq:private");
	d->iq.appendChild(query);
	query.appendChild(n);
}

void JT_PrivateStorage::onGo()
{
	send(d->iq);
}

bool JT_PrivateStorage::take(const QDomElement &x)
{
	QString to = client()->host();
	if(!iqVerify(x, to, id()))
		return false;

	if(x.attribute("type") == "result") {
		if(d->type == 0) {
			QDomElement q = queryTag(x);
			for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;
				d->elem=i;
				break;
			}
		}
		setSuccess();
		return true;
	}
	else {
		setError(x);
	}

	return true;
}


QDomElement JT_PrivateStorage::element( )
{
	return d->elem;
}

#include "jt_privatestorage.moc"
