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

#include "jt_getlastactivity.h"
#include "xmpp_xmlcommon.h"

using namespace XMPP;

//----------------------------------------------------------------------------
// JT_GetLastActivity
//----------------------------------------------------------------------------
class JT_GetLastActivity::Private
{
	public:
		Private() {}

		int seconds;
		QString message;
};

JT_GetLastActivity::JT_GetLastActivity(Task *parent)
	:Task(parent), d(new Private())
{
}

JT_GetLastActivity::~JT_GetLastActivity()
{
	delete d;
}

void JT_GetLastActivity::get(const Jid &j)
{
	jid = j;
	iq = createIQ(doc(), "get", jid.full(), id());
	QDomElement query = doc()->createElement("query");
	query.setAttribute("xmlns", "jabber:iq:last");
	iq.appendChild(query);
}

int JT_GetLastActivity::seconds() const
{
	return d->seconds;
}

const QString &JT_GetLastActivity::message() const
{
	return d->message;
}

void JT_GetLastActivity::onGo()
{
	send(iq);
}

bool JT_GetLastActivity::take(const QDomElement &x)
{
	if(!iqVerify(x, jid, id()))
		return false;

	if(x.attribute("type") == "result") {
		QDomElement q = queryTag(x);

		d->message = q.text();
		bool ok;
		d->seconds = q.attribute("seconds").toInt(&ok);

		setSuccess(ok);
	}
	else {
		setError(x);
	}

	return true;
}

#include "jt_getlastactivity.moc"
