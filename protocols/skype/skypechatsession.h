/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#ifndef SKYPECHATSESSION_H
#define SKYPECHATSESSION_H

#include <kopetechatsession.h>

class SkypeProtocol;
class SkypeAccount;
class SkypeContact;
class SkypeChatSessionPrivate;

/**
 * The chat session for the Skype protocol
 * @author Kopete Developers
 */
class SkypeChatSession : public Kopete::ChatSession
{
	Q_OBJECT
	private:
		///The insides of the chat session
		SkypeChatSessionPrivate *d;
	private slots:
		///sends message to the skype user who this chat belongs to
		void message(Kopete::Message&);
		/**This disables permanently the call button when the chat becomes a multi-user chat
		 * @todo make this unneeded and allow multiple-user calls
		 */
		void disallowCall();
		///Do a call to all participants of the chat (in future, now it allows only one at onece)
		void callChatSession();
	public:
		/**
		 * Constructor. The chat session will be created with first message comming out.
		 * @param account The account it belongs to
		 * @param other The other side. There it no way user could create chat with more than one user at once. If user is invited to chat by someone, the other constructor should be used. It is automatically registered in the manager.
		 */
		SkypeChatSession(SkypeAccount *account, SkypeContact *other);
		/**
		 * Constructor from chat session.
		 * Use this one if user is invited to an existing chat or if the first message in that chat was incoming message. The list of users will be loaded at startup. It is automatically registered in the manager.
		 * @param account The account this belongs to.
		 * @param session Identificator of the chat session in skype. The list of users will be loaded in startup and therefore they are not needed to be specified.
		 */
		SkypeChatSession(SkypeAccount *account, const QString &session, const Kopete::ContactPtrList &contacts);
		///Destructor
		~SkypeChatSession();
		/**
		 * Invites a contact to the chat
		 * @param contactId What contact
		 */
		virtual void inviteContact(const QString &contactId);
	public slots:
		/**
		 * Update the chat topic
		 * @param chat What chat is it about? Maybe me?
		 * @param topic What to set as topic
		 */
		void setTopic(const QString &chat, const QString &topic);
		/**
		 * Set this chat's ID
		 * @param chatId The new ID
		 */
		void setChatId(const QString &chatId);
		/**
		 * Add new user to chat
		 * @param chat To know if he joined this chat
		 * @param userId ID of that user
		 */
		void joinUser(const QString &chat, const QString &userId);
		/**
		 * Some user left the chat
		 * @param chat Is it this chat?
		 * @param userId ID of that user
		 * @param reason Why he left
		 */
		void leftUser(const QString &chat, const QString &userId, const QString &reason);
		/**
		 * This will add message that has been sent out by this user
		 * @param recv List of receivers. If there are more than one, replaced by an dummy contact of that chat, because it does crash kopete otherwise
		 * @param body Text to show
		 */
		void sentMessage(const QPtrList<Kopete::Contact> *recv, const QString &body);
	signals:
		/**
		 * This is emited when it become a multi-user chat. It should be removed from the contact so when user clicks on the contact, new one with only that one should be created
		 * @param chatSession Identificator of the chat
		 * @param previousUser Id of the other user before it became a multichat or empty string if no such user ever was
		 * @param sender Pointer to the chat session that emited this
		 */
		void becameMultiChat(const QString &chatSession, SkypeChatSession *sender);
		/**
		 * This is emited when there is an request to get a frindly name of a chat
		 * @param chat Id of that chat
		 */
		void wantTopic(const QString &chat);
		/**
		 * This chat's ID has changed
		 * @param oldId What was before? If it is the first set of the ID, it is empty
		 * @param newId The new ID. If it is empty, it means that this chat is being deleted right now and should be removed from all lists
		 * @param sender Pointer to that chat
		 */
		void updateChatId(const QString &oldId, const QString &newId, SkypeChatSession *sender);
		/**
		 * Request inviting user to a chat
		 * @param chatId What chat
		 * @param userId What user
		 */
		void inviteUserToChat(const QString &chatId, const QString &userId);
		/**
		 * Request leaving the chat
		 * @param chatId What chat
		 */
		void leaveChat(const QString &chatId);
};

#endif
