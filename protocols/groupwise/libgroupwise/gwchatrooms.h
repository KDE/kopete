/*
    Kopete Groupwise Protocol
    gwchatrooms.h - Data types for group chat

    Copyright (c) 2005      SUSE Linux AG	 	 http://www.suse.com

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

#include <qdatetime.h>
#include <qvaluelist.h>

#ifndef GROUPWISE_CHATROOMS_H
#define GROUPWISE_CHATROOMS_H

namespace GroupWise
{
	
class ChatContact
{
	public:
		QString dn;
		uint chatRights;
};
typedef QValueList<GroupWise::ChatContact> ChatContactList;

struct ChatroomSearchResult
{
	QString name;
	QString ownerDN;
	uint participants;
};


class Chatroom
{
	public:
		enum UserStatus { Participating, NotParticipating };
		enum Rights { Read = 1, Write = 2, Modify = 4, Moderator = 8, Owner = 16 };
		QString creatorDN;
		QString description;
		QString disclaimer;
		QString displayName;
		QString objectId;
		QString ownerDN;
		QString query;
		QString topic;
		bool archive;
		uint maxUsers;
		uint chatRights;
		UserStatus userStatus;
		QDateTime createdOn;
		uint participantsCount;
		// haveParticipants, Acl, Invites indicate if we have obtained these lists from the server, so we can tell 'not fetched list' and 'fetched empty list' apart.
		bool haveParticipants;
		ChatContactList	participants;
		bool haveAcl;
		ChatContactList acl;
		bool haveInvites;
		ChatContactList invites;
			
		Chatroom() { archive = false; maxUsers = 0; chatRights = 0; participantsCount = 0; haveParticipants = false; haveAcl = false; haveInvites = false; }
		Chatroom( ChatroomSearchResult csr ) { archive = false; maxUsers = 0; chatRights = 0; participantsCount = csr.participants; haveParticipants = false; haveAcl = false; haveInvites = false; ownerDN = csr.ownerDN; displayName = csr.name; }
};

typedef QValueList<Chatroom> ChatroomList;
typedef QMap<QString, Chatroom> ChatroomMap;
}
#endif
