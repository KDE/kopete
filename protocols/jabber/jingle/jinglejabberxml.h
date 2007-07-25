#ifndef JINGLEJABBERXML_H_
#define JINGLEJABBERXML_H_ 

//#include <QtXml>

#include "xmpp_xmlcommon.h"

//#include "jinglecontenttype.h"

//BEGIN JingleContentType
struct JingleContentType
{
	JingleContentType(xmlns_,name_,transNS):xmlns(xmlns_),name(name_),transportNS(transNS);
	QString xmlns;
	QString name;
	QString transportNS;
//	QList<QDomElement> payloads;
//	QList<JingleConnectionCandidate> candidates;
}
//END JingleContentType

QDomElement createInitializationMessage(QString* from, QString* to, QString* id, QString* sid, QString* initiator, QList<JingleContentType> types);

QDomElement createAcceptMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QList<JingleContentType> types, JingleConnectionCandidate connection);

QDomElement createTerminateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* reason);

QDomElement createReceiptMessage(QDomElement stanza);

QDomElement createContentErrorMessage(QDomElement stanza);

QDomElement createTransportErrorMessage(QDomElement stanza);

//NOTE not implemented yet
QDomElement createTransportCandidateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* contentName, QString* transportNS, JingleConnectionCandidate* candidate);

QDomElement createOrderErrorMessage(QDomElement stanza);

/**
 * Sent if the candidate picked on the other side does not work here
 */
QDomElement createTransportNotAcceptableMessage(QDocument stanza);

#endif
