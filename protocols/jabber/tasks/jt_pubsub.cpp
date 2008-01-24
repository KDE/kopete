 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "jt_pubsub.h"

#include "xmpp_xmlcommon.h"
#include "xmpp_client.h"

using namespace XMPP;

#define PUBSUB_NS "http://jabber.org/protocol/pubsub"

JT_PubSubPublish::JT_PubSubPublish(XMPP::Task *parent, const QString &node, XMPP::PubSubItem &psitem):
Task(parent)
{
	mIQ = createIQ(doc(), "set", "", id());
	QDomElement pubsub = doc()->createElement("pubsub");
	pubsub.setAttribute("xmlns", PUBSUB_NS);
	mIQ.appendChild(pubsub);
	QDomElement publish = doc()->createElement("publish");
	publish.setAttribute("node", node);
	pubsub.appendChild(publish);
	QDomElement item = doc()->createElement("item");
	item.setAttribute("id", psitem.id());
	publish.appendChild(item);
	item.appendChild(psitem.payload());
}

JT_PubSubPublish::~JT_PubSubPublish()
{
}

void JT_PubSubPublish::onGo()
{
	send(mIQ);
}

bool JT_PubSubPublish::take(const QDomElement &x)
{
	if(!iqVerify(x, "", id()))
		return false;
	if(x.attribute("type") == "result")
		setSuccess();
	else
		setError(x);
	return true;
}
