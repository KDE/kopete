/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

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

#include jingleconnectioncandidate.h"

QDomDocument Jingle::createInitializationMessage(QString* from, QString* to, QString* id, QString* sid, QString* initiator, QList<JingleContentType> types)
{
	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set",to,id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-initiate");
	jingle.setAttribute("initiator", from);
	jingle.setAttribute("sid", sid);
	for(int i=0; i<types.length();i++){
		QDomElement content= doc.createElement("content");
		content.setAttribute("creator",initiator);
		content.setAttribute("name",types[i].name);
		QDomElement description = doc.createElement("description");
		description.setAttribute("xmlns",types[i].xmlns);
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns",types[i].transportNS);

//		for(int j=0; j<types[i].candidates.length(); j++){
//			transport.append(types[i].candidates[j].getCandidateElement();
//		}

		content.append(description);
		content.append(transport);
		jingle.append(content);	
	}


	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;
}

QDomDocument Jingle::createAcceptMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QList<JingleContentType> types, JingleConnectionCandidate connection)
{

	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set",to,id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-accept");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("responder", responder);
	jingle.setAttribute("sid", id);
	for(int i=0; i<types.length();i++){
		QDomElement content= doc.createElement("content");
		content.setAttribute("creator",initiator);
		content.setAttribute("name",types[i].name);
		QDomElement description = doc.createElement("description");
		description.setAttribute("xmlns",types[i].xmlns);
		QDomElement transport = doc.createElement("transport");
		transport.setAttribute("xmlns",types[i].transportNS);
		content.append(description);
		content.append(transport);
		jingle.append(content);	
	}


	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;
}

QDomDocument Jingle::createTerminateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* reason)
{
	QDomDocument doc;
	QDomElement root = createIQ(&doc,"set",to,id);
	//root.setAttribute("from", from);
	QDomElement jingle = doc.createElement("jingle");
	jingle.setAttribute("xmlns", "http://www.xmpp.org/extensions/xep-0166.html#ns");
	jingle.setAttribute("action", "session-terminate");
	jingle.setAttribute("initiator", initiator);
	jingle.setAttribute("responder", responder);
	jingle.setAttribute("sid", id);
	jingle.setAttribute("reason", reason);

	root.appendChild(jingle);
	doc.appendChild(root);

	return doc;

}

QDomDocument Jingle::createReceiptMessage(QDomElement stanza);
{
	QDomElement doc;
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
	error.append(fni);
	error.append(uc);
	iq.append(error);

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
	error.append(fni);
	error.append(ut);
	iq.append(error);

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
	error.append(ur);
	error.append(ooo);
	iq.append(error);

	doc.appendChild(iq);
	return (doc);
}

QDomDocument Jingle::createTransportNotAcceptableMessage(QDocument stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(&doc, "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement na = doc.createElement("not-acceptable");
	na.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	error.append(na);
	iq.append(error);

	doc.appendChild(iq);
	return (doc);
}
