/*
    irccontact.h - IRC Contact

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include <qptrlist.h>
#include <qmap.h>

#include "kirc.h"
#include "kopetecontact.h"
#include "kopetemessage.h"

class KopeteMessageManager;
class KopeteMetaContact;
class IRCAccount;

struct whoIsInfo;

/**
 * @author Jason Keirstead <jason@keirstead.org
 *
 * This class is the base class for @ref IRCUserContact and @ref IRCChannelContact.
 * Common routines and signal connections that are required for both types of
 * contacts reside here, to avoid code duplication between these two classes.
 */
class IRCContact : public KopeteContact
{
	Q_OBJECT

	public:
		IRCContact(IRCAccount *account, const QString &nick, KopeteMetaContact *metac, const QString& icon = QString::null );

		/**
		 * Sets the nickname of this contact. The nickname is distinct from the displayName
		 * in case trackNameChanges is disabled.
		 */
		void setNickName(const QString &nickname) { mNickName = nickname; }

		/**
		 * Returns the nickname / channel name
		 */
		const QString &nickName() const { return mNickName; }

		/**
		 * This function attempts to find the nickname specified within the current chat
		 * session. Returns a pointer to that IRCUserContact, or 0L if the user does not
		 * exist in this session. More useful for channels. Calling IRCChannelContact::locateUser()
		 * for example tells you if a user is in a certain channel.
		 */
		KopeteContact *locateUser( const QString &nickName );

		virtual bool isReachable();

		/**
		 * return true if the contact is in a chat. false if the contact is in no chats
		 * that loop over all manager, and checks the presence of the user
		 */
		bool isChatting() const;


	private slots:
		void slotConnectionClosed();
		void slotNewMessage(const QString &originating, const QString &target, const QString &message);
		void slotNewAction(const QString &originating, const QString &target, const QString &message);
		void slotNewWhoIsUser(const QString &nickname, const QString &username, const QString &hostname, const QString &realname);
		void slotNewWhoIsServer(const QString &nickname, const QString &server, const QString &serverInfo);
		void slotNewWhoIsOperator(const QString &nickname);
		void slotNewWhoIsIdle(const QString &nickname, unsigned long seconds);
		void slotNewWhoIsChannels(const QString &nickname, const QString &channel);
		void slotWhoIsComplete(const QString &nickname);
		void slotNewNickChange( const QString &oldnickname, const QString &newnickname);
		void slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived);
		void slotUserDisconnected( const QString &nickname, const QString &reason);

	protected slots:
		void slotSendMsg(KopeteMessage &message, KopeteMessageManager *);

	protected:
		QPtrList<KopeteContact> mMyself;
		QMap<QString, whoIsInfo*> mWhoisMap;
		KopeteMetaContact *mMetaContact;
		KIRC *mEngine;
		KopeteMessageManager *mMsgManager;
		IRCAccount *mAccount;
		QString mNickName;
		QValueList<KopeteMessage> messageQueue;
		bool isConnected;
		KopeteMessage::MessageDirection execDir;

};

#endif
