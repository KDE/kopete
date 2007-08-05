/*
    jinglejabberxml.h - Jingle jabber stanza creation.

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
#ifndef JINGLEJABBERXML_H_
#define JINGLEJABBERXML_H_ 

#define JINGLE_NS "http://www.xmpp.org/extensions/xep-0166.html#ns"
#define JINGLE_FOO_NS "http://kopete.kde.com/jingle/foo.html"
#define JINGLE_VOICE_SESSION_NS "http://www.xmpp.org/extensions/xep-0167.html#ns"
#define JINGLE_FOO_TRANSPORT_NS "http://kopete.kde.com/jingle/foo-transport.html"
#define JINGLE_UDP_TRANSPORT_NS "http://www.xmpp.org/extensions/xep-0177.html#ns"

//#include <QtXml>

#include "xmpp_xmlcommon.h"
#include <QList>

//#include "jinglecontenttype.h"
#include "jingleconnectioncandidate.h"


//BEGIN JingleContentType
struct JingleContentType
{
//	JingleContentType();
//	JingleContentType(xmlnsX , nameX , transNS):
//		xmlns(xmlnsX),name(nameX),transportNS(transNS);
	QString xmlns;
	QString name;
	QString transportNS;
	QString creator;
	virtual QList<QDomElement> payloads();

	/**
	 * List of candidates to connect to this machine over the transport
	 */
	QList<JingleConnectionCandidate> candidates;

	/**
	 * Chosen candidate to connect to the other machine
	 */
	JingleConnectionCandidate *connection;

	JingleConnectionCandidate *writeConnection;
};
//END JingleContentType





namespace Jingle{

//session-initiate
QDomDocument createInitializationMessage(QString from, QString to, QString id, QString sid, QString initiator, QList<JingleContentType> types);

//session-accept
QDomDocument createAcceptMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QList<JingleContentType> types);

//session-terminate
QDomDocument createTerminateMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QString reason);

QDomDocument createReceiptMessage(QDomElement stanza);

//unsupported-content
QDomDocument createContentErrorMessage(QDomElement stanza);

//unsupported transport
QDomDocument createTransportErrorMessage(QDomElement stanza);

QDomDocument createTransportCandidateMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, QString contentName, QString contentCreator, QString transportNS, JingleConnectionCandidate *candidate);

QDomDocument createOrderErrorMessage(QDomElement stanza);

/**
 * Sent if the candidate picked on the other side does not work here
 */
QDomDocument createTransportNotAcceptableMessage(QDomElement stanza);

QDomDocument createUnknownSessionError(QDomElement stanza);

QDomDocument createContentAcceptMessage(QString from, QString to, QString initiator, QString responder, QString id, QString sid, JingleContentType content);

}
#endif
