//
// C++ Interface: createcontacttask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CREATECONTACTTASK_H
#define CREATECONTACTTASK_H

#include "modifycontactlisttask.h"

/**
Creates a contact on the server.  The response to this action is handled by its parent

@author SUSE AG
*/
class CreateContactTask : public ModifyContactListTask
{
Q_OBJECT
public:
	CreateContactTask(Task* parent);
	~CreateContactTask();
	/** 
	 * Sets up the request message.  
	 */
	void contactFromUserId( const QString & userId, const QString & displayName, const int parentFolder );
	void contactFromDN( const QString & dn, const QString & displayName, const int parentFolder );
protected:
	void contact( Field::SingleField * id, const QString & displayName, const int parentFolder );
};

#endif
