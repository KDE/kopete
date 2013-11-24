/*
    Kopete Groupwise Protocol
    chatroommanager.cpp - tracks our knowledge of server side chatrooms

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

#include "chatroommanager.h"

#include <qmap.h>
#include <q3valuelist.h>

#include <kdebug.h>

#include "client.h"
#include "tasks/chatcountstask.h"
#include "tasks/chatpropertiestask.h"
#include "tasks/searchchattask.h"

ChatroomManager::ChatroomManager( Client * parent)
	: QObject(parent), m_client( parent ), m_replace( false )
{
}

ChatroomManager::~ChatroomManager()
{
}

void ChatroomManager::updateRooms()
{
	getChatrooms( !m_rooms.isEmpty() );
}

GroupWise::ChatroomMap ChatroomManager::rooms()
{
	return m_rooms;
}

void ChatroomManager::getChatrooms( bool refresh )
{
	m_replace = !refresh;
	SearchChatTask * sct = new SearchChatTask( m_client->rootTask() );
	sct->search( ( refresh ? SearchChatTask::SinceLastSearch : SearchChatTask::FetchAll ) );
	connect( sct, SIGNAL(finished()), SLOT(slotGotChatroomList()) );
	sct->go( true );
}

void ChatroomManager::slotGotChatroomList()
{
//	kDebug () ;
	SearchChatTask * sct = (SearchChatTask *)sender();
	if ( sct )
	{
		if ( m_replace )
			m_rooms.clear();
		
		const QList<ChatroomSearchResult> roomsFound = sct->results();
		QList<ChatroomSearchResult>::ConstIterator it = roomsFound.begin();
		QList<ChatroomSearchResult>::ConstIterator end = roomsFound.end();
		for ( ; it != end; ++it )
		{
			GroupWise::Chatroom c( *it );
			m_rooms.insert( c.displayName, c );
		}
	}
	emit updated();
}

void ChatroomManager::updateCounts()
{
	ChatCountsTask * cct = new ChatCountsTask( m_client->rootTask() );
	connect( cct, SIGNAL(finished()), SLOT(slotGotChatCounts()) );
	cct->go( true );
}

void ChatroomManager::slotGotChatCounts()
{
	ChatCountsTask * cct = (ChatCountsTask *)sender();
	if ( cct )
	{
		QMap< QString, int > newCounts = cct->results();
		QMap< QString, int >::iterator it = newCounts.begin();
		const QMap< QString, int >::iterator end = newCounts.end();

		for ( ; it != end; ++it )
			if ( m_rooms.contains( it.key() ) )
				m_rooms[ it.key() ].participantsCount = it.value();
	}
	emit updated();
}

void ChatroomManager::requestProperties( const QString & displayName )
{
	if ( 0 /*m_rooms.contains( displayName ) */)
		emit gotProperties( m_rooms[ displayName ] );	
	else
	{
		ChatPropertiesTask * cpt = new ChatPropertiesTask( m_client->rootTask() );
		cpt->setChat( displayName );
		connect ( cpt, SIGNAL(finished()), SLOT(slotGotChatProperties()) );
		cpt->go( true );
	}
}

void ChatroomManager::slotGotChatProperties()
{
//	kDebug() ;
	ChatPropertiesTask * cpt = (ChatPropertiesTask *)sender();
	if ( cpt )
	{
		Chatroom room = m_rooms[ cpt->m_chat ];
		room.displayName = cpt->m_chat;
		room.ownerDN = cpt->m_ownerDn;
		room.description = cpt->m_description;
		room.disclaimer = cpt->m_disclaimer;
		room.query = cpt->m_query;
		room.archive = ( cpt->m_archive == "0" );
		room.maxUsers = cpt->m_maxUsers.toInt();
		room.topic = cpt->m_topic;
		room.creatorDN = cpt->m_creatorDn;
		room.createdOn = cpt->m_creationTime;
		room.acl = cpt->m_aclEntries;
		room.chatRights = cpt->m_rights;
		m_rooms.insert( room.displayName, room );
		emit gotProperties( room );
	}
}

#include "chatroommanager.moc"
