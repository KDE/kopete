/*
    Kopete Groupwise Protocol
    requesttask.h - Ancestor of all tasks that carry out a user request

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

#ifndef GW_REQUESTTASK_H
#define GW_REQUESTTASK_H

#include "libgroupwise_export.h"

#include "task.h"

class Transfer;

class LIBGROUPWISE_EXPORT RequestTask : public Task
{
Q_OBJECT
	public:
		RequestTask( Task *parent );
		bool take( Transfer * transfer );
		virtual void onGo();
	protected:
		virtual bool forMe( const Transfer * transfer ) const;
		void createTransfer( const QString & command, const Field::FieldList & fields );
	private:
		int m_transactionId;
};

#endif
