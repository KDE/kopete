/*
    Kopete Groupwise Protocol
    chatcountstask.cpp - Task to update chatroom participant counts

    Copyright (c) 2005      SUSE Linux Products GmbH	 http://www.suse.com

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATPROPERTIESTASK_H
#define CHATPROPERTIESTASK_H

#include <qdatetime.h>
#include <qlist.h>
#include "gwchatrooms.h"
#include "gwerror.h"
#include "gwfield.h"

#include "requesttask.h"

/**
Get the current number of users in each chat on the server

@author SUSE Linux Products GmbH
 */
class ChatPropertiesTask : public RequestTask
{
	Q_OBJECT
	public:
		ChatPropertiesTask(Task* parent);
		~ChatPropertiesTask();
		/**
		 * Specify which chatroom to get properties for
		 */
		void setChat( const QString & );
		bool take( Transfer * transfer );
		/**
		 * Contains a list of the ACL entries for the specified chatroom
		 */
		QList< GroupWise::ChatContact > aclEntries();
		QString m_chat;
		QString m_ownerDn;
		QString m_description;
		QString m_disclaimer;
		QString m_query;
		QString m_archive;
		QString m_maxUsers;
		QString m_topic;
		QString m_creatorDn;
		QDateTime m_creationTime;
		uint m_rights;
		QList< GroupWise::ChatContact > m_aclEntries;
};

#endif
