//
// C++ Interface: logintask
//
// Description: 
//
//
// Author: SUSE AG (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOGINTASK_H
#define LOGINTASK_H

#include <qstring.h> //see typedef

#include "requesttask.h"

typedef QString ContactListItem; // temp typedef pending impl

/**
@author Kopete Developers
*/
class LoginTask : public RequestTask
{
Q_OBJECT
public:
	LoginTask( Task * parent );
	~LoginTask();
	/**
	 * Get the login fields ready to go
	 */
	void initialise();
	/**
	 * Sends the login fields and gets ready to handle the contactlist that is returned.
	 */
	void onGo();
	/**
	 * Only accepts the contactlist that comes back from the server, 
	 * processes it and notifies the client of the contactlist
	 */
	bool LoginTask::take( Transfer * transfer );
	
signals:
	void contactListItemAdded( ContactListItem & );
	//void userRecord( UserRecord &, bool myself );
};

#endif
