/*
    Kopete Groupwise Protocol
    getdetailstask.h - fetch a contact's details from the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GETDETAILSTASK_H
#define GETDETAILSTASK_H

#include "gwerror.h"
#include "requesttask.h"

/**
This task fetches the details for a set of user IDs from the server.  Sometimes we get an event that only has a DN, and we need other details before showing the event to the user.

@author SUSE AG
*/
using namespace GroupWise;

class GetDetailsTask : public RequestTask
{
Q_OBJECT
public:
	GetDetailsTask( Task * parent );
	~GetDetailsTask();
	bool take( Transfer * transfer );
	void userDNs( const QStringList & userDNs );
signals:
	void gotContactUserDetails( const GroupWise::ContactDetails & );
protected:
	GroupWise::ContactDetails extractUserDetails( Field::MultiField * details );

};

#endif
