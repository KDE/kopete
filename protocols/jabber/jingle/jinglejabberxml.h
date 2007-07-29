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
#ifndef JINGLEJABBERXML_H_
#define JINGLEJABBERXML_H_ 

//#include <QtXml>

#include "xmpp_xmlcommon.h"

//#include "jinglecontenttype.h"

class JingleConnectionCandidate;

//BEGIN JingleContentType
struct JingleContentType
{
//	JingleContentType();
//	JingleContentType(xmlnsX , nameX , transNS):
//		xmlns(xmlnsX),name(nameX),transportNS(transNS);
	QString xmlns;
	QString name;
	QString transportNS;
//	QList<QDomElement> payloads;
//	QList<JingleConnectionCandidate> candidates;
};
//END JingleContentType

namespace Jingle{

QDomDocument createInitializationMessage(QString* from, QString* to, QString* id, QString* sid, QString* initiator, QList<JingleContentType> types);

QDomDocument createAcceptMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QList<JingleContentType> types, JingleConnectionCandidate connection);

QDomDocument createTerminateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* reason);

QDomDocument createReceiptMessage(QDomElement stanza);

QDomDocument createContentErrorMessage(QDomElement stanza);

QDomDocument createTransportErrorMessage(QDomElement stanza);

//NOTE not implemented yet
QDomDocument createTransportCandidateMessage(QString* from, QString* to, QString* initiator, QString* responder, QString* id, QString* sid, QString* contentName, QString* transportNS, JingleConnectionCandidate* candidate);

QDomDocument createOrderErrorMessage(QDomElement stanza);

/**
 * Sent if the candidate picked on the other side does not work here
 */
QDomDocument createTransportNotAcceptableMessage(QDomElement stanza);

}
#endif
