/*
    jinglejabberxml.cpp - Jingle jabber stanza creation.

    Copyright (c) 2007      by Joshua Hodosh     <josh.hodosh@gmail.com>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "jinglejabberxml.h"

#include "jingleconnectioncandidate.h"

#include <QList>
#include <QtXml>

QList<QDomElement> JingleContentType::payloads()
{
	QList<QDomElement> elements;
	if (xmlns == JINGLE_FOO_NS){
		QDomElement pl;
		pl.setTagName("payload");
		pl.setAttribute("codepage","ascii");
		elements.append(pl);
	}
	return elements;
}

QDomDocument Jingle::createInitializationMessage(QString from, QString to, QString id, QString sid, QString initiator, QList<JingleContentType> types)
{
	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set", to, id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("sid", sid);
	for(int i=0; i<types.size();i++){
		QDomElement content= doc.createElement("content");
		content.setAttribute("creator",types[i].creator);
		content.setAttribute("name",types[i].name);
		QDomElement description = doc.createElement("description");
		description.setAttribute("xmlns",types[i].xmlns);
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns",types[i].transportNS);
		transport.appendChild(types[i].connection->getCandidateElement());

		content.appendChild(description);
		content.appendChild(transport);
		jingle.appendChild(content);	
	}


	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;
}

QDomDocument Jingle::createAcceptMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QList<JingleContentType> types)
{

	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set",to,id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-accept");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("responder", responder);
	jingle.setAttribute("sid", sid);
	for(int i=0; i<types.size();i++){
		QDomElement content= doc.createElement("content");
		content.setAttribute("creator",initiator);
		content.setAttribute("name",types[i].name);
		QDomElement description = doc.createElement("description");
		description.setAttribute("xmlns",types[i].xmlns);
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns",types[i].transportNS);
		transport.appendChild(types[i].connection->getCandidateElement());
		content.appendChild(description);
		content.appendChild(transport);
		jingle.appendChild(content);	
	}

	root.appendChild(jingle);
	doc.appendChild(root);

	Q_UNUSED(from);
	return doc;
}

QDomDocument Jingle::createTerminateMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QString reason)
{
	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set",to,id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-terminate");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("responder", responder);
	jingle.setAttribute("sid", sid);
	jingle.setAttribute("reason", reason);

	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;

}

QDomDocument Jingle::createReceiptMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "result", stanza.attribute("from"), stanza.attribute("id"));
	doc.appendChild(iq);
	return doc;
}

QDomDocument Jingle::createContentErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement fni = doc.createElement("feature-not-implemented");
	fni.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement uc = doc.createElement("unsupported-content");
	uc.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.appendChild(fni);
	error.appendChild(uc);
	iq.appendChild(error);

	doc.appendChild(iq);
	return doc;
}

QDomDocument Jingle::createTransportErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement fni = doc.createElement("feature-not-implemented");
	fni.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement ut = doc.createElement("unsupported-transports");
	ut.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.appendChild(fni);
	error.appendChild(ut);
	iq.appendChild(error);

	doc.appendChild(iq);
	return doc;
}

QDomDocument Jingle::createOrderErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement ur = doc.createElement("unexpected-request");
	ur.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement ooo = doc.createElement("out-of-order");
	ooo.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.appendChild(ur);
	error.appendChild(ooo);
	iq.appendChild(error);

	doc.appendChild(iq);
	return (doc);
}

QDomDocument Jingle::createTransportNotAcceptableMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement na = doc.createElement("not-acceptable");
	na.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	error.appendChild(na);
	iq.appendChild(error);

	doc.appendChild(iq);
	return (doc);
}

QDomDocument Jingle::createUnknownSessionError(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement  br= doc.createElement("bad-request");
	br.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	error.appendChild(br);
	QDomElement us = doc.createElement("unknown-session");
	us.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.appendChild(us);
	iq.appendChild(error);

	doc.appendChild(iq);
	return (doc);
}

QDomDocument Jingle::createTransportCandidateMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QString contentName, QString contentCreator, QString transportNS, JingleConnectionCandidate *candidate)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "set", to, id);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "transport-info");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("responder", responder);
	jingle.setAttribute("sid", sid);
	QDomElement content = doc.createElement("content");
	content.setAttribute("name",contentName);
	content.setAttribute("creator", contentCreator);
	QDomElement transport = doc.createElement("transport");
	transport.setAttribute("xmlns", transportNS);
	transport.appendChild(candidate->getCandidateElement());
	content.appendChild(transport);
	jingle.appendChild(content);
	iq.appendChild(jingle);

	doc.appendChild(iq);
	return (doc);
}

QDomDocument Jingle::createContentAcceptMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, JingleContentType content)
{
	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set", to, id);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "content-accept");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("sid", sid);

	QDomElement contentElement= doc.createElement("content");
	contentElement.setAttribute("creator",content.creator);
	contentElement.setAttribute("name",content.name);
	QDomElement description = doc.createElement("description");
	description.setAttribute("xmlns",content.xmlns);
	foreach(QDomElement payload, content.payloads())
		description.appendChild(payload);
	QDomElement transport = doc.createElement("transport");
	transport.setAttribute("xmlns",content.transportNS);
	transport.appendChild(content.connection->getCandidateElement());


	contentElement.appendChild(description);
	contentElement.appendChild(transport);
	jingle.appendChild(contentElement);	



	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;

}
