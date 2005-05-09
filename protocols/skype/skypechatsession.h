/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <vorner@seznam.cz>

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

#include <kopetemessagemanager.h>

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
		/**
		 * This slot is used for knowing the ID of the last message and putting it into the pending list
		 * @param id ID of the message
		 */
		void knowId(const QString &id);
		/**
		 * Used for knowing when a message has been sent by skype
		 * @param id ID of the message that has been sent
		 */
		void messageSent(const QString &id);
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
		SkypeChatSession(SkypeAccount *account, const QString *session);
		///Destructor
		~SkypeChatSession();
	signals:
		/**
		 * This is emited when it become a multi-user chat. It should be removed from the contact so when user clicks on the contact, new one with only that one should be created
		 */
		void becameMultiChat();
};

#endif
