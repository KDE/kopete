/*
    jinglefooconnectioncandidate.h - definition of foo transport candidate

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

#ifndef JINGLEFOOCANDIDATE_H_
#define JINGLEFOOCANDIDATE_H_

#include "jingleconnectioncandidate.h"

class JingleFooConnectionCandidate : public JingleConnectionCandidate
{
public:
	JingleFooConnectionCandidate(QString ip, int port){ JingleConnectionCandidate(ip,port);}
	virtual bool isUseful();
	/**
	* A candiate for the foo transport has the following XML structure:
	* <candidate ip="10.10.10.10" port="300" />
	* Implemented in jinglefootransport.cpp
	*/
	virtual QDomElement getCandidateElement();
};




#endif
