//
// C++ Interface: getstatustask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GETSTATUSTASK_H
#define GETSTATUSTASK_H

#include "requesttask.h"

/**
 * Request the status for a specific contact (e.g. one who's not on our contact list)
 * @author SUSE AG
*/
class GetStatusTask : public RequestTask
{
Q_OBJECT
public:
	GetStatusTask(Task* parent);
	~GetStatusTask();
	void userDN( const QString & dn );
	bool take( Transfer * transfer );
signals:
	void gotStatus( const QString & contactId, Q_UINT16 status, const QString & statusText );
protected:
	void onGo();
private:
	QString m_userDN;
};

#endif
