/*
    jinglefootransport.cpp - Definition of a foo transport.

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

#include "jinglefootransport.h"
#include "jinglefooconnectioncandidate.h"

JingleConnectionCandidate* JingleFooTransport::createCandidateFromElement(QDomElement transportElement)
{
	QDomElement candidate = transportElement.firstChild().toElement();
	return new JingleFooConnectionCandidate(candidate.attribute("ip"), candidate.attribute("port").toInt());
}

QList<JingleConnectionCandidate> JingleFooTransport::getLocalCandidates()
{
	JingleFooConnectionCandidate c("127.0.0.1", 300);
	QList<JingleConnectionCandidate> list;
	list.append(c);
	return list;
}

QDomElement JingleFooConnectionCandidate::getCandidateElement()
{
	QDomElement candidate;
	candidate.setTagName("candidate");
	candidate.setAttribute("ip", ip_);
	candidate.setAttribute("port",port_);
	return candidate;
}

bool JingleFooConnectionCandidate::isUseful()
{
	return true;
}
