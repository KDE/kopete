//
// C++ Interface: getdetailstask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GETDETAILSTASK_H
#define GETDETAILSTASK_H

#include "gwerror.h"
#include "requesttask.h"

/**
This task fetches the details for a set of user IDs from the server.  Sometimes we get an event that only has a DN, and we need other details before showing the event to the user.

@author SUSE AG
*/
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
