/*
    Kopete Groupwise Protocol
    joinconferencetask.h - Join a conference on the server, after having been invited.

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

#ifndef JOINCONFERENCETASK_H
#define JOINCONFERENCETASK_H

#include <QStringList>
#include "requesttask.h"

using namespace GroupWise;

/**
Sends Join Conference messages when the user accepts an invitation

@author SUSE AG
*/

class JoinConferenceTask : public RequestTask
{
Q_OBJECT
public:
	JoinConferenceTask(Task* parent);
	~JoinConferenceTask();
	void join( const ConferenceGuid & guid );
	bool take( Transfer * transfer );
	QStringList participants() const;
	QStringList invitees() const;
	ConferenceGuid guid() const;
public slots:
	void slotReceiveUserDetails( const GroupWise::ContactDetails & details );
private:
	ConferenceGuid m_guid;
	QStringList m_participants;
	QStringList m_invitees;
	QStringList m_unknowns;
};

#endif
