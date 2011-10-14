/*
    Kopete Groupwise Protocol
    chatroommanager.h - tracks our knowledge of server side chatrooms

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com

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

#ifndef CHATROOMMANAGER_H
#define CHATROOMMANAGER_H

#include <qobject.h>

#include "gwchatrooms.h"
#include "libgroupwise_export.h"

namespace GroupWise {
	class Client;
}

/**
 * Keeps a record of the server side chatrooms
 * @author SUSE Linux Products GmbH
 */
class LIBGROUPWISE_EXPORT ChatroomManager : public QObject
{
	Q_OBJECT
	public:
		ChatroomManager( GroupWise::Client * client);
		~ChatroomManager();
		GroupWise::ChatroomMap rooms();
		void requestProperties( const QString & displayName );
		void updateRooms();
		void updateCounts();
	signals:
		void gotProperties( const GroupWise::Chatroom & );
		void updated();
	protected:
		void getChatrooms( bool refresh );
	protected slots:
		/**
		 * Used to initialise the list of chatrooms in response to a SearchChatTask.
		 */
		void slotGotChatroomList();
		/**
		 * Used to update the user counts of chatrooms.
		 */
		void slotGotChatCounts();
		/**
		 * Get the properties of a specific room.
		 */
		void slotGotChatProperties();
	private:
		GroupWise::Client * m_client;
		GroupWise::ChatroomMap m_rooms;
		bool m_replace;
};

#endif
