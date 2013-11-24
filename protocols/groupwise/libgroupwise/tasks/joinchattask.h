/*
    Kopete Groupwise Protocol
    joinchattask.h - Join a chatroom on the server, after having been invited.

    Copyright (c) 2004      SUSE Linux Products GmbH	 	 http://www.suse.com
    
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

#ifndef JOINCHATTASK_H
#define JOINCHATTASK_H

#include <QStringList>
#include "requesttask.h"

using namespace GroupWise;

/**
Sends Join Conference messages when the user accepts an invitation

@author SUSE Linux Products GmbH
 */

class JoinChatTask : public RequestTask
{
	Q_OBJECT
	public:
		JoinChatTask(Task* parent);
		~JoinChatTask();
		void join( const QString & displayName );
		bool take( Transfer * transfer );
		QStringList participants() const;
		QStringList invitees() const;
		QString displayName() const;
	private:
		ConferenceGuid m_displayName;
		QStringList m_participants;
		QStringList m_invitees;
		QStringList m_unknowns;
};

#endif
