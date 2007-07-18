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
