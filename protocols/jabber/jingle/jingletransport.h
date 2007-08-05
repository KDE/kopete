/*
    jingletransport.h - Define jingle transport base class.

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
#ifndef JINGLETRANSPORT_H
#define JINGLETRANSPORT_H

#include "jingleconnectioncandidate.h"

/**
 * A class representing a Jingle transport protocol
 */
class JingleTransport
{

public:
	/**
	 * Enumerate candidates for this transport type
	 */
	virtual QList<JingleConnectionCandidate> getLocalCandidates();

	/**
	 * Create a candidate of this type from a <transport> element containing a <candidate> element
	 */
	virtual JingleConnectionCandidate* createCandidateFromElement(QDomElement candidate);

private:

};

#endif
