//
// C++ Interface: requesttask
//
// Description: 
//
//
// Author: SUSE AG  (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef GW_REQUESTTASK_H
#define GW_REQUESTTASK_H

#include "task.h"

class Transfer;

class RequestTask : public Task
{
Q_OBJECT
	public:
		RequestTask( Task *parent );
	protected:
		bool forMe( Transfer * transfer ) const;
		void setTransactionId( const int transactionId );
	private:
		int m_transactionId;
};

#endif
