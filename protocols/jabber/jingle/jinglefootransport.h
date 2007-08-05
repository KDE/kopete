/*
    jinglefootransport.h - Definition of a foo transport.

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
#ifndef JINGLEFOOTRANSPORT_H
#define JINGLEFOOTRANSPORT_H

#include "jingletransport.h"

class JingleFooTransport : public JingleTransport
{
public:
	virtual QList<JingleConnectionCandidate> getLocalCandidates();
	virtual JingleConnectionCandidate* createCandidateFromElement(QDomElement transportElement);


private:
//	JingleFooTransport();
//	JingleFooTransport(const JingleFooTransport& other);
	

};

#endif 
