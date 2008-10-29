/*
    Kopete Groupwise Protocol
    getstatustask.h - fetch a contact's details from the server

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
	void gotStatus( const QString & contactId, quint16 status, const QString & statusText );
private:
	QString m_userDN;
};

#endif
