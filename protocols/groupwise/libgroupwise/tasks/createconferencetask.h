/*
    Kopete Groupwise Protocol
    createconferencetask.h - Request task that creates conferences on the server

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

#ifndef CREATECONFERENCETASK_H
#define CREATECONFERENCETASK_H

#include "requesttask.h"
#include "client.h"

/**
This task is responsible for creating a conference at the server, and confirming that the server allowed the conference to be created.

@author SUSE AG
*/
class CreateConferenceTask : public RequestTask
{
Q_OBJECT
public:
	CreateConferenceTask(Task* parent);
	~CreateConferenceTask();
	/**
	 * Set up a create conference request
	 * @param confId The client-unique conference Id.
	 * @param participants A list of Novell DNs of the people taking part in the conference.
	 */
	void conference( const int confId, const QStringList &participants );
	bool take( Transfer * transfer );
	int clientConfId() const;
	GroupWise::ConferenceGuid conferenceGUID() const;
	
signals:
	void created( const GroupWise::ConferenceGuid & guid );
private: 
	int m_confId; // the conference id given us before making the request
	ConferenceGuid m_guid;
};

#endif
