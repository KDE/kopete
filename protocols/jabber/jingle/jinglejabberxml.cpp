#include "jinglejabberxml.h"

QDomElement createInitializationMessage(QString* from, QString* to, QString* id, QString* sid, QString* initiator, QList<JingleContentType> types)
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
		transport.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0176.html#ns");
		content.append(description);
		content.append(transport);
		jingle.append(content);	
	}


	root.appendChild(jingle);

	return root;
}

QDomElement createAcceptMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QList<JingleContentType> types)
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
		transport.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0176.html#ns");
		content.append(description);
		content.append(transport);
		jingle.append(content);	
	}


	root.appendChild(jingle);

	return root;
}

QDomElement createTerminateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* reason)
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

	return root;

}

QDomElement createReceiptMessage(QDomElement stanza);
{
	QDomElement iq = createIQ(doc(), "result", stanza.attribute("from"), stanza.attribute("id"));
	return iq;
}

QDomElement createContentErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(doc(), "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement fni = doc.createElement("feature-not-implemented");
	fni.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement uc = doc.createElement("unsupported-content");
	uc.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.append(fni);
	error.append(uc);
	iq.append(error);

	return (iq);
}

QDomElement createTransportErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(doc(), "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement fni = doc.createElement("feature-not-implemented");
	fni.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement ut = doc.createElement("unsupported-transports");
	ut.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.append(fni);
	error.append(ut);
	iq.append(error);

	return (iq);
}

QDomElement createOrderErrorMessage(QDomElement stanza)
{
	QDomDocument doc;
	QDomElement iq = createIQ(doc(), "error", stanza.attribute("from"), stanza.attribute("id"));
	QDomElement error = doc.createElement("error");
	error.setAttribute("type","cancel");
	QDomElement ur = doc.createElement("unexpected-request");
	ur.setAttribute("xmlns","urn:ietf:params:sml:ns:xmpp-stanzas");
	QDomElement ooo = doc.createElement("out-of-order");
	ooo.setAttribute("xmlns","http://www.xmpp.org/extensions/xep-0166.html#ns-errors");
	error.append(ur);
	error.append(ooo);
	iq.append(error);

	return (iq);
}