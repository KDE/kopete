/*
    iconnector.h
 
    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef ICONNECTOR_H
#define ICONNECTOR_H

class IConnector {
	IConnector(const IConnector&);
	IConnector& operator=(const IConnector&);

public:
	IConnector() {};
	virtual ~IConnector() {}
	virtual void setConnectedStatus(bool newStatus) = 0;
};

#endif
